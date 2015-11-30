#define F_CPU  8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Define the chip select bits for 3 sensors, sram, SD
//all bits are on PORTD
#define sensor1_cs 2
#define sensor2_cs 3
#define sensor3_cs 4
#define SD_cs 6
//sram cs will be in port c
#define sram_cs 0

//sRam Instruction codes
#define sram_READ 0x03
#define sram_WRITE 0x02
#define sram_RDMR 0x05
#define sram_WRMR 0x01 
uint8_t add_l=100;
uint8_t add_m=100;//Address pointers
uint8_t add_h=100;

volatile uint16_t delay=500;

//SPI init will set up SPI 
void spi_init()
{
//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<2)|(1<<1);
//PORTB|=(1<<4);
//PORT D holds all chip select pins. Set to output
//DDRD=(1<<sensor1_cs)|(1<<sensor2_cs)|(1<<sensor3_cs);
DDRC=(1<<sram_cs);
PORTC=(1<<sram_cs);
SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0);  //master mode sets 2x speed
SPSR=(1<<SPI2X);
}//end spi_init()


void sram_init()
{//Function will text comms with sram
//the actuall mode we want is 0x00
//since no data recived might look the same,  set mode to 1
//look at it, set it to 0, look again.  
PORTC &=(0<<sram_cs);
SPDR=sram_WRMR;//write mode command
while(bit_is_clear(SPSR,SPIF)){}//spin till done
SPDR=0x00;//write new mode
while(bit_is_clear(SPSR,SPIF)){}//spin till done
_delay_ms(0.01);
SPDR=sram_RDMR;//send read command
while(bit_is_clear(SPSR,SPIF)){}
SPDR=0xFF; //Send junk
while(bit_is_clear(SPSR,SPIF)){}
//if(SPDR==128){PORTB &=(0<<1);}//got correct byte
//PORTB &=(0<<1);
 
PORTC |=(1<<sram_cs);
}//end sram_init

void check_sensor(){}//end check_sensor
//Writes byte(s) to SD card
//possible arguments address, number of bytes, data
void SD_write(){}//end sram_write()


//Read byte(s) from sram
//arguments address, number of bytes
uint8_t sram_read(uint8_t low,uint8_t mid, uint8_t high){
PORTC &=(0<<sram_cs);
SPDR=sram_READ;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=low;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=mid;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=high;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=0xFF; //Junk
while(bit_is_clear(SPSR,SPIF)){}
PORTC |=(1<<sram_cs);
return SPDR;

}//end sram_read()

//Will write byte to sram
void sram_write(uint8_t low,uint8_t mid, uint8_t high,uint8_t data){
PORTC &=(0<<sram_cs);
SPDR=sram_WRITE;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=high;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=mid;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=low;
while(bit_is_clear(SPSR,SPIF)){}
SPDR=data;
while(bit_is_clear(SPSR,SPIF)){}
PORTC |=(1<<sram_cs);
}

int main()
{
spi_init(); 
PORTB &=(0<<1);
_delay_ms(200);
PORTB |=(1<<1);
sram_init();
while(1)
{
//_delay_ms(200);
//PORTB|=(1<<1);
//sram_init();
//_delay_ms(200);
//PORTB |=(1<<1);
uint8_t i=0;
for(i=0;i<100;i++)
{
sram_write(i,add_m,add_h,i);
}
for(i=0;i<100;i++)
{
if (sram_read(i,add_m,add_h)==i)
{
PORTB ^=~(0<<1);
_delay_ms(200);
//PORTB |=(1<<1);
}//if
}

} //end while 
} //end main
