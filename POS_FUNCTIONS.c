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
	while(1)
	{
	 //_delay_ms(1);
	 read_all(sensor1_cs);
	 if(count_t>30000){count_t=0;}
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
	
	uart_puts("*HC**JC**KC*");
	uart_puts("*TLive....*");
	count_t=0;
	struct POS c_pos,p_pos;
	float fXg=0;
	float fYg=0;
	float fZg=0;
	float alpha=0.5f;
	
	while(1)
	{	
		//_delay_ms(1);
		read_all(sensor1_cs);
		c_pos.t=count_t;
		c_pos.a_x=Accel_data[0];
		c_pos.a_y=Accel_data[1];
		c_pos.a_z=Accel_data[2];
		
		//low pass filter
		fXg=c_pos.a_x*alpha+(fXg*(1.0-alpha));
		fYg=c_pos.a_y*alpha+(fYg*(1.0-alpha));
		fZg=c_pos.a_z*alpha+(fZg*(1.0-alpha));
		
		//roll and pitch equations
		p_pos.t=count_t;
		p_pos.a_x=(atan2(-fYg,fZg)*180.0f)/3.14159f;
		p_pos.g_x=(atan2(fXg,sqrt(fYg*fYg+fZg*fZg))*180.0f)/3.14159f;
		
		memset(buf, 0, BUFSZ);
		snprintf(buf, BUFSZ, "*TX:%f Y:%f Z:%f*", fXg,fYg,fZg);
		uart_puts(buf);
		
		rate++;
		print_record(p_pos);
	}
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*S**TDone:%d\n*", rate);
	uart_puts(buf);
}

void read_back(int c)
{
	uart_puts("*Treading back*");
	uart_puts("*HC**JC**KC*");
	mem_reset();
	for(int i=0;i<=c;i++)
	{
		print_record(read_mem_pos());
	}
}

int save_window()
{
	mem_reset();
	uart_puts("*T\nStarting Window record\n*");
	int c=0;
	struct POS c_pos;
	count_t=0;
	while(count_t<30000)//needs to be 30 seconds
	{
		_delay_ms(20.5);
		if(count_t%100==0){uart_puts("*T.*");}
		read_all(sensor1_cs);
		c_pos.t=count_t;
		c_pos.a_x=Accel_data[0];
		c_pos.a_y=Accel_data[1];
		c_pos.a_z=Accel_data[2];
		
		c_pos.g_x=Gyro_data[0];
		c_pos.g_y=Gyro_data[1];
		c_pos.g_z=Gyro_data[2];

		c_pos.m_x=Mag_data[0];
		c_pos.m_y=Mag_data[1];
		c_pos.m_z=Mag_data[2];
		store_pos(c_pos);
		/*
		read_all(sensor1_cs);
		c_pos.t=count_t;
		c_pos.a_x=Accel_data[0];
		c_pos.a_y=Accel_data[1];
		c_pos.a_z=Accel_data[2];
		
		c_pos.g_x=Gyro_data[0];
		c_pos.g_y=Gyro_data[1];
		c_pos.g_z=Gyro_data[2];

		c_pos.m_x=Mag_data[0];
		c_pos.m_y=Mag_data[1];
		c_pos.m_z=Mag_data[2];
		store_pos(c_pos);
		
		read_all(sensor1_cs);
		c_pos.t=count_t;
		c_pos.a_x=Accel_data[0];
		c_pos.a_y=Accel_data[1];
		c_pos.a_z=Accel_data[2];
		
		c_pos.g_x=Gyro_data[0];
		c_pos.g_y=Gyro_data[1];
		c_pos.g_z=Gyro_data[2];

		c_pos.m_x=Mag_data[0];
		c_pos.m_y=Mag_data[1];
		c_pos.m_z=Mag_data[2];
		store_pos(c_pos);
		*/
		c++;
		
	}
	uart_puts("*T\nEnd window\n*");
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*TCount is: %d*", c);
	uart_puts(buf);
	return c;
}

void store_pos(struct POS pos)
{
	//time is 4
	sram_write(add_l,add_m,add_h,(int8_t)(pos.t>>24));
	sram_write(add_l,add_m,add_h,(int8_t)(pos.t>>16));
	sram_write(add_l,add_m,add_h,(int8_t)(pos.t>>8));
	sram_write(add_l,add_m,add_h,(int8_t)(pos.t));
	
	//Acceleration
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_x>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_x>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_x>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_x));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_y>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_y>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_y>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_y));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_z>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_z>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_z>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.a_z));
	
	//Gyro
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_x>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_x>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_x>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_x));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_y>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_y>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_y>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_y));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_z>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_z>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_z>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.g_z));
	
	//Mag
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_x>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_x>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_x>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_x));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_y>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_y>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_y>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_y));
	
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_z>>24));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_z>>16));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_z>>8));
	sram_write(add_l,add_m,add_h,(int8_t)((unsigned long)pos.m_z));
	
}

struct POS read_mem_pos()
{
	struct POS pos;
	unsigned long temp_byte[4];
	unsigned long temp_long;
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.t=temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.a_x=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.a_y=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.a_z=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.g_x=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.g_y=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.g_z=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.m_x=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.m_y=(float)temp_long;
		//***********************************************
		
		for(int i=0;i<4;i++)//read next for bytes
		{
			temp_byte[i]=sram_read(add_l,add_m,add_h);
			temp_byte[i]=temp_byte[i]<<(8*(3-i));
		}
		temp_long=temp_byte[0]+temp_byte[1]+temp_byte[2]+temp_byte[3];
		pos.m_z=(float)temp_long;
		//***********************************************
	return pos;
}

void mem_reset()
{
	add_m=0;
	add_h=0;
	add_l=1;
}

void test_mem()
{
	
	struct POS pos;
	pos.t=1;
	pos.a_x=1.1;
	pos.a_x=1.2;
	pos.a_x=1.3;
	
	pos.g_x=2.1;
	pos.g_x=2.2;
	pos.g_x=2.3;
	
	pos.m_x=3.1;
	pos.m_x=3.2;
	pos.m_x=3.3;
	mem_reset();
	store_pos(pos);
	
	_delay_ms(2000);
	mem_reset();
	struct POS new=read_mem_pos();
	uart_puts("*TStarting Mem test*");
	//if(new.t==1){uart_puts("*Ttime was good*");}
	//else{uart_puts("*Ttime was BAD*");}
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	snprintf(buf, BUFSZ, "*Ttime is: %lu*", new.t);
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