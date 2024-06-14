#include <stdio.h>
#include "pico/stdlib.h"

#include "lora.hpp"

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
	//status_blink(0.8, 5);
	sleep_ms(3000);

	printf("hello, drone!\n");

	if (!LoRa.begin(432E6)) {
		printf("Starting LoRa failed!\n");
		while (1);
	}

	printf("LoRa Started\n");

	int counter = 0;

	while (1) {
		int packetSize = LoRa.parsePacket();
		if (packetSize > 0) {
			printf("Received packet:\n");

			char number[25] = "";
			while (LoRa.available()) {
				char character = LoRa.read();
				strncat(number, &character, 1);
			}

			if (counter+1 == atoi(number)) {
				status_set(false);
				counter++;
			} else {
				status_set(true);
			}


			printf("\n");
			printf("EOF\n");
		}
	}

	//Motor M1 = motor_init(2, 250);
	//Motor M2 = motor_init(28, 250);
	//Motor M3 = motor_init(15, 250);
	//Motor M4 = motor_init(16, 250);
	//printf("testing speed...\n");

	//motor_speed(M1, 0);
	//motor_speed(M2, 0);
	//motor_speed(M3, 0);
	//motor_speed(M4, 0);
	//sleep_ms(2000);
	//motor_speed(M1, 0.1);
	//motor_speed(M2, 0.1);
	//motor_speed(M3, 0.1);
	//motor_speed(M4, 0.1);

	//int res;

	//res = mpu6050_init(12, 13, i2c_default);
	//if (res > 0) return 1;

	//printf("mpu6050 initialized!\n");

	//mpu6050_calibrate_gyro();
	//printf("calculated mpu6050 bias\n");
}
