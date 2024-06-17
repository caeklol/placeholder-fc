#include "external/lora/lib.hpp"

#include "controller.hpp"

#include "util.h"

static uint8_t CONTROLLER_COUNTER = 0;

int controller_init(long frequency) {
	if (!LoRa.begin(432E6)) {
		return fatal_error("controller: failed to start!");
	} else {
		return 0;
	}
};

void controller_accept_packet(uint8_t* panic, uint16_t* joyX1, uint16_t* joyY1, uint16_t* joyX2, uint16_t* joyY2) {
	int packetSize = LoRa.parsePacket();
	if (packetSize > 0) {
		CONTROLLER_COUNTER++;

		while (LoRa.available()) {
			uint8_t packetCounter = (uint8_t)LoRa.read();

			if (CONTROLLER_COUNTER != packetCounter) {
					CONTROLLER_COUNTER = packetCounter;

					for (int i = 0; i < 10; i++) {
						LoRa.read();
					}

					return;
			}

			*panic = (uint8_t)LoRa.read();

			uint16_t* values[4] = {joyX1, joyY1, joyX2, joyY2};

			for (int i = 0; i < 4; i++) {
				unsigned char a = LoRa.read();
				unsigned char b = LoRa.read();
				uint16_t result = (uint16_t)((a << 8) | b);
				*values[i] = result;
			}
		}
	}
};
