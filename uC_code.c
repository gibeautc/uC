#define F_CPU  8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//SPI init will set up SPI 
void spi_init()
{
//DDRB need to be set correctly
//Set at least 4 Chip select pins  3 sensors plus SRAM
SPCR=(1<<SPE)|(1<<MSTR);  //master mode
SPSR=(1<<SPI2X);          //sets 2x speed
}//end spi_init()


int main()
{
 

spi_init();

while(1)
{ 
 } //end while 
} //end main
