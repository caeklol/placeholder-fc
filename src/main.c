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

	int16_t gyro[3];
    while (1) {
        mpu6050_read_gyro_raw(gyro);

        printf("gyro: x=%d, y=%d, Z=%d\n", gyro[0], gyro[1], gyro[2]);

        sleep_ms(100);
    }
}
