/*
 * MPU9250TWI.cpp
 *
 * Library for MPU9250 I2C TW Interface
 * Created: 3/5/2016 5:03:31 PM
 *  Author: Kody Conrad
 */
#include <stdint.h>
#include "twi_master.h"
#include "MPU9250.h"

void writeReg(const uint8_t reg_addr, const uint8_t val){
	uint8_t dataArr[2];
	dataArr[0] = reg_addr;
	dataArr[1] = val;
	twi_start_wr(MPU9250_DEFAULT_ADDRESS, dataArr, 2);
	return;
}

void readReg(const uint8_t reg_addr_strt, const uint8_t bytes, uint8_t *buf){
	//uint8_t databuf[bytes];
	uint8_t reg[1];
	reg[0]=reg_addr_strt;
	twi_start_wr(MPU9250_DEFAULT_ADDRESS, reg, 1);
	while(twi_busy()){};
	twi_start_rd(MPU9250_DEFAULT_ADDRESS, buf, bytes);
	return;
}

void writeMagReg(const uint8_t reg_addr, const uint8_t val){
	uint8_t dataArr[2];
	dataArr[0] = reg_addr;
	dataArr[1] = val;
	twi_start_wr(MPU9250_MAG_ADDRESS, dataArr, 2);
	return;
}

void readMagReg(const uint8_t reg_addr_strt, const uint8_t bytes, uint8_t *buf){
	uint8_t reg[1];
	reg[0]=reg_addr_strt;
	twi_start_wr(MPU9250_MAG_ADDRESS, reg, 1);
	while(twi_busy()){};
	twi_start_rd(MPU9250_MAG_ADDRESS, buf, bytes);
	return;
}

void getAccel(int16_t* ax, int16_t* ay, int16_t* az){
	uint8_t AccelData[6];
	
	readReg(MPU9250_ACCEL_XOUT_H, 6, AccelData);
	
	*ax = (((int16_t)AccelData[0]) << 8) | AccelData[1];
	*ay = (((int16_t)AccelData[2]) << 8) | AccelData[3];
	*az = (((int16_t)AccelData[4]) << 8) | AccelData[5];
	return;
}
void getGyro(int16_t* gx, int16_t* gy, int16_t* gz){
	uint8_t GyroData[6];
	
	readReg(MPU9250_ACCEL_XOUT_H, 6, GyroData);
	
	*gx = (((int16_t)GyroData[0]) << 8) | GyroData[1];
	*gy = (((int16_t)GyroData[2]) << 8) | GyroData[3];
	*gz = (((int16_t)GyroData[4]) << 8) | GyroData[5];
	return;
}

void getMag(int16_t* mx, int16_t* my, int16_t* mz){
	uint8_t ST1[1];
	uint8_t MagData[6];

	//If problems occur possibly look at changing ST1[1] -> ST1 and then using readMagReg(MPU9250_MAG_ST1, 1, &ST1)
	//do
	//{//
	//	readMagReg(MPU9250_MAG_ST1, 1, ST1); //Looking for data ready
	//}
	//while (!(ST1[0]&0x01));
	
	// Read magnetometer data
	
	readMagReg(MPU9250_MAG_XOUT_L, 6, MagData);
	
	*mx = (((int16_t)MagData[1]) << 8) | MagData[0];
	*my = (((int16_t)MagData[3]) << 8) | MagData[2];
	*mz = (((int16_t)MagData[5]) << 8) | MagData[4];
}

void init_MPU(const uint8_t A_range, const uint8_t G_range){
	//Global Setup:
	writeReg(MPU9250_PWR_MGMT_1, 0x00); //Set internal oscillator
	writeReg(MPU9250_PWR_MGMT_2, 0x00); //Enable all Axis-Gyro/Accel
	//Configure Gyroscope:
	writeReg(MPU9250_GYRO_CONFIG, G_range);
	writeReg(MPU9250_CONFIG, 0x06); //Low pass 5Hz Gyro 33.48ms Delay
	//Configure Accelerometer:
	writeReg(MPU9250_ACCEL_CONFIG, A_range);
	writeReg(MPU9250_ACCEL_CONFIG2, 0x06); //Low pass 5Hz Accel 66.96ms Delay
	//Configure Magnetometer:
	//writeReg(MPU9250_INT_PIN_CFG, 0x02);
	writeMagReg(MPU9250_MAG_CNTL2, 0x01);
	writeMagReg(MPU9250_MAG_CNTL1, 0x12);
}
