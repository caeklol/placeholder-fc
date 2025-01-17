#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "motors.h"
#include "util.h"

struct motor {
	uint slice;
	uint chan;
	uint32_t wrap;
	uint32_t period;
	bool enabled;
};

Motor motor_init(uint pin, uint16_t freq) {

	gpio_set_function(pin, GPIO_FUNC_PWM);

	uint slice_num = pwm_gpio_to_slice_num(pin);

	// math taken from https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
	
	uint32_t clock = 250000000;
	uint32_t divider16 = clock / freq / 4096 + 
		(clock % (freq * 4096) != 0);
	if (divider16 / 16 == 0)
 	divider16 = 16;

	uint32_t wrap = clock * 16 / divider16 / freq - 1;
	
	// this will cause incompatibilities with motors on the same slice that have different frequencies.
	// that configuration is not supported.
	//
	// (am i even going to support multiple configurations?)
	
	pwm_set_wrap(slice_num, wrap);
	pwm_set_clkdiv_int_frac(slice_num, divider16/16,
		divider16 & 0xF);

	struct motor* m;

	m = malloc(sizeof(struct motor));
	m->slice = slice_num;
	m->chan = pin%2;
	m->wrap = wrap;
	m->enabled = false;
	m->period = (1 / freq) * 10e9; // hz -> ns

	return m;
}

void motor_ns(Motor m, uint32_t duty_ns) {
	float duty_cycle = ((float)duty_ns / (float)m->period) * 100.0; // according to scout, this is in range (0, 50) aka (1m, 2m at 250hz (4m period))
	
	// todo: optimizeable perhapss
	// im targeting the completion of this project before the most optimal sol so idk if this is the best
	pwm_set_chan_level(m->slice, m->chan, m->wrap * (int)duty_cycle / 100);
  	if (!m->enabled) pwm_set_enabled(m->slice, 1);
}
