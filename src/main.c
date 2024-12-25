#include <stdio.h>

#include "pico/stdlib.h"

#include "controller.hpp"
#include "mpu6050.h"
#include "motors.h"

#include "util.h"

float throttle_idle = 0.04;
float throttle_governor = 0.5;

float max_rate_roll = 30.0;
float max_rate_pitch = 30.0;
float max_rate_yaw = 50.0;

float cycle_hz = 250;

const float pid_roll_kp = 0.025;
const float pid_roll_ki = 0;
const float pid_roll_kd = 0;
const float pid_pitch_kp = pid_roll_kp;
const float pid_pitch_ki = pid_roll_ki;
const float pid_pitch_kd = pid_roll_kd;
const float pid_yaw_kp = 0.025;
const float pid_yaw_ki = 0;
const float pid_yaw_kd = 0;

int main() {
    stdio_init_all();

	printf("overclocking...\n");
	set_sys_clock_khz(250000, true);
	printf("successfully overclocked!\n");
	
	// blink for 3s
	// this is not just for status and but also waiting for the IMU
	status_blink(0.8, 5);

	printf("hello, drone!\n");

	// gyro init
	int res;

	res = mpu6050_init(12, 13, i2c_default);
	if (res > 0) return 1;
	mpu6050_calibrate_gyro();
	printf("gyro initialized\n");

	// controller init
	res = controller_init(432E6);
	if (res > 0) return 1;
	printf("controller initialized!\n");

	// motors init
	Motor M1 = motor_init(2, 250);
	Motor M2 = motor_init(28, 250);
	Motor M3 = motor_init(15, 250);
	Motor M4 = motor_init(16, 250);

	motor_speed(M1, 0);
	motor_speed(M2, 0);
	motor_speed(M3, 0);
	motor_speed(M4, 0);
	printf("motors initialized!\n");
	sleep_ms(2000);
	
	/// actual fc code
	float cycle_time_seconds = 1.0 / cycle_hz;
	int cycle_time = (int) (cycle_time_seconds * 1000000);
	float throttle_range = throttle_governor - throttle_idle;
	float i_limit = 150;
	uint8_t last_mode = 1;

	// lora cannot send packets as fast as others transmitters,
	// so inputs from the previous iteration must be carried onto the next.
	// this will eventually turn into just GPS coordinates so only occasional updates
	// are required.
	uint8_t mode = 1;
	uint16_t jx1 = 510;
	uint16_t jy1 = 510;
	uint16_t jx2 = 510;
	uint16_t jy2 = 510;

	controller_accept_packet(&mode, &jx1, &jy1, &jx2, &jy2);

	if (mode != 1) {
		return fatal_error("flight mode detected on startup!");
	}

	float roll_last_integral = 0;
	float roll_last_error = 0;
	float pitch_last_integral = 0;
	float pitch_last_error = 0;
	float yaw_last_integral = 0;
	float yaw_last_error = 0;

	status_set(true);

	float gyro[3];

	while (1) {
		uint64_t start_time = to_us_since_boot(get_absolute_time());

		mpu6050_read_gyro(gyro);
		mpu6050_correct_bias(gyro);

		controller_accept_packet(&mode, &jx1, &jy1, &jx2, &jy2);
	
		if (mode == 1) {
			motor_speed(M1, 0);
			motor_speed(M2, 0);
			motor_speed(M3, 0);
			motor_speed(M4, 0);

			yaw_last_error = 0;
			yaw_last_integral = 0;
			pitch_last_error = 0;
			pitch_last_integral = 0;
			roll_last_error = 0;
			roll_last_integral = 0;

			last_mode = 1;

			printf("standby mode! shutting motors off\n");
		} else if (mode == 0) {
			gyro[0] *= -1;
			
			float throttle = map(jx1, 0, 1023, 0, 1);
			float pitch = map(jx2, 0, 1023, -1, 1) * -1;
			float yaw = map(jy1, 0, 1023, -1, 1);
			float roll = map(jy2, 0, 1023, -1, 1);

			if (last_mode == 1) {
				if (throttle > 0.05) {
					return fatal_error("throttle was above 5 on setup");
				}
			}

			float adj_throttle = throttle_idle + (throttle_range * throttle);

			// proportional
			float error_rate_roll = (roll * max_rate_roll) - gyro[0];
			float error_rate_pitch = (pitch * max_rate_pitch) - gyro[1];
			float error_rate_yaw = (yaw * max_rate_yaw) - gyro[2];

			float roll_p = error_rate_roll * pid_roll_kp;
			float pitch_p = error_rate_pitch * pid_pitch_kp;
			float yaw_p = error_rate_yaw * pid_yaw_kp;

			// integral
			float roll_i = roll_last_integral + (error_rate_roll * pid_roll_ki * cycle_time_seconds);
			roll_i = max(min(roll_i, i_limit), -i_limit);

            float pitch_i = pitch_last_integral + (error_rate_pitch * pid_pitch_ki * cycle_time_seconds);
			pitch_i = max(min(pitch_i, i_limit), -i_limit);

			float yaw_i = yaw_last_integral + (error_rate_yaw * pid_yaw_ki * cycle_time_seconds);
			yaw_i = max(min(yaw_i, i_limit), -i_limit);
	
			// derivative
			float roll_d = pid_roll_kd * (error_rate_roll - roll_last_error) / cycle_time_seconds;
			float pitch_d = pid_pitch_kd * (error_rate_pitch - pitch_last_error) / cycle_time_seconds;
			float yaw_d = pid_yaw_kd * (error_rate_yaw - yaw_last_error) / cycle_time_seconds;

			float pid_roll = roll_p + roll_i + roll_d;
			float pid_pitch = pitch_p + pitch_i + pitch_d;
			float pid_yaw = yaw_p + yaw_i + yaw_d;

			float t1 = adj_throttle + pid_pitch + pid_roll - pid_yaw;
            float t2 = adj_throttle + pid_pitch - pid_roll + pid_yaw;
			float t3 = adj_throttle - pid_pitch + pid_roll + pid_yaw;
			float t4 = adj_throttle - pid_pitch - pid_roll - pid_yaw;

			motor_speed(M1, t1);
			motor_speed(M2, t2);
			motor_speed(M3, t3);
			motor_speed(M4, t4);

			roll_last_error = error_rate_roll;
			pitch_last_error = error_rate_pitch;
			yaw_last_error = error_rate_yaw;
			roll_last_integral = roll_i;
			pitch_last_integral = pitch_i;
			yaw_last_integral = yaw_i;

			//printf("throttle: %.6f, yaw: %.6f, pitch: %.6f, roll: %.6f\n", throttle, yaw, pitch, roll);
			printf("re: %.6f, pe: %.6f, ye: %.6f, ri: %.6f, pi: %.6f, yi: %.6f\n", roll_last_error,  pitch_last_error, yaw_last_error, roll_last_integral, pitch_last_integral, yaw_last_integral);
			printf("gyro_x: %.6f, gyro_y: %.6f, gyro_z: %.6f\n", gyro[0], gyro[1], gyro[2]);
			printf("t1: %.6f, t2: %.6f, t3: %.6f, t4: %.6f\n", t1, t2, t3, t4);

			last_mode = 0;
			// ADJUST MOTORS
		}


		uint64_t end_time = to_us_since_boot(get_absolute_time());
		uint64_t elapsed = end_time - start_time;

		if (elapsed < cycle_time) {
			sleep_us(cycle_time - elapsed);
		}
	}
}
