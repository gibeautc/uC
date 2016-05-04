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
#include "POS_FUNCTIONS.h"
#define BUFSZ 128

volatile unsigned long count_t=0;

ISR(TIMER2_OVF_vect)
{
	count_t++;
}

int main(void)
{
	_delay_ms(10);
	uint8_t response[4];
	char buf[BUFSZ];
	
	sei();
	init_SPI();
	_delay_us(10);
	init_Feedback();
	uart_init();
	init_tcnt2();
	
	_delay_ms(10);
	SPIinit_MPU(sensor1_cs, 1, BITS_DLPF_CFG_188HZ);
	_delay_ms(10);
	SPIinit_MPU(sensor2_cs, 1, BITS_DLPF_CFG_188HZ);
	_delay_ms(10);
	SPIinit_MPU(sensor3_cs, 1, BITS_DLPF_CFG_188HZ);
	
	//Initializations Successful
	LED(G_LED, Pulse_3);

	_delay_ms(10);
	response[2] = set_gyro_scale(sensor1_cs, BITS_FS_2000DPS);
	_delay_ms(10);
	response[3] = set_acc_scale(sensor1_cs, BITS_FS_16G);
	_delay_ms(10);
	
	response[2] = set_gyro_scale(sensor2_cs, BITS_FS_2000DPS);
	_delay_ms(10);
	response[3] = set_acc_scale(sensor2_cs, BITS_FS_16G); 
	_delay_ms(10);
	
	response[2] = set_gyro_scale(sensor3_cs, BITS_FS_2000DPS);
	_delay_ms(10);
	response[3] = set_acc_scale(sensor3_cs, BITS_FS_16G);
	_delay_ms(10);

	uart_puts("*HC**JC**KC*");
	
	LED(G_LED, Pulse_Long);
	
	//OUTPUT: WHOAMI Results & GYRO/ACC Scale
	/*(memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*TMPU9250 Addr=%02X  AK8963 Addr=%02X \n Gyro Scale=%d  Accel Scale=%d*\n\n", 
		response[0], response[1], response[2], response[3]);
	uart_puts(buf);*/

	AK8963_calib_Magnetometer(sensor1_cs);
	_delay_ms(10);
	AK8963_calib_Magnetometer(sensor2_cs);
	_delay_ms(10);
	AK8963_calib_Magnetometer(sensor3_cs);
	_delay_ms(10);
	
	
	_delay_ms(2000);
	
	while(1)
	{
		if(uart_getc()=='S'){record_window();}
		if(uart_getc()=='P'){live_pos();}
	}
	
    while (1) 
    {
		if(count_t >20000)
			count_t=0;
		_delay_ms(100);
		read_all(sensor3_cs);
		//snprintf(buf, BUFSZ, "*TTemperature=%f*\n",	Temp);
		//uart_puts(buf);
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*HX%luY%f,X%luY%f,X%luY%f*\n", count_t, Accel_data[0], count_t, Accel_data[1], count_t, Accel_data[2]);
		uart_puts(buf);
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*JX%luY%f,X%luY%f,X%luY%f*\n", count_t, Gyro_data[0], count_t, Gyro_data[1], count_t, Gyro_data[2]);
		uart_puts(buf);
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*KX%luY%f,X%luY%f,X%luY%f*\n", count_t, Mag_data[0], count_t, Mag_data[1], count_t, Mag_data[2]);
		uart_puts(buf);
		_delay_ms(30);
    }
	return 0;
}
