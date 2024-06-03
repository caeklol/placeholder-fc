#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "mpu6050.h"
#include "util.h"

int main() {
    stdio_init_all();

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
}
