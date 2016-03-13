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
#include <avr/io.h>
#include <math.h>
#define readbit 0x80

int16_t AG_SelfTest[6]; //Holds the self test results for accelerometer/Gyro
int16_t magCalib[3]={0,0,0}, magBias[3]={0,0,0};
int16_t accBias[3]={0,0,0}, gyroBias[3]={0,0,0};
/*
	MPU Master i2c mode test functions for Magnetometer.
*/
enum SPI{
	SPI_Dis=0,
	SPI_En=1// Set initial input parameters
};

enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};

enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

enum Mscale {
  MFS_14BITS = 0, // 0.6 mG per LSB
  MFS_16BITS      // 0.15 mG per LSB
};

uint8_t test_com(uint8_t SPI_Enable, const uint8_t SSel){
	uint8_t response[1];
	switch (SPI_Enable)
	{
	case SPI_Dis:
		readReg(0x75 , 1 , response, SSel);
		if(response[0] == 0x71)
			return 1;
		else
			return 0;	
	break;
	case SPI_En:
		SPIreadRegs(0x75 , 1 , response);	
			if(response[0] == 0x71)
			return 1;
		else
			return 0;
	break;
	}
	return 0;
}

//differences is a percent off factory set calibration;
//differences: should be array of 6 floats.
void selfTest(float* differences, uint8_t SSel){
	uint8_t rawData[6] = {0,0,0,0,0,0};
	uint8_t selfTestAcc[3], selfTestGyro[3];
	int16_t gAvg[3], aAvg[3], gSTAvg[3], aSTAvg[3];
	float factoryTrim[6];
	int i = 0;
	int FS=0;
	//SETUP SENSOR FOR TESTING
	writeReg(MPU9250_SMPLRT_DIV, 0x00, SSel);
	writeReg(MPU9250_CONFIG, 0x02, SSel);//92HZ
	writeReg(MPU9250_GYRO_CONFIG, 0x01, SSel);//250DPS
	writeReg(MPU9250_ACCEL_CONFIG2, 0x02, SSel);//1kHz rate @ 92Hz bandwidth
	writeReg(MPU9250_ACCEL_CONFIG, 0x01, SSel);//2g accelerometer
	
	for(i=0; i<200; i++)
	{
		readReg(MPU9250_ACCEL_XOUT_H, 6, rawData, SSel);
		aAvg[0]	+= (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]);
		aAvg[1]	+= (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
		aAvg[2]	+= (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
	
		readReg(MPU9250_GYRO_XOUT_H, 6, rawData, SSel);
		gAvg[0]	+= (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]);
		gAvg[1]	+= (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
		gAvg[2]	+= (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
	} 

	for(i=0; i<3; i++){
		aAvg[i] /= 200;
		gAvg[i] /= 200;
	}
	
	//setup gyro/accel for selftest
	writeReg(MPU9250_ACCEL_CONFIG, 0xE0, SSel);
	writeReg(MPU9250_GYRO_CONFIG, 0xE0, SSel);
	
	for(i=0; i<200; i++)
	{
		readReg(MPU9250_ACCEL_XOUT_H, 6, rawData, SSel);
		aSTAvg[0]+= (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]);
		aSTAvg[1]+= (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
		aSTAvg[2]+= (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
	
		readReg(MPU9250_GYRO_XOUT_H, 6, rawData, SSel);
		gSTAvg[0]+= (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]);
		gSTAvg[1]+= (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
		gSTAvg[2]+= (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
	} 

	for(i=0; i<3; i++){
		aSTAvg[i] /= 200;
		gSTAvg[i] /= 200;
	}
	
	//setup gyro/accel for selftest
	writeReg(MPU9250_ACCEL_CONFIG, 0x00, SSel);
	writeReg(MPU9250_GYRO_CONFIG, 0x00, SSel);

	// Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
	readReg(MPU9250_SELF_TEST_X_ACCEL, 3, selfTestAcc, SSel); // X-axis accel self-test results
	readReg(MPU9250_SELF_TEST_X_GYRO, 3, selfTestGyro, SSel); // X-axis accel self-test results

	// Retrieve factory self-test value from self-test code reads
	//FT[Xa] factory trim calculation
	factoryTrim[0] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestAcc[0] - 1.0) )); 
	// FT[Ya] factory trim calculation
	factoryTrim[1] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestAcc[1] - 1.0) )); 
	// FT[Za] factory trim calculation
	factoryTrim[2] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestAcc[2] - 1.0) ));
	// FT[Xg] factory trim calculation
	factoryTrim[3] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestGyro[0] - 1.0) ));
	// FT[Yg] factory trim calculation
	factoryTrim[4] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestGyro[1] - 1.0) ));
	// FT[Zg] factory trim calculation
	factoryTrim[5] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTestGyro[2] - 1.0) ));

	// Report results as a ratio of (STR - FT)/FT; the change from Factory Trim of the Self-Test Response
	// To get percent, must multiply by 100
	for (i = 0; i < 3; i++) {
		differences[i]   = 100.0*((float)(aSTAvg[i] - aAvg[i]))/factoryTrim[i];   //percent differences
		differences[i+3] = 100.0*((float)(gSTAvg[i] - gAvg[i]))/factoryTrim[i+3]; //percent differences
	}
}

