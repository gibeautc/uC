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
	short data[3];
	uint8_t response[3];
	char buf[BUFSZ];
	
	init_Feedback();
	init_SPI();
	uart_init();
	init_tcnt2();
	_delay_ms(10);
	//mpu_init(NULL);
	SPIinit_MPU(sensor1_cs, BITS_FS_4G, BITS_FS_500DPS);
	sei();
	
	//Initializations Successful
	LED(G_LED, Pulse_3);
	response[0] = whoami(sensor1_cs);
	response[1] = AK8963_whoami(sensor1_cs);
	
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "\nWhoAmI Value: %d\n\n", response[0]);
	uart_puts(buf);
	
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*TMPU=%d  AK=%d*", response[0], response[1]);
	uart_puts(buf);
	
	if(response[0] == 0)
	{
		snprintf(buf, BUFSZ, "*BR255G0B0*");
		uart_puts(buf);
	}
	else if (response[0] == 113)
	{
		snprintf(buf, BUFSZ, "*BR0G255B0*");
		uart_puts(buf);
	}
	else
	{
		snprintf(buf, BUFSZ, "*BR0G0B255*");
		uart_puts(buf);
	}
	
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "\nMAG WhoAmI Value: %d\n\n", response[1]);
	uart_puts(buf);
	if(response[1] == 0)
	{
		snprintf(buf, BUFSZ, "*LR255G0B0*");
		uart_puts(buf);
	}
	else if (response[1] == 72)
	{
		snprintf(buf, BUFSZ, "*LR0G255B0*");
		uart_puts(buf);
	}
	else
	{
		snprintf(buf, BUFSZ, "*LR0G0B255*");
		uart_puts(buf);
	}
	_delay_ms(5000);
	
    while (1) 
    {
		
		if(count_t>20000)
			count_t=0;
			
		//readDatShit(sensor1_cs);
		//mpu_get_accel_reg(data, NULL);
		//uart_puts("\nAccel:::");
		SPIgetAccel(data, sensor1_cs);
		ax = Accel_data[0];//*MPU9250A_16g_scale;
		ay = Accel_data[1];//*MPU9250A_16g_scale;
		az = Accel_data[2];//*MPU9250A_16g_scale;
		
		SPIgetGyro(data, sensor1_cs);
		gx = Gyro_data[0]*MPU9250G_500dps_scale;
		gy = Gyro_data[1]*MPU9250G_500dps_scale;
		gz = Gyro_data[2]*MPU9250G_500dps_scale;
		
		SPIgetMag(data, sensor1_cs);
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
		
		
    }
	return 0;
}

