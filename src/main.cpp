#include <stdio.h>
#include "pico/stdlib.h"

#include "external/lora/lib.hpp"

#include "mpu6050.h"
#include "motors.h"

#include "util.h"



void parse_lora(uint8_t* packetCounter, uint16_t* joyX1, uint16_t* joyY1, uint16_t* joyX2, uint16_t* joyY2) {
	while (LoRa.available()) {
		*packetCounter = (uint8_t)LoRa.read();

		uint16_t* values[4] = {joyX1, joyY1, joyX2, joyY2};

		for (int i = 0; i < 4; i++) {
			unsigned char a = LoRa.read();
			unsigned char b = LoRa.read();
			uint16_t result = (uint16_t)((a << 8) | b);
			*values[i] = result;
		}
	}
}

int main() {
    stdio_init_all();

	printf("overclocking...\n");
	set_sys_clock_khz(250000, true);
	printf("successfully overclocked!\n");

	// blink for 3s
	// this is not just for status and but also waiting for the IMU
	status_blink(0.8, 5);

	printf("hello, drone!\n");

	if (!LoRa.begin(432E6)) {
		return fatal_error("lora failed to start!");
	}

	printf("lora started\n");

	uint8_t counter = 0;
	uint16_t jx1 = 510;
	uint16_t jy1 = 510;
	uint16_t jx2 = 510;
	uint16_t jy2 = 510;

	while (1) {
		int packetSize = LoRa.parsePacket();
		if (packetSize > 0) {
			counter++;

			uint8_t packetCounter;
			parse_lora(&packetCounter, &jx1, &jy1, &jx2, &jy2);
			printf("c: %d, pc: %d, jx1: %d, jy1: %d, jx2: %d, jy2: %d\n", counter, packetCounter, jx1, jy1, jx2, jy2);

			if (counter != packetCounter) {
				counter = packetCounter;
			}
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
