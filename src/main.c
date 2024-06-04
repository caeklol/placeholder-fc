#include <stdio.h>
#include "pico/stdlib.h"

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


	Motor M1 = motor_init(2, 50);
	printf("testing speed...\n");

	motor_speed(M1, 0);
	sleep_ms(2000);
	motor_speed(M1, 0.05);


	//int res;

	//res = mpu6050_init(12, 13, i2c_default);
	//if (res > 0) return 1;

	//printf("mpu6050 initialized!\n");

	//mpu6050_calibrate_gyro();
	//printf("calculated mpu6050 bias\n");
}
