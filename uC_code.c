#define F_CPU  8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart_functions.h"
//#include "sdcard.h"
//Define the chip select bits for 1  sensor, sram, SD
//also define the two sensor addresses for the I2C bus
//all bits are on PORTD
#define sensor1_cs 2
#define sensor2_add 0x00 
#define sensor3_add 0x00
#define SD_cs 6
//sram cs will be in port c
#define sram_cs 0

//sRam Instruction codes
#define sram_READ 0x03
#define sram_WRITE 0x02
#define sram_RDMR 0x05
#define sram_WRMR 0x01 


#define CMD0 0x00  //go idle cond   init card in spi mode if CS low
#define CMD8 0x08  //send if cond   verify SD card interface operations condition
#define CMD9 0x09  //read card specific data
#define CMD10 0x0A //read card ID info
#define CMD13 0x0D //read card statsus register
#define CMD17 0x11 //read a single block from the card
#define CMD24 0x18 //write a single data blcok to card
#define CMD25 0x19 //write multiple blocks
#define CMD32 0x20 //sets the address of first blcok to be erased
#define CMD33 0x21 //sets the address of the last block to be erased
#define CMD38 0x26 //erase previously selected blocks
#define CMD55 0x37 // 
#define CMD58 0x3A
#define ACMD23 0x17
#define ACMD41 0x29

#define r1_ready_state 0x00
#define r1_idle_state 0x01
#define r1_illegal_command 0x04
uint8_t add_l=100;//Address pointers for the SRAM
uint8_t add_m=100;//Address pointers
uint8_t add_h=1;// only zero are used, any other bits are ignored

volatile uint16_t delay=500;
//init the SD Card


//SPI init will set up SPI 
void spi_init()
{
//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<2)|(1<<1);
DDRD|=(1<<SD_cs);//sets chip select for SD
PORTD |=(1<<SD_cs);//deselect 
DDRC=(1<<sram_cs);//Sets up chip select for sram 
PORTC=(1<<sram_cs);//deselect
SPCR=0;
SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0);  //master mode sets 2x speed
SPSR=(1<<SPI2X);
}//end spi_init()

void sd_write(uint16_t sector,char* buf)
{
  uart_puts("Sector: ");
  uart_putc((uint8_t)sector+48);
  uart_puts("\n");
  uart_puts(buf);

}//end sd_write

void sd_init()
{
  //SPSR|=(1<<SPI2X);		//resets to slower speed
  SPCR=0;
  SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR1); 
  PORTD |=1<<SD_cs;   //set SD select bit high for reset
  //need to send at least 74 consecutive 1's (So i will try 10 bytes)
  uint8_t i=0;
  for(i=0;i<11;i++)
  {
    SPDR=0xFF;//Send all 1's
    while(bit_is_clear(SPSR,SPIF)){} //spin till done
  }
  PORTD &=~(1<<SD_cs);//select SD Card
  SPDR=0x40;
  while(bit_is_clear(SPSR,SPIF)){}
  for(i=0;i<4;i++)
  {
    SPDR=0x00;
    while(bit_is_clear(SPSR,SPIF)){}
  }
  SPDR=0x95;
  while(bit_is_clear(SPSR,SPIF)){}
  //Should be set up, send jumk and read in response, a good response is 0x01
  //_delay_ms(1);
  for(i=0;i<=0xFF;i++)
  {
    SPDR=0xFF;
    while(bit_is_clear(SPSR,SPIF)){}
    uint8_t input=SPDR;
    if(input!=0xFF){uart_puts("SD-Card Init: PASSED");break;}
    if(i==0xFF){uart_puts("SD-Card Init: FAILED");}
  }
 // uart_puts("Bits are set:");
  for(i=0;i<8;i++)
  {
   // if(bit_is_set(input,i)){uart_putc(i+48);}
  }
  spi_init();  //resets spi to faster rate, and deselects SD 
}


void sram_init()
{	//Function will text comms with sram
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
}//end sram_write

int main()
{
uart_init();//Keep this as first init so that text can be sent out in others
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
char rx_char;
sd_init();
char sd_buf[10];
uint8_t i=0;
for(i=0;i<10;i++){sd_buf[i]='0';}
while(1)
{
  sd_write(1,sd_buf);
  uart_puts("sd_write called....");
//  uart_puts("\2 00001,04502,12403,04204,12005,13576,06507,65008,99909,13010,11111,\4");
//  uart_puts("00001,00002,00003,00004,00005,\8\8\8\8,00007,00008,00009,00010,00011,\4");
  //uart_putc(4);
  _delay_ms(1000);
  continue;
  uart_puts("Starting Testing");
  uart_putc('\r');
  rx_char=uart_getc(); 
  if(rx_char=='c')
  {
    rx_char=uart_getc();
    while(rx_char!='c')
    {
      if(rx_char=='r'){PORTB&=~(1<<2);PORTB |=(1<<1);}
      if(rx_char=='g'){PORTB&=~(1<<1);PORTB |=(1<<2);}
      rx_char=uart_getc();
    }
  }
 // uint8_t i=0;
  //Load values into 10 memory locations
  //Zero is only skipped to avoid loading zero into the byte
  //since when the byte is read back, zero could also mean communication failed

  for(i=48;i<=57;i++)
  {
    sram_write(i,add_m,add_h,i);//only lower address byte is incremented 
    //uart_putc(i);
  }//end loading for loop

  //change value in one spot in array as build in "error"
  sram_write(50,add_m,add_h,100);

  //check values in first 10 memory locations, one should be "wrong"
  for(i=48;i<57;i++)
  {
    _delay_ms(100);
    PORTB |=(1<<1)|(1<<2);
    _delay_ms(100);
    if (sram_read(i,add_m,add_h)==i)
    {
      PORTB &=~(1<<1);
      uart_puts("passed");
    }//Byte read back correct  GREEN light
    else{PORTB &=~(1<<2);uart_puts("FAILED!!");}//Byte Read back was incorrect   RED light
  }//end for loop for checking values

  PORTB |=(1<<1)|(1<<2);//both lights off

  uart_puts("****Tesst finished*****\r\r");
  _delay_ms(1500);//wait 1.5 seconds before starting again
} //end while 
} //end main
