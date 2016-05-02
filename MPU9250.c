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
#include "i2c_master.h"
#include <avr/io.h>
#include <math.h>
#define READ_FLAG   0x80
#define SUPPORTS_AK89xx_HIGH_SENS   (0x10)
#define AKM_REG_WHOAMI      (0x00)

#define AKM_REG_ST1         (0x02)
#define AKM_REG_HXL         (0x03)
#define AKM_REG_ST2         (0x09)

#define AKM_REG_CNTL        (0x0A)
#define AKM_REG_ASTC        (0x0C)
#define AKM_REG_ASAX        (0x10)
#define AKM_REG_ASAY        (0x11)
#define AKM_REG_ASAZ        (0x12)

#define AKM_DATA_READY      (0x01)
#define AKM_DATA_OVERRUN    (0x02)
#define AKM_OVERFLOW        (0x80)
#define AKM_DATA_ERROR      (0x40)

#define AKM_BIT_SELF_TEST   (0x40)

#define AKM_POWER_DOWN          (0x00 | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_SINGLE_MEASUREMENT  (0x01 | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_FUSE_ROM_ACCESS     (0x0F | SUPPORTS_AK89xx_HIGH_SENS)
#define AKM_MODE_SELF_TEST      (0x08 | SUPPORTS_AK89xx_HIGH_SENS)

#define AKM_WHOAMI      (0x48)
#define i2c_delay_ctrl 0x67
	
//unsigned char compass_addr, compass_sample_rate;

void SPIinit_MPU(unsigned char sel, unsigned char A_range, unsigned char G_range){
	//unsigned char data[6];
	//uint8_t i;
	
	spi_writeReg(sel, MPU9250_PWR_MGMT_1, 0x80);
	spi_writeReg(sel, MPU9250_PWR_MGMT_1, 0x01);
	spi_writeReg(sel, MPU9250_PWR_MGMT_2, 0x00);
	spi_writeReg(sel, MPU9250_CONFIG, 0x01);

	spi_writeReg(sel, MPU9250_GYRO_CONFIG, G_range);

	spi_writeReg(sel, MPU9250_ACCEL_CONFIG, A_range);
	
	//BW:20Hz Delay:19.80ms 1kHz Rate 
	spi_writeReg(sel, MPU9250_ACCEL_CONFIG2, 0x09);
	
	spi_writeReg(sel, MPU9250_INT_PIN_CFG, 0x30);

	spi_writeReg(sel, MPU9250_USER_CTRL, 0x20);

	spi_writeReg(sel, MPU9250_I2C_MST_CTRL, 0x0D);

	spi_writeReg(sel, MPU9250_I2C_SLV0_ADDR, MPU9250_MAG_ADDRESS);
	spi_writeReg(sel, MPU9250_I2C_SLV0_REG, MPU9250_MAG_CNTL2);
	spi_writeReg(sel, MPU9250_I2C_SLV0_DO, 0x01);
	spi_writeReg(sel, MPU9250_I2C_SLV0_CTRL, 0x81);
	
	spi_writeReg(sel, MPU9250_I2C_SLV0_REG, MPU9250_MAG_CNTL1);
	spi_writeReg(sel, MPU9250_I2C_SLV0_DO, 0x12);
	spi_writeReg(sel, MPU9250_I2C_SLV0_CTRL, 0x81);
	set_acc_scale(sel, A_range);
	set_gyro_scale(sel, G_range);
	
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

spi_writeReg(sel, MPU9250_I2C_SLV0_ADDR, MPU9250_MAG_ADDRESS|READ_FLAG); //Set the I2C slave address of AK8963 and set for read.
spi_writeReg(sel, MPU9250_I2C_SLV0_REG, MPU9250_MAG_XOUT_L); //I2C slave 0 register address from where to begin data transfer
spi_writeReg(sel, MPU9250_I2C_SLV0_CTRL, 0x87);

_delay_ms(.1);
spi_readRegs(sel, MPU9250_EXT_SENS_DATA_00, 7, MagData);

data[0] = (((int16_t)MagData[1]) << 8) | MagData[0];
data[1] = (((int16_t)MagData[3]) << 8) | MagData[2];
data[2] = (((int16_t)MagData[5]) << 8) | MagData[4];
}


