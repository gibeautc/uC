/*
 * SPI.c
 *
 * Created: 4/16/2016 8:07:34 PM
 *  Author: gbone
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include "SPI.h"
#include "UART.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>

#define readbit 0x80
#define sram_READ 0x03
#define sram_WRITE 0x02
#define sram_RDMR 0x05
#define sram_WRMR 0x01


#define readbit 0x80

void init_SPI(void)
{
	//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
	DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<sensor1_cs)|(1<<1)|(1<<sensor2_cs)|(1<<sensor3_cs);
	deselect(sensor1_cs);
	deselect(sensor2_cs);
	deselect(sensor3_cs);
	DDRC |=(1<<sram_cs);//Sets up chip select for sram
	PORTC|=(1<<sram_cs);//deselect
	SPCR=0;
	SPCR=(1<<SPE)|(1<<MSTR)|(0<<SPR1)|(1<<SPR0)|(0<<DORD)|(1<<CPOL)|(1<<CPHA);  //master mode sets 2x speed
	//(1<<SPR0);
	//SPSR=(1<<SPI2X);
	SPSR=0;
}//end spi_init()

char SPI_send(char chr)
{
	char receivedchar=0;
	SPDR=chr;
	while(bit_is_clear(SPSR,SPIF)){}
	receivedchar=SPDR;
	return (receivedchar);
}//End SPI_send

void select(uint8_t slvdevice){
	switch (slvdevice)
	{
	case sensor1_cs:
		PinLOW(PORTB, sensor1_cs);
		break;
	case sensor2_cs:
		PinLOW(PORTB, sensor2_cs);
		break;
	case sensor3_cs:
		PinLOW(PORTB, sensor3_cs);
		break;
	case sram_cs:
		PinLOW(PORTC, sram_cs);
		break;
	}
	return;
}

void deselect(uint8_t slvdevice){
{
		switch (slvdevice)
		{
			case sensor1_cs:
			PinHIGH(PORTB, sensor1_cs);
			break;
			case sensor2_cs:
			PinHIGH(PORTB, sensor2_cs);
			break;
			case sensor3_cs:
			PinHIGH(PORTB, sensor3_cs);
			break;
			case sram_cs:
			PinHIGH(PORTC, sram_cs);
			break;
		}
		return;
	}
}

int spi_writeRegs(unsigned char sel, unsigned char reg_addr,
		unsigned char length, unsigned char const *data) {
	uint8_t i;
	select(sel);
	
	SPDR=reg_addr;
	while(bit_is_clear(SPSR,SPIF)){}
	for(i=0; i<length; i++)
	{
		SPDR=data[i];
		while(bit_is_clear(SPSR,SPIF)){};
		SPDR=0xFF;
	}
	deselect(sel);
	_delay_us(10);
	return 0;
}

unsigned int spi_writeReg(unsigned char sel, unsigned char reg_addr, unsigned char data){
	unsigned int temp_val;
	select(sel);
	transfer(reg_addr);
	temp_val=transfer(data);
	deselect(sel);
	_delay_us(50);
	return temp_val;
}

void spi_readRegs(unsigned char sel, unsigned char reg_addr,
		unsigned char length, unsigned char *data) {
	unsigned int  i = 0;
	
	select(sel);
	transfer(reg_addr | readbit);
	for(i=0; i<length; i++) data[i] = transfer(0x00);
	deselect(sel);
	_delay_us(50);
}


//SRAM setup, should return 1 if success and 0 if failure
uint8_t sram_init()
{
	add_l=0;
	add_m=0;
	add_h=0;
	
	select(sram_cs);
	transfer(sram_WRMR);
	transfer(0x00);
	unsigned char test=transfer(sram_RDMR);
	deselect(sram_cs);
	if(test==0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}


void add_inc()
{
	if(add_l==255 && add_m==255 && add_h==1){uart_puts("*TMemory Over Flow*");while(1){}}//add in user feed back
	
	if(add_l==255)
	{
		add_l=0;
		if(add_m==255)
		{
			add_m=0;
			add_h=1;
		}
		else{add_m++;}
	}
	else{add_l++;}
		//char buf[20];
		//memset(buf, 0, 20);
		//snprintf(buf, 20, "*TL:%d*", add_l);
		//uart_puts(buf);

}//end add_inc


uint8_t sram_read(uint8_t low,uint8_t mid, uint8_t high)
{
	select(sram_cs);
	transfer(sram_READ);
	transfer(high);
	transfer(mid);
	transfer(low);
	uint8_t temp=transfer(0xFF);
	deselect(sram_cs);
	add_inc();
	return temp;
}

void sram_write(uint8_t low,uint8_t mid, uint8_t high, uint8_t data)
{
	select(sram_cs);
	transfer(sram_WRITE);
	transfer(high);
	transfer(mid);
	transfer(low);
	transfer(data);
	deselect(sram_cs);
	add_inc();
}



