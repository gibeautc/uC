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
#include <stdlib.h>
#include <avr/interrupt.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "MPU9250.h"
#include "Feedback.h"
#include "SPI.h"
#include "UART.h"

volatile unsigned long count_t=0;

ISR(TIMER2_OVF_vect)
{
	count_t++;
}

int main(void)
{
	_delay_ms(10);
	int16_t ax=0,ay=0,az=0;
	int16_t gx,gy,gz;
	short data[3];
	char addL[10];
	
	init_Feedback();
	init_SPI();
	uart_init();
	init_tcnt2();
	_delay_ms(10);
	//mpu_init(NULL);
	SPIinit_MPU(sensor1_cs, MPU9250_FULL_SCALE_4G, MPU9250_GYRO_FULL_SCALE_500DPS);
	sei();
	
	//Initializations Successful
	LED(G_LED, Pulse_3);
	
    while (1) 
    {
		if(count_t>20000)
			count_t=0;
		//mpu_get_accel_reg(data, NULL);
		//uart_puts("\nAccel:::");
		SPIgetAccel(data, sensor1_cs);
				
		
		uart_puts("*HX");
		itoa(count_t,addL,10);
		uart_puts(addL);
		uart_puts("Y");
		itoa((int)data[0],addL,10);
		uart_puts(addL);
		uart_puts("*");
		
		uart_puts("*JX");
		itoa(count_t,addL,10);
		uart_puts(addL);
		uart_puts("Y");
		itoa((int)data[1],addL,10);
		uart_puts(addL);
		uart_puts("*");
		
		uart_puts("*KX");
		itoa(count_t,addL,10);
		uart_puts(addL);
		uart_puts("Y");
		itoa((int)data[2],addL,10);
		uart_puts(addL);
		uart_puts("*");
		/*
		itoa((int)data[1],addL,10);
		uart_puts(" Y: ");
		uart_puts(addL);
		
		itoa((int)data[2],addL,10);
		uart_puts(" Z: ");
		uart_puts(addL);
		
		
		_delay_ms(100);
		uart_puts("GYRO:::");
		//mpu_get_accel_reg(data, NULL);
		SPIgetGyro(data, sensor1_cs);
		
		itoa((int)data[0],addL,10);
		uart_puts("X: ");
		uart_puts(addL);
		
		itoa((int)data[1],addL,10);
		uart_puts(" Y: ");
		uart_puts(addL);
		
		itoa((int)data[2],addL,10);
		uart_puts(" Z: ");
		uart_puts(addL);
		
		//MAG
		_delay_ms(100);
		uart_puts("MAG:::");
		//mpu_get_accel_reg(data, NULL);
		SPIgetGyro(data, sensor1_cs);
		
		itoa((int)data[0],addL,10);
		uart_puts("X: ");
		uart_puts(addL);
		
		itoa((int)data[1],addL,10);
		uart_puts(" Y: ");
		uart_puts(addL);
		
		itoa((int)data[2],addL,10);
		uart_puts(" Z: ");
		uart_puts(addL);
		
		uart_puts("\n\n\n\n");
		_delay_ms(1000);
		*/
		
    }
	return 0;
}

