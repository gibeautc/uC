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
#include "MPU9250.h"
#include "Feedback.h"
#include "SPI.h"
#include "UART.h"

int main(void)
{
	int16_t ax=0,ay=0,az=0;
	int16_t gx,gy,gz;
	char addL[10];
	
	init_Feedback();
	init_SPI();
	uart_init();
	
	select(sensor1_cs);
	SPIinit_MPU(MPU9250_FULL_SCALE_4G, MPU9250_GYRO_FULL_SCALE_500DPS);
	deselect(sensor1_cs);
	
	select(sensor2_cs);
	SPIinit_MPU(MPU9250_FULL_SCALE_4G, MPU9250_GYRO_FULL_SCALE_500DPS);
	deselect(sensor2_cs);
	
	//Initializations Successful
	LED(G_LED, Pulse_3);
	
    while (1) 
    {
		
		//Vibrate(Pulse_3);
		
		//_delay_ms(200);
		
		/*
		 * SENSOR 1
		 */			
		select(sensor1_cs);
		SPIgetAccel(&ax,&ay,&az);
		SPIgetGyro(&gx, &gy, &gz);
		deselect(sensor1_cs);
		/*
		 * Accelerometer
		 */
		itoa(ax,addL,10);
		uart_puts("X axis: ");
		uart_puts(addL);
		
		itoa(ay,addL,10);
		uart_puts("Y axis: ");
		uart_puts(addL);
		
		itoa(az,addL,10);
		uart_puts("Z axis: ");
		uart_puts(addL);
				
		/*
		 * Gyro
		 */
		itoa(gx,addL,10);
		uart_puts("X axis: ");
		uart_puts(addL);
		
		itoa(gy,addL,10);
		uart_puts("Y axis: ");
		uart_puts(addL);
		
		itoa(gz,addL,10);
		uart_puts("Z axis: ");
		uart_puts(addL);
		
		/*
		 * SENSOR 2 
		 */	
		select(sensor2_cs);
		SPIgetAccel(&ax,&ay,&az);
		SPIgetGyro(&gx, &gy, &gz);
		deselect(sensor2_cs);
				
		/*
		 * Accelerometer
		 */
		itoa(ax,addL,10);
		uart_puts("X axis: ");
		uart_puts(addL);
		
		itoa(ay,addL,10);
		uart_puts("Y axis: ");
		uart_puts(addL);
		
		itoa(az,addL,10);
		uart_puts("Z axis: ");
		uart_puts(addL);
				
		/*
		 * Gyro
		 */
		itoa(gx,addL,10);
		uart_puts("X axis: ");
		uart_puts(addL);
		
		itoa(gy,addL,10);
		uart_puts("Y axis: ");
		uart_puts(addL);
		
		itoa(gz,addL,10);
		uart_puts("Z axis: ");
		uart_puts(addL);
		
		
    }
	return 0;
}