void writeReg(const uint8_t reg_addr, const uint8_t val, uint8_t SSel){
	uint8_t dataArr[2];
	dataArr[0] = reg_addr;
	dataArr[1] = val;
	twi_start_wr(SSel, dataArr, 2);
	while(twi_busy()){}
	return;
}

void readReg(const uint8_t reg_addr_strt, const uint8_t bytes, uint8_t *buf, uint8_t SSel){
	//uint8_t databuf[bytes];
	uint8_t reg[1];
	reg[0]=reg_addr_strt;
	twi_start_wr(SSel, reg, 1);
	while(twi_busy()){};
	twi_start_rd(SSel, buf, bytes);
	return;
}
//Write=0MSB Read=1MSB
void SPIwriteReg(const uint8_t reg_addr, const uint8_t val){
	
        SPDR=reg_addr;
	while(bit_is_clear(SPSR,SPIF)){}
	SPDR=val;
	while(bit_is_clear(SPSR,SPIF)){}
	return;	
}

void SPIreadReg(const uint8_t reg_addr, uint8_t *data){
	SPDR=(reg_addr | readbit);
	while(bit_is_clear(SPSR,SPIF)){};
	SPDR=0xFF;
	while(bit_is_clear(SPSR,SPIF)){};
	data[0]=SPDR;
	return;
}