/**
 *  @brief      Read raw accel data directly from the registers.
 *  @param[out] data        Raw data in hardware units.
 *  @param[out] timestamp   Timestamp in milliseconds. Null if not needed.
 *  @return     0 if successful.
 */
int I2CgetAccel(unsigned char addr, short *data)
{
    unsigned char tmp[6];

    i2c_readReg(addr, MPU9250_GYRO_XOUT_H, 6, tmp);
   
    data[0] = (tmp[0] << 8) | tmp[1];
    data[1] = (tmp[2] << 8) | tmp[3];
    data[2] = (tmp[4] << 8) | tmp[5];
   
    return 0;
}

/**
 *  @brief      Read raw gyro data directly from the registers.
 *  @param[out] data        Raw data in hardware units.
 *  @param[out] timestamp   Timestamp in milliseconds. Null if not needed.
 *  @return     0 if successful.
 */
int I2CgetGyro(unsigned char addr, short *data)
{
    unsigned char tmp[6];

    i2c_readReg(addr, MPU9250_GYRO_XOUT_H, 6, tmp);
    
    data[0] = (tmp[0] << 8) | tmp[1];
    data[1] = (tmp[2] << 8) | tmp[3];
    data[2] = (tmp[4] << 8) | tmp[5];
    
    return 0;
}

//==============================================================================================//
//									              Who Am I?	//
//==============================================================================================//  
/*-----------------------------------------------------------------------------------------------
                                            WHO AM I?
usage: call this function to know if SPI is working correctly. It checks the I2C address of the
mpu9250 which should be 104 when in SPI mode.
returns the I2C address (104)
-----------------------------------------------------------------------------------------------*/
unsigned int whoami(unsigned char sel)
{
  unsigned int response;
  response=spi_writeReg(sel, MPU9250_WHO_AM_I|READ_FLAG, 0x00);
  return response;
}

