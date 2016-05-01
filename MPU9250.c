/*
 * MPU9250TWI.cpp
 *
 * Library for MPU9250 I2C TW Interface
 * Created: 3/5/2016 5:03:31 PM
 *  Author: Kody Conrad
 */
#include <stdint.h>
#include <util/delay.h>
#include "MPU9250.h"
#include "SPI.h"
#include <avr/io.h>
#include <math.h>
#define READ_FLAG   0x80

int16_t AG_SelfTest[6]; //Holds the self test results for accelerometer/Gyro
int16_t magCalib[3]={0,0,0}, magBias[3]={0,0,0};
int16_t accBias[3]={0,0,0}, gyroBias[3]={0,0,0};

void SPIinit_MPU(unsigned char sel, unsigned char A_range, unsigned char G_range){
	unsigned char data[6];
	//uint8_t i;
	
	data[0] = 0x80;
	spi_writeRegs(sel, MPU9250_PWR_MGMT_1, 1, data);
	data[0] = 0x01;
	spi_writeRegs(sel, MPU9250_PWR_MGMT_1, 1, data);
	data[0] = 0x00;
	spi_writeRegs(sel, MPU9250_PWR_MGMT_2, 1, data);
	data[0] = 0x01;
	spi_writeRegs(sel, MPU9250_CONFIG, 1, data);
	data[0] = 0x01;
	spi_writeRegs(sel, MPU9250_USER_CTRL, 1, data);
	data[0] = G_range;
	spi_writeRegs(sel, MPU9250_GYRO_CONFIG, 1, data);
	data[0] = A_range;
	spi_writeRegs(sel, MPU9250_ACCEL_CONFIG, 1, data);
	data[0] = 0x09;
	spi_writeRegs(sel, MPU9250_ACCEL_CONFIG2, 1, data);
	/*data[0] = 0x02;
	spi_writeRegs(sel, MPU9250_INT_PIN_CFG, 1, data);
	data[0] = 0X01;
	spi_writeRegs(sel, MPU9250_MAG_CNTL2, 1, data);
	data[0] = 0X12;
	spi_writeRegs(sel, MPU9250_MAG_CNTL1, 1, data);
	data[0] = 0X08;
	spi_writeRegs(sel, MPU9250_I2C_MST_CTRL, 1, data);*/
	
	data[0] = 0x20;
	spi_writeRegs(sel, MPU9250_USER_CTRL, 1, data);
	data[0] = 0x0D;
	spi_writeRegs(sel, MPU9250_I2C_MST_CTRL, 1, data);
	data[0] = MPU9250_MAG_ADDRESS;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_ADDR, 1, data);
	data[0] = MPU9250_MAG_CNTL2;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_REG, 1, data);
	data[0] = 0X01;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_DO, 1, data);
	data[0] = 0X81;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_CTRL, 1, data);
	data[0] = MPU9250_MAG_CNTL1;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_REG, 1, data);
	data[0] = 0X12;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_DO, 1, data);
	data[0] = 0X81;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_CTRL, 1, data);
	
	return;
}

void SPIgetAccel(short* data, unsigned char sel){
	uint8_t AccelData[6];
	
	spi_readRegs(sel, MPU9250_ACCEL_XOUT_H, 6, AccelData);
	
	data[0] = (((int16_t)AccelData[0]) << 8) | AccelData[1];
	data[1] = (((int16_t)AccelData[2]) << 8) | AccelData[3];
	data[2] = (((int16_t)AccelData[4]) << 8) | AccelData[5];
	return;
}

void SPIgetGyro(short* data, unsigned char sel){
	uint8_t GyroData[6];
	
	spi_readRegs(sel, MPU9250_GYRO_XOUT_H, 6, GyroData);
	
	data[0] = (((int16_t)GyroData[0]) << 8) | GyroData[1];
	data[1] = (((int16_t)GyroData[2]) << 8) | GyroData[3];
	data[2] = (((int16_t)GyroData[4]) << 8) | GyroData[5];
	return;
}

void SPIgetMag(short* data, unsigned char sel){
//uint8_t ST1[2];
uint8_t MagData[7];

MagData[0]=MPU9250_MAG_ADDRESS|READ_FLAG;
spi_writeRegs(sel, MPU9250_I2C_SLV0_ADDR, 1, MagData); //Set the I2C slave addres of AK8963 and set for read.
MagData[0]=MPU9250_MAG_XOUT_L;
spi_writeRegs(sel, MPU9250_I2C_SLV0_REG, 1, MagData); //I2C slave 0 register address from where to begin data transfer
MagData[0]=0x87;
spi_writeRegs(sel, MPU9250_I2C_SLV0_CTRL, 1, MagData);

_delay_ms(1);
spi_readRegs(sel, MPU9250_EXT_SENS_DATA_00, 7, MagData);

data[0] = (((int16_t)MagData[1]) << 8) | MagData[0];
data[1] = (((int16_t)MagData[3]) << 8) | MagData[2];
data[2] = (((int16_t)MagData[5]) << 8) | MagData[4];
}


int magwhoami(uint8_t sel){
	unsigned char response[3];
	unsigned char data[3];
	data[0] = MPU9250_MAG_ADDRESS|READ_FLAG;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_ADDR, 1, data);
	data[0] = MPU9250_MAG_WIA;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_REG, 1, data);
	data[0] = 0x81;
	spi_writeRegs(sel, MPU9250_I2C_SLV0_CTRL, 1, data);
	
	data[0]=0x00;
	spi_writeRegs(sel, MPU9250_EXT_SENS_DATA_00|READ_FLAG, 1, data);
	
	//spi_readRegs(sel, MPU9250)
	return 0;
}