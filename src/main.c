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

const int throttle_max = 2000000;
const int throttle_min = 1000000;

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

	motor_ns(M1, 0);
	motor_ns(M2, 0);
	motor_ns(M3, 0);
	motor_ns(M4, 0);
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
			motor_ns(M1, 0);
			motor_ns(M2, 0);
			motor_ns(M3, 0);
			motor_ns(M4, 0);

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
			printf("gyro_x: %.6f, gyro_y: %.6f, gyro_z: %.6f\n", gyro[0], gyro[1], gyro[2]);
			printf("t1: %.6f, t2: %.6f, t3: %.6f, t4: %.6f\n", t1, t2, t3, t4);

			last_mode = 0;
		}


		uint64_t end_time = to_us_since_boot(get_absolute_time());
		uint64_t elapsed = end_time - start_time;

		if (elapsed < cycle_time) {
			sleep_us(cycle_time - elapsed);
		}
	}
}
