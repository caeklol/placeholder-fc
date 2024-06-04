#include "pico/stdlib.h"

typedef struct motor* Motor;

Motor motor_init(uint pin, uint16_t freq);
void motor_speed(Motor m, float speed);