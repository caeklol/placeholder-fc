#include "hardware/i2c.h"

int mpu6050_init(int scl, int sda, i2c_inst_t* bus);
void mpu6050_read_gyro_raw(int16_t gyro[3]);
bool mpu6050_initialized();
