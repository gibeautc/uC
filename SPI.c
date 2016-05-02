/*
 * SPI.c
 *
 * Created: 4/16/2016 8:07:34 PM
 *  Author: gbone
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include "SPI.h"

#define readbit 0x80

void init_SPI(void)
{
	//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
	DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<sensor1_cs)|(1<<1)|(1<<sensor2_cs);
	deselect(sensor1_cs);
	deselect(sensor2_cs);
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

/*
 * @ brief, spi write function for Invensense IC using MSP430
 * @ parameters
 * @ sel = this is the handle, used to be
 *   just for compatibility with I2C and can be used ofr
 *   chip select if needed.
 * @ reg_addr, the register address of the chip
 * @ length, length of the data to be written, assuming
 *   auto increment of the register address by the chip
 *   parameter data. ;
 * @ data, data pointer where the data is written to reg addr
 *   and sequentially hence forth if greater than 1
 * @ return, success or failure
 */
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
	
		
/*
 * @ brief, spi read function for Invensense IC using MSP430
 * @ parameters
 * @ sel = this is the handle, used to be
 *   just for compatibility with I2C and can be used ofr
 *   chip select if needed.
 * @ reg_addr, the register address of the chip
 * @ length, length of the data to be read, assuming
 *   auto increment of the register address by the chip
 *   parameter data. ;
 * @ data, data pointer where the data is read from reg addr
 *   and sequentially hence forth if greater than 1
 * @ return, success or failure
 */
void spi_readRegs(unsigned char sel, unsigned char reg_addr,
		unsigned char length, unsigned char *data) {
	unsigned int  i = 0;
	
	select(sel);
	transfer(reg_addr | readbit);
	for(i=0; i<length; i++) data[i] = transfer(0x00);
	deselect(sel);
	_delay_us(50);
}