uint8_t AK8963_whoami(unsigned char sel)
{
	uint8_t response;
	spi_writeReg(sel, MPU9250_I2C_SLV0_ADDR, MPU9250_MAG_ADDRESS|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
	//WriteReg(MPU9250_I2C_SLV0_ADDR,0x0|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
	spi_writeReg(sel, MPU9250_I2C_SLV0_REG, MPU9250_MAG_WIA); //I2C slave 0 register address from where to begin data transfer
	spi_writeReg(sel, MPU9250_I2C_SLV0_CTRL, 0x81); //Read 1 byte from the magnetometer
	
	//WriteReg(MPU9250_I2C_SLV0_CTRL, 0x81);    //Enable I2C and set bytes
	_delay_us(100);
	response=spi_writeReg(sel, MPU9250_EXT_SENS_DATA_00|READ_FLAG, 0x00);    //Read I2C
	//ReadRegs(MPU9250_EXT_SENS_DATA_00,response,1);
	//response=WriteReg(MPU9250_I2C_SLV0_DO, 0x00);    //Read I2C
	
	return response;
}

void readDatShit(unsigned char sel){
	uint8_t response[21];
	int16_t bit_data;
	
	float data;
	int i;
	
	//Send I2C command at first
	spi_writeReg(sel, MPU9250_I2C_SLV0_ADDR, MPU9250_MAG_ADDRESS|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
	spi_writeReg(sel, MPU9250_I2C_SLV0_REG, MPU9250_MAG_XOUT_L); //I2C slave 0 register address from where to begin data transfer
	spi_writeReg(sel, MPU9250_I2C_SLV0_CTRL, 0x87); //Read 7 bytes from the magnetometer
	//must start your read from AK8963A register 0x03 and read seven bytes so that upon read of ST2 register 0x09 the AK8963A will unlatch the data registers for the next measurement.
	
	//wait(0.001);
	_delay_ms(1);
	spi_readRegs(sel, MPU9250_ACCEL_XOUT_H, 21, response);
	
	//Get accelerometer value
	 for(i=0; i<3; i++)
	 {
		 bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
		 data=(float)bit_data;
		 Accel_data[i]=data/acc_divider;
	 }
	 //Get temperature
	 bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
	 data=(float)bit_data;
	 Temp=((data-21)/333.87)+21;
	 //Get gyroscope value
	 for(i=4; i<7; i++)
	 {
		 bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
		 data=(float)bit_data;
		 Gyro_data[i-4]=data/gyro_divider;
	 }
	 //Get Magnetometer value
	 for(i=7; i<10; i++)
	 {
		 bit_data=((int16_t)response[i*2+1]<<8)|response[i*2];
		 data=(float)bit_data;
		 Mag_data[i-7]=data;
	 }
}

/*-----------------------------------------------------------------------------------------------
                                        ACCELEROMETER SCALE
usage: call this function at startup, after initialization, to set the right range for the
accelerometers. Suitable ranges are:
BITS_FS_2G
BITS_FS_4G
BITS_FS_8G
BITS_FS_16G
returns the range set (2, 4, 8, or 16)
-----------------------------------------------------------------------------------------------*/
unsigned int set_acc_scale(unsigned char sel, int scale){
  unsigned int temp_scale;
  spi_writeReg(sel, MPU9250_ACCEL_CONFIG, scale);
    
  switch (scale)
  {
    case BITS_FS_2G:
      acc_divider=16384;
    break;
    case BITS_FS_4G:
      acc_divider=8192;
    break;
    case BITS_FS_8G:
      acc_divider=4096;
    break;
    case BITS_FS_16G:
      acc_divider=2048;
    break;   
  }
 
  temp_scale=spi_writeReg(sel, MPU9250_ACCEL_CONFIG|READ_FLAG, 0x00);
    
  switch (temp_scale)
  {
    case BITS_FS_2G:
      temp_scale=2;
    break;
    case BITS_FS_4G:
      temp_scale=4;
    break;
    case BITS_FS_8G:
      temp_scale=8;
    break;
    case BITS_FS_16G:
      temp_scale=16;
    break;   
  }
  return temp_scale;
}

//==============================================================================================//
//									        Gyroscope Scale	//
//==============================================================================================// 
/*-----------------------------------------------------------------------------------------------
                                        GYROSCOPE SCALE
usage: call this function at startup, after initialization, to set the right range for the
gyroscopes. Suitable ranges are:
BITS_FS_250DPS
BITS_FS_500DPS
BITS_FS_1000DPS
BITS_FS_2000DPS
returns the range set (250, 500, 1000 or 2000)
-----------------------------------------------------------------------------------------------*/
unsigned int set_gyro_scale(unsigned char sel, int scale)
{
  unsigned int temp_scale;
  spi_writeReg(sel, MPU9250_GYRO_CONFIG, scale);
  switch (scale)
  {
    case BITS_FS_250DPS:
      gyro_divider=131;
    break;
    case BITS_FS_500DPS:
      gyro_divider=65.5;
    break;
    case BITS_FS_1000DPS:
      gyro_divider=32.8;
    break;
    case BITS_FS_2000DPS:
      gyro_divider=16.4;
    break;   
  }
  
  temp_scale=spi_writeReg(sel, MPU9250_GYRO_CONFIG|READ_FLAG, 0x00);
  
  switch (temp_scale)
  {
    case BITS_FS_250DPS:
      temp_scale=250;
    break;
    case BITS_FS_500DPS:
      temp_scale=500;
    break;
    case BITS_FS_1000DPS:
      temp_scale=1000;
    break;
    case BITS_FS_2000DPS:
      temp_scale=2000;
    break;   
  }
  return temp_scale;
}
