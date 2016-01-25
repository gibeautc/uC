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
uint8_t add_l=100;//Address pointers for the SRAM
uint8_t add_m=100;//Address pointers
uint8_t add_h=1;// only zero are used, any other bits are ignored

volatile uint16_t delay=500;

//SPI init will set up SPI 
void spi_init()
{
//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<2)|(1<<1);
DDRC=(1<<sram_cs);//Sets up chip select for sram 
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
//_delay_ms(0.01);
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
PORTC &=~(1<<sram_cs);//select sram chip on SPI bus
SPDR=sram_READ; //Send the read command to sram
while(bit_is_clear(SPSR,SPIF)){}//This line shows up a lot in this code, it waits for the byte to be sent
SPDR=high;//High address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=mid;//Mid address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=low;//Low address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=0xFF; //Junk
while(bit_is_clear(SPSR,SPIF)){}
PORTC |=(1<<sram_cs);//deactivate sram chip
return SPDR;//return the byte received

}//end sram_read()

//Will write byte to sram
void sram_write(uint8_t low,uint8_t mid, uint8_t high,uint8_t data){
PORTC &=~(1<<sram_cs);//select sram chip on SPI bus
SPDR=sram_WRITE;//send write command
while(bit_is_clear(SPSR,SPIF)){}
SPDR=high;//Send high address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=mid;//mid address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=low;//low address byte
while(bit_is_clear(SPSR,SPIF)){}
SPDR=data;//send data byte
while(bit_is_clear(SPSR,SPIF)){}
PORTC |=(1<<sram_cs);//deselect sram chip
}

int main()
{
spi_init(); //initialize SPI bus as master


PORTB |=(1<<1)|(1<<2);
PORTB &=~(1<<2);
_delay_ms(200);
PORTB |=(1<<2);
PORTB &=~(1<<1);// blinks both lights to show the program is starting
_delay_ms(200);
PORTB |=(1<<1);
_delay_ms(1000);
sram_init();//initialize sram
while(1)
{
uint8_t i=0;

//Load values into 10 memory locations
//Zero is only skipped to avoid loading zero into the byte
//since when the byte is read back, zero could also mean communication failed

for(i=1;i<11;i++)
{
  sram_write(i,add_m,add_h,i);//only lower address byte is incremented 
}//end loading for loop

//change value in one spot in array as build in "error"
sram_write(5,add_m,add_h,100);

//check values in first 10 memory locations, one should be "wrong"
for(i=1;i<11;i++)
{
  _delay_ms(100);
  PORTB |=(1<<1)|(1<<2);
  _delay_ms(100);
  if (sram_read(i,add_m,add_h)==i){PORTB &=~(1<<1);}//Byte read back was correct  GREEN light
  else{PORTB &=~(1<<2);}//Byte Read back was incorrect   RED light
}//end for loop for checking values

PORTB |=(1<<1)|(1<<2);//both lights off
_delay_ms(1500);//wait 1.5 seconds before starting again
} //end while 
} //end main
