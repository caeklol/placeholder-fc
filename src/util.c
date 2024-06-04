#include "stdio.h"
#include "pico/stdlib.h"

bool status_initialized = false;

void status_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	status_initialized = true;
}

void status_set(int state) {
	if (!status_initialized) status_init();
	gpio_put(PICO_DEFAULT_LED_PIN, state);
}

void status_blink(float speed, int blinks) {
	int counter = blinks;
	int delay = (1.1F-speed) * 1000;

	while(counter) {
		status_set(1);
		sleep_ms(delay);
		status_set(0);
		sleep_ms(delay);
		counter--;
	}
}

int fatal_error(char* message) {
	printf("FATAL ERROR: ");
	printf(message);
	printf("\n");
	status_blink(1, 10);
	return -1;
}
