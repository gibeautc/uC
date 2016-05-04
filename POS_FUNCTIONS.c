#include "POS_FUNCTIONS.h"
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

void record_window()
{
	char buf[BUFSZ];
	int rate=0;
	uart_puts("*HC**JC**KC*");
	uart_puts("*TRecording....*");
	count_t=0;
	struct POS temp_pos;
	while(count_t<20000)
	{
	 //_delay_ms(1);
	 read_all(sensor3_cs);
	 
	 temp_pos.t=count_t;
	 temp_pos.a_x=Accel_data[0];
	 temp_pos.a_y=Accel_data[1];
	 temp_pos.a_z=Accel_data[2];
	 
	temp_pos.g_x=Gyro_data[0];
	temp_pos.g_y=Gyro_data[1];
	temp_pos.g_z=Gyro_data[2];
	
	temp_pos.m_x=Mag_data[0];
	temp_pos.m_y=Mag_data[1];
	temp_pos.m_z=Mag_data[2];
	 rate++;
	 print_record(temp_pos);
	}
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*S**TDone:%d\n*", rate);
	uart_puts(buf);
}


void live_pos()
{
	//float x_pos,y_pos,z_pos,x_vel,y_vel,z_vel;
	char buf[BUFSZ];
	int rate=0;
	int dt=0;
	
	uart_puts("*HC**JC**KC*");
	uart_puts("*TRecording....*");
	count_t=0;
	struct POS l_pos,c_pos,p_pos;
	while(count_t<20000)
	{	
		//_delay_ms(1);
		read_all(sensor1_cs);
		c_pos.t=count_t;
		c_pos.a_x=Accel_data[0];
		c_pos.a_y=Accel_data[1];
		c_pos.a_z=Accel_data[2];
		p_pos.a_x=Accel_data[0];
		p_pos.a_y=Accel_data[1];
		p_pos.a_z=Accel_data[2];
		if(rate>0)//wait till the second record before looking for changes
		{
			dt=c_pos.t-l_pos.t; //calculate the time difference
			p_pos.g_x=p_pos.g_x+(c_pos.a_x*(float)dt*0.001f);
			p_pos.g_y=p_pos.g_y+(c_pos.a_y*(float)dt*0.001f);
			p_pos.g_z=p_pos.g_z+(c_pos.a_z*(float)dt*0.001f);
			
			p_pos.m_x=p_pos.m_x+(c_pos.a_x*(float)dt*0.001f);
			p_pos.m_y=p_pos.m_y+(c_pos.a_y*(float)dt*0.001f);
			p_pos.m_z=p_pos.m_z+(c_pos.a_z*(float)dt*0.001f);
			
		}
		
		rate++;
		print_record(p_pos);
		l_pos.t=c_pos.t;
		l_pos.a_x=c_pos.a_x;
		l_pos.a_y=c_pos.a_y;
		l_pos.a_z=c_pos.a_z;
	}
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*S**TDone:%d\n*", rate);
	uart_puts(buf);
}


void print_record(struct POS pos)
{
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*HX%luY%f,X%luY%f,X%luY%f*\n", pos.t, pos.a_x, pos.t, pos.a_y, pos.t, pos.a_z);
	uart_puts(buf);
	
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*JX%luY%f,X%luY%f,X%luY%f*\n",pos.t, pos.g_x, pos.t, pos.g_y, pos.t, pos.g_z);
	uart_puts(buf);
	
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*KX%luY%f,X%luY%f,X%luY%f*\n",pos.t, pos.m_x, pos.t, pos.m_y, pos.t, pos.m_z);
	uart_puts(buf);
	//_delay_ms(30);
	
}