#include <stdio.h>
#include "pico/stdlib.h"

#include "controller.hpp"
#include "mpu6050.h"
#include "motors.h"

#include "util.h"

int main() {
    stdio_init_all();

	printf("overclocking...\n");
	set_sys_clock_khz(250000, true);
	printf("successfully overclocked!\n");

	// blink for 3s
	// this is not just for status and but also waiting for the IMU
	status_blink(0.8, 5);

	printf("hello, drone!\n");


	int res;

	res = mpu6050_init(12, 13, i2c_default);
	if (res > 0) return 1;

	printf("mpu6050 initialized!\n");

	mpu6050_calibrate_gyro();
	printf("calculated mpu6050 bias\n");

	res = controller_init(432E6);
	if (res > 0) return 1;

	printf("controller initialized!\n");
	Motor M1 = motor_init(2, 250);
	Motor M2 = motor_init(28, 250);
	Motor M3 = motor_init(15, 250);
	Motor M4 = motor_init(16, 250);
	printf("setting up motors...\n");

	motor_speed(M1, 0);
	motor_speed(M2, 0);
	motor_speed(M3, 0);
	motor_speed(M4, 0);
	printf("done!\n");
	sleep_ms(2000);

	uint8_t panic = 0;
	uint16_t jx1 = 510;
	uint16_t jy1 = 510;
	uint16_t jx2 = 510;
	uint16_t jy2 = 510;

	while (1) {
			controller_accept_packet(&panic, &jx1, &jy1, &jx2, &jy2);
		
			if (panic > 0) {
				motor_speed(M1, 0);
				motor_speed(M2, 0);
				motor_speed(M3, 0);
				motor_speed(M4, 0);
				printf("panic mode! shutting motors off\n");
				continue;
			}

			printf("jx1: %d, jy1: %d, jx2: %d, jy2: %d, p: %d\n", jx1, jy1, jx2, jy2, panic);
	}
}