void SPIreadRegs(const uint8_t reg_addr, const uint8_t bytes, uint8_t *data){
	int i=0;
	SPDR=(reg_addr | readbit);
	while(bit_is_clear(SPSR,SPIF)){};
	SPDR=0xFF;
	while(bit_is_clear(SPSR,SPIF)){};
	for(i=0; i<bytes; i++)
	{
		data[i]=SPDR;
		SPDR=0xFF;
		while(bit_is_clear(SPSR,SPIF)){};
	}	
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

void getAccel(int16_t* ax, int16_t* ay, int16_t* az, uint8_t SSel){
	uint8_t AccelData[6];
	
	readReg(MPU9250_ACCEL_XOUT_H, 6, AccelData, SSel);
	
	*ax = (((int16_t)AccelData[0]) << 8) | AccelData[1];
	*ay = (((int16_t)AccelData[2]) << 8) | AccelData[3];
	*az = (((int16_t)AccelData[4]) << 8) | AccelData[5];
	return;
}

void SPIgetAccel(int16_t* ax, int16_t* ay, int16_t* az){
	uint8_t AccelData[6];
	
	SPIreadRegs(MPU9250_ACCEL_XOUT_H, 6, AccelData);
	
	*ax = (((int16_t)AccelData[0]) << 8) | AccelData[1];
	*ay = (((int16_t)AccelData[2]) << 8) | AccelData[3];
	*az = (((int16_t)AccelData[4]) << 8) | AccelData[5];
	return;
}

void getGyro(int16_t* gx, int16_t* gy, int16_t* gz, uint8_t SSel){
	uint8_t GyroData[6];
	
	readReg(MPU9250_ACCEL_XOUT_H, 6, GyroData, SSel);
	
	*gx = (((int16_t)GyroData[0]) << 8) | GyroData[1];
	*gy = (((int16_t)GyroData[2]) << 8) | GyroData[3];
	*gz = (((int16_t)GyroData[4]) << 8) | GyroData[5];
	return;
}

void SPIgetGyro(int16_t* gx, int16_t* gy, int16_t* gz){
	uint8_t GyroData[6];
	
	SPIreadRegs(MPU9250_ACCEL_XOUT_H, 6, GyroData);
	
	*gx = (((int16_t)GyroData[0]) << 8) | GyroData[1];
	*gy = (((int16_t)GyroData[2]) << 8) | GyroData[3];
	*gz = (((int16_t)GyroData[4]) << 8) | GyroData[5];
	return;
}

void getMag(int16_t* mx, int16_t* my, int16_t* mz){
	//uint8_t ST1[1];
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

void init_MPU(const uint8_t A_range, const uint8_t G_range, uint8_t SSel){
	writeReg(MPU9250_PWR_MGMT_1, 0x80, SSel);
/*	writeReg(MPU9250_PWR_MGMT_1, 0x01, SSel);
	writeReg(MPU9250_PWR_MGMT_2, 0x00, SSel);
	writeReg(MPU9250_CONFIG, 0x01, SSel);
	writeReg(MPU9250_USER_CTRL, 0x01, SSel);
	writeReg(MPU9250_GYRO_CONFIG, G_range, SSel);
	writeReg(MPU9250_ACCEL_CONFIG, A_range, SSel);
	writeReg(MPU9250_ACCEL_CONFIG2, 0x09, SSel);
	writeReg(0x37, 0x30, SSel);//int_pin_cfg(register) i2c_lost_arb enable/i2c_slv4_nack enablea
	writeReg(0x6A, 0x20, SSel);//user_ctrl
	writeReg(0x24, 0x0D, SSel);//i2c_mst_ctrl
	writeReg(0x25, MPU9250_MAG_ADDRESS, SSel);//slv0_addr
	writeReg(0x26, MPU9250_MAG_CNTL2, SSel);//slv0_reg
	writeReg(0x63, 0x01, SSel); //slv0_DO
	writeReg(0x27, 0x81, SSel);//slv0_ctrl  enable i2c/set 1 byte
	writeReg(0x26, MPU9250_MAG_CNTL1, SSel);
	writeReg(0x63, 0x12, SSel);
	writeReg(0x27, 0x81, SSel);
	return;*/

	//Global Setup:
	writeReg(MPU9250_PWR_MGMT_1, 0x00, SSel); //Set internal oscillator
	writeReg(MPU9250_PWR_MGMT_2, 0x00, SSel); //Enable all Axis-Gyro/Accel
	//Configure Gyroscope:
	writeReg(MPU9250_GYRO_CONFIG, G_range, SSel);
	writeReg(MPU9250_CONFIG, 0x06, SSel); //Low pass 5Hz Gyro 33.48ms Delay
	//Configure Accelerometer:
	writeReg(MPU9250_ACCEL_CONFIG, A_range, SSel);
	writeReg(MPU9250_ACCEL_CONFIG2, 0x06, SSel); //Low pass 5Hz Accel 66.96ms Delay
	//Configure Magnetometer:
	//writeReg(MPU9250_INT_PIN_CFG, 0x02);
	writeMagReg(MPU9250_MAG_CNTL2, 0x01);
	writeMagReg(MPU9250_MAG_CNTL1, 0x12); //Old Initilization
	return;
}

void SPIinit_MPU(const uint8_t A_range, const uint8_t G_range){
	SPIwriteReg(MPU9250_PWR_MGMT_1, 0x80);
	SPIwriteReg(MPU9250_PWR_MGMT_1, 0x01);
	SPIwriteReg(MPU9250_PWR_MGMT_2, 0x00);
	SPIwriteReg(MPU9250_CONFIG, 0x01);
	SPIwriteReg(MPU9250_USER_CTRL, 0x01);
	SPIwriteReg(MPU9250_GYRO_CONFIG, G_range);
	SPIwriteReg(MPU9250_ACCEL_CONFIG, A_range);
	SPIwriteReg(MPU9250_ACCEL_CONFIG2, 0x09);
	SPIwriteReg(0x37, 0x30);//int_pin_cfg(register) i2c_lost_arb enable/i2c_slv4_nack enablea
	SPIwriteReg(0x6A, 0x20);//user_ctrl
	SPIwriteReg(0x24, 0x0D);//i2c_mst_ctrl
	SPIwriteReg(0x25, MPU9250_MAG_ADDRESS);//slv0_addr
	SPIwriteReg(0x26, MPU9250_MAG_CNTL2);//slv0_reg
	SPIwriteReg(0x63, 0x01); //slv0_DO
	SPIwriteReg(0x27, 0x81);//slv0_ctrl  enable i2c/set 1 byte
	SPIwriteReg(0x26, MPU9250_MAG_CNTL1);
	SPIwriteReg(0x63, 0x12);
	SPIwriteReg(0x27, 0x81);
	return;
}

void mpu_AG_Calibration(){

}
