#include "stdio.h"

#include "pico/stdlib.h"
#include "pico/time.h"
#include "time.h"


#include "hardware/i2c.h"
#include "util.h"

static int MPU6050_ADDR = 0x68;
static bool MPU6050_INIT = false;
static i2c_inst_t* MPU6050_BUS = {0};
static float MPU6050_BIAS[3];

int mpu6050_init(int scl, int sda, i2c_inst_t* bus) {
	if (MPU6050_INIT) {
		return fatal_error("mpu6050: init called twice?");
	}

	MPU6050_BUS = bus;

	// init i2c bus
    i2c_init(bus, 400 * 1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);

	/// taken from scout

	// init
	uint8_t buf[2];
	buf[0] = 0x6B;
	buf[1] = 0x01;
    i2c_write_blocking(i2c_default, MPU6050_ADDR, buf, 2, false);

	// low pass filter
	buf[0] = 0x1A;
	buf[1] = 0x05;
    i2c_write_blocking(i2c_default, MPU6050_ADDR, buf, 2, false);

	// gyro scale
	buf[0] = 0x1B;
	buf[1] = 0x08;
    i2c_write_blocking(i2c_default, MPU6050_ADDR, buf, 2, false);

	/// verify if suceeded
	
	uint8_t data[1];
	uint8_t reg;

	// whoami
	reg = 0x75;
	i2c_write_blocking(i2c_default, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_ADDR, data, 1, false);
	if (data[0] != 0x68) {
		return fatal_error("mpu6050: whoami failed");
	}

	// lpf
	reg = 0x1A;
	i2c_write_blocking(i2c_default, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_ADDR, data, 1, false);
	if (data[0] != 0x05) {
		return fatal_error("mpu6050: lpf failed");
	}

	// gyro scale
	reg = 0x1B;
	i2c_write_blocking(i2c_default, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, MPU6050_ADDR, data, 1, false);
	if (data[0] != 0x08) {
		return fatal_error("mpu6050: gyro scale failed");
	}

	MPU6050_INIT = 1;
	return 0;
}

void mpu6050_read_gyro_raw(int16_t gyro[3]) {
	uint8_t reg = 0x43;
	uint8_t buf[6];

	i2c_write_blocking(MPU6050_BUS, MPU6050_ADDR, &reg, 1, true);
	i2c_read_blocking(MPU6050_BUS, MPU6050_ADDR, buf, 6, false);

	for (int i = 0; i < 3; i++) {
        gyro[i] = (buf[i * 2] << 8 | buf[(i * 2) + 1]);;
    }
}

void mpu6050_calibrate_gyro() {
	/// taken from scout
	int32_t raw_gx = 0;
	int32_t raw_gy = 0;
	int32_t raw_gz = 0;
	int iters = 0;

	clock_t t = clock();

	while (((double)(clock()-t))/CLOCKS_PER_SEC < 3) {
		int16_t	buf[3];

		mpu6050_read_gyro_raw(buf);
		
		raw_gx += buf[0];
		raw_gy += buf[1];
		raw_gz += buf[2];

		iters++;

		sleep_ms(26);
	}

	float gx = raw_gx / 65.5F;
	float gy = raw_gy / 65.5F;
	float gz = raw_gz / 65.5F;

	MPU6050_BIAS[0] = gx / iters;
	MPU6050_BIAS[1] = gy / iters;
	MPU6050_BIAS[2] = gz / iters;
}

void mpu6050_read_gyro(float gyro[3]) {
	int16_t tmp[3];
	mpu6050_read_gyro_raw(tmp);

	gyro[0] = (float)(tmp[0] / 65.5F) - MPU6050_BIAS[0];
	gyro[1] = (float)(tmp[1] / 65.5F) - MPU6050_BIAS[1];
	gyro[2] = (float)(tmp[2] / 65.5F) - MPU6050_BIAS[2];
}
