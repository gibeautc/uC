/*
 * Sensor_Board.cpp
 *
 * Created: 4/15/2016 5:08:41 PM
 * Author : gbone
 */ 
#ifndef F_CPU
#define F_CPU 8000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>
#include "MPU9250.h"
#include "Feedback.h"
#include "SPI.h"
#include "UART.h"
#include "asf.h"
#define BUFSZ 128

volatile unsigned long count_t=0;

ISR(TIMER2_OVF_vect)
{
	count_t++;
}

int main(void)
{
	_delay_ms(10);
	float ax,ay,az;
	float gx,gy,gz;
	float mx,my,mz;
	uint8_t response[4];
	char buf[BUFSZ];
	
	sei();
	init_SPI();
	_delay_us(10);
	init_Feedback();
	uart_init();
	init_tcnt2();
	_delay_ms(10);
	//mpu_init(NULL);
	
	SPIinit_MPU(sensor1_cs, 1, BITS_DLPF_CFG_188HZ);
	//Initializations Successful
	LED(G_LED, Pulse_3);
	
	response[0] = whoami(sensor1_cs);
	_delay_ms(10);
	response[2] = set_gyro_scale(sensor1_cs, BITS_FS_2000DPS);
	_delay_ms(10);
	response[3] = set_acc_scale(sensor1_cs, BITS_FS_16G);
	_delay_ms(10);
	response[1] = AK8963_whoami(sensor1_cs);
	_delay_ms(10);
	
	//OUTPUT: WHOAMI Results & GYRO/ACC Scale
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*TMPU9250 Addr=%02X  AK8963 Addr=%02X \n Gyro Scale=%d  Accel Scale=%d*\n\n", 
		response[0], response[1], response[2], response[3]);
	uart_puts(buf);
	
	_delay_ms(1);
	AK8963_calib_Magnetometer(sensor1_cs);
	_delay_ms(5000);
	
    while (1) 
    {
		_delay_ms(100);
		read_all(sensor1_cs);
		snprintf(buf, BUFSZ, "*TTemperature=%f*\n",	Temp);
		uart_puts(buf);
		
		ax = Accel_data[0];
		ay = Accel_data[1];
		az = Accel_data[2];
		
		gx = Gyro_data[0];
		gy = Gyro_data[1];
		gz = Gyro_data[2];
		
		mx = Mag_data[0];
		my = Mag_data[1];
		mz = Mag_data[2];
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*HX%luY%f,X%luY%f,X%luY%f*\n", count_t, ax, count_t, ay, count_t, az);
		uart_puts(buf);
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*JX%luY%f,X%luY%f,X%luY%f*\n", count_t, gx, count_t, gy, count_t, gz);
		uart_puts(buf);
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*KX%luY%f,X%luY%f,X%luY%f*\n", count_t, mx, count_t, my, count_t, mz);
		uart_puts(buf);
		_delay_ms(30);
    }
	return 0;
}

