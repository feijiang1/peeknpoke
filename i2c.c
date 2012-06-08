/*
 * peeknpoke I2C access file
 *
 * Copyright (c) 2012, Intel Corporation.
 * Hari Kanigeri <hari.k.kanigeri@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "pnp_utils_inc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define I2C_DEV_PATH "/dev/i2c-%d"

struct i2c_message {
	uint16_t addr;	/*slave address*/
	uint16_t flags;
	uint16_t len;	/* msg length*/
	uint8_t *buf;	/* pointer to msg data */
};

#define MAX_SMBUS_BLOCK_SIZE 		32
#define DEF_I2C_SLAVE_FORCE			0x0706
#define DEF_I2C_SMBUS				0x0720

union i2c_smbus_data {
	uint8_t byte;
	uint16_t word;
	uint8_t block[MAX_SMBUS_BLOCK_SIZE + 2];
};

struct i2c_smbus_ioctl_data {
	char read_write;
	uint8_t command;
	int size;
	union i2c_smbus_data *data;
};

int write_i2c_device(char bus, int addr, int reg, int size, int value)
{
	struct i2c_smbus_ioctl_data args;
	union i2c_smbus_data data;
	char i2c_dev[15];
	int fd;
	int ret;

	snprintf(i2c_dev, sizeof(i2c_dev), I2C_DEV_PATH, bus);

	printf("%s\n", i2c_dev);


	fd = open(i2c_dev, O_RDWR | O_SYNC);
	printf("the value of /dev/i2c-2 is %d\n", fd);

	if (fd == -1) {
		printf("Error opening %d\n", fd);
		return -1;
	}

	ret = ioctl(fd,  DEF_I2C_SLAVE_FORCE, addr);
	if (fd == -1) {
		printf("Error with the ioctl for address 0x%x\n", addr);
		return -1;
	}

	switch (size) {
	case SMBUS_BYTE_DATA:
		data.byte = value & 0xff;
		break;
	case SMBUS_WORD_DATA:
		data.word = value & 0xffff;
		break;
#if 0
	case SMBUS_BLOCK_DATA:
		if (array_size > (MAX_SMBUS_BLOCK_SIZE + 2)) {
			printf("Error: data size for block transfer\n");
			return -1;
		}
		memcpy(value, args.data, array_size);
		break;
#endif
	default:
		printf("Error size provided %d\n", size);
		return -1;
	}

	args.read_write = 0;
	args.command = reg;
	args.size = size;
	args.data = &data;
	ret = ioctl(fd, DEF_I2C_SMBUS, &args);
	printf("Value written = 0x%x, return value = %d\n", data.word, ret);

	return ret;

}

int read_i2c_device(char bus, int addr, int reg, int size, int *result)
{

	struct i2c_smbus_ioctl_data args;
	union i2c_smbus_data data;
	char i2c_dev[15];
	int fd;
	int ret;
	int i;
	int num_values;

	if (result == NULL) {
		printf("invalid input buffer \n");
		return -1;
	}
	data.word = 0;


	snprintf(i2c_dev, sizeof(i2c_dev), I2C_DEV_PATH, bus);

	printf("%s\n", i2c_dev);


	fd = open(i2c_dev, O_RDWR | O_SYNC);
	printf("the value of /dev/i2c-%d is %d\n", bus, fd);

	if (fd == -1) {
		printf("Error opening %d\n", fd);
		return -1;
	}

	ret = ioctl(fd,  DEF_I2C_SLAVE_FORCE, addr);
	if (ret == -1) {
		printf("Error with the ioctl for address 0x%x\n", addr);
		return -1;
	}

	switch (size) {

	case SMBUS_BYTE_DATA:
	case SMBUS_WORD_DATA:
	//case SMBUS_BLOCK_DATA:
		args.size = size;
		break;

	default:
		printf("Error size provided %d\n", size);
		return -1;
	}

	args.read_write = 1;
	args.command = reg;
	args.data = &data;
	ret = ioctl(fd, DEF_I2C_SMBUS, &args);

	if (ret) {
		printf("Failed to communicate with Device: error = %d\n", ret);
		return ret;
	}

	if (size == SMBUS_WORD_DATA)
		*result = data.word;
	else if (size == SMBUS_BYTE_DATA)
		*result = data.byte;
	else if (size == SMBUS_BLOCK_DATA) {
		num_values = data.block[0];
		for (i = 1; i < num_values; i++)
			printf("value read = 0x%x\n", data.block[i]);
	}

	printf("Result = 0x%x, reg=0x%x\n", *result, reg);

	return 0;

}
