#define F_CPU  8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart_functions.h"
#include "i2c_master.h"
#include "MPU9250.h"
#include "twi_master.h"
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
#define CRC   35   //this is the CRC char with a 1 shifted in for SD Card 


#define r1_ready_state 0x00
#define r1_idle_state 0x01
#define r1_illegal_command 0x04
uint8_t add_l=0;//Address pointers for the SRAM
uint8_t add_m=0;//Address pointers
uint8_t add_h=0;// only zero are used, any other bits are ignored
char sd_buf[512];
volatile uint16_t delay=500;
uint8_t sd_add[4];
uint8_t arrayI=0;

void i2c_test()
{
  //i2c_init();
  //uint8_t data[5]={1,2,3,4,5};
  //uint8_t x=0;
  uart_puts("Starting I2C test\n\n");
  DDRC|=(1<<5);

  PORTC&=~(1<<5);
  _delay_ms(1000);
  PORTC|=(1<<5);
  //i2c_start(100);
  //for(x=0;x<100;x++)
  //{
   // uart_putc('.'); 
    //i2c_write(x);
    //_delay_ms(100);
    //while(twi_busy()){}
  //}
  //i2c_stop();
  uart_puts("\nI2C Test complete\n");
}


//SPI init will set up SPI 
void spi_init()
{
//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<2)|(1<<1);
DDRD|=(1<<SD_cs)|(1<<sensor1_cs);//sets chip select for SD
PORTD |=(1<<SD_cs)|(1<<sensor1_cs);//deselect 
DDRC |=(1<<sram_cs);//Sets up chip select for sram 
PORTC|=(1<<sram_cs);//deselect
SPCR=0;
SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0);  //master mode sets 2x speed
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

}


char sd_cmd(char cmd,uint16_t ArgH,uint16_t ArgL,char crc)
{
  SPCR=0;
  SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0);
  PORTD &=~(1<<SD_cs);
  SPI_send(0xFF);
  SPI_send(cmd|0x40);//cmd is less then 64 but needs to start with 01
  SPI_send((uint8_t)(ArgH>>8));
  SPI_send((uint8_t)ArgH);
  SPI_send((uint8_t)(ArgL>>8));
  SPI_send((uint8_t)ArgL);
  SPI_send(crc);
  //SPI_send(0xFF);
  //SPI_send(0xFF);
  //SPI_send(0xFF);
  for(arrayI=0;arrayI<10;arrayI++)
  {
   uart_putc(SPI_send(0xFF)+48);
  }
  spi_init();
  return(SPDR);
}//end sd_write


void sd_read(uint16_t sector)
{

}

void sd_init()
{
    

  //SPSR|=(1<<SPI2X);		//resets to slower speed
  SPCR=0;
  SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0); 
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
    SPDR=0x40;//*********CHANGE BACK TO ZERO*************
    while(bit_is_clear(SPSR,SPIF)){}
  }
  SPDR=0x95;
  while(bit_is_clear(SPSR,SPIF)){}
  //Should be set up, send jumk and read in response, a good response is 0x01
  //_delay_ms(1);
  uint8_t input=0;
  for(i=0;i<=150;i++)
  {
    SPDR=0xFF;
    while(bit_is_clear(SPSR,SPIF)){}
    input=SPDR;
    if(input==0x01){uart_puts("SD-Card Init: PASSED\n");break;}
    if(i==100){uart_puts("SD-Card Init: FAILED\n");break;}
  }
 // uart_puts("Bits are set:");
  //for(i=0;i<8;i++)
  //{
   // if(bit_is_set(input,i)){uart_putc(i+48);}
  //}
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
void sram_write(uint8_t low,uint8_t mid, uint8_t high,int8_t data){
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

void init_tcnt2()
{
 ASSR |=(0<<AS2);//Run of 32khz osc
 TIMSK2=0x00;  //reset TIMSK
 TIMSK2 |=(1<<TOIE2);//turns on comp match interupt
 TCCR2A=0;
 TCCR2B=(1<<CS21)|(1<<CS20);//Normal mode prescale 32 should give a 1ms count
 
}


volatile long int count_t=0;


//records on shot in sram (20seconds) returns the number of records
//need to add error checking on high byte of address (only 0 and 1)
void add_inc()
{
  add_l++;
  if(add_l>=255)
  {
    add_l=0;
    add_m++;
    if(add_m>=255)
    {
      add_m=0;
      add_h=1;
    }
  }
 
}//end add_inc
 
uint16_t record_shot()
{
  uint16_t num_records=0;
  int16_t ax=0,ay=0,az=0;
  //int16_t gx,gy,gz;
  //int16_t mx,my,mz;
  add_l=0;
  add_m=0;//reset addresses to zero
  add_h=0;
  uart_puts("Shot Started...\n");
  //get 20 seconds of data
  //each sensor is 14 bytes of data
  //each byte is roughtly 2.5 ms 
  uint16_t i=0;
  int8_t x=0;
  //uint8_t data_count=0;
  uint8_t data[20];
  char addL[10];
  //char addM[5];
  //char addH[5];
  count_t=0;
  while(count_t<10000)//for full shot change back to 20000************
  {
//    _delay_ms(1000);
    getAccel(&ax,&ay,&az, MPU9250_DEFAULT_ADDRESS);//fetch all axis compass readings

    //data[0]=(int8_t)(ax>>8); 
    //data[1]=(int8_t)ax;
    //data[2]=(int8_t)(ay>>8);
    //data[3]=(int8_t)ay;
    //data[4]=(int8_t)(az>>8);
    //data[5]=(int8_t)az;
    uart_puts("X-accel: ");
    itoa(ax,addL,10);
    uart_puts(addL);
    uart_putc('\t');
    uart_puts(" ");
    uart_puts("Y-accel: ");
    itoa(ay,addL,10);
    uart_puts(addL);
    uart_putc('\t');
    uart_puts("Z-accel: ");
    itoa(az,addL,10);
    uart_puts(addL);
    uart_putc('\n');
  
  
    getAccel(&ax,&ay,&az, MPU9250_ALT_DEFAULT_ADDRESS);//fetch all axis compass readings
/*    data[0]=(int8_t)(ax>>8); 
    data[1]=(int8_t)ax;
    data[2]=(int8_t)(ay>>8);
    data[3]=(int8_t)ay;
    data[4]=(int8_t)(az>>8);
    data[5]=(int8_t)az;*/
    uart_puts("X2-accel: ");
    itoa(ax,addL,10);
    uart_puts(addL);
    uart_putc('\t');
    uart_puts("Y2-accel: ");
    itoa(ay,addL,10);
    uart_puts(addL);
    uart_putc('\t');
    uart_puts("Z2-accel: ");
    itoa(az,addL,10);
    uart_puts(addL);
    uart_putc('\n');
    uart_putc('\n');

  } 
//  PORTD|=(1<<sensor1_cs);
}//end record_shot


ISR(TIMER2_OVF_vect)
{
count_t++;
}//end Timer 0 ISR


//Used to catch unhandled interupts 
ISR(BADISR_vect)
{
  uart_puts("Unhandled Interupt caught!!!!");
}//end Bad ISR


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
//sram_init();//initialize sram
char rx_char;
//sd_init();  //initialize SD card
init_tcnt2();//set up timer (RTC)
init_twi(); //initialize TWI interface
sei();
init_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS, MPU9250_ALT_DEFAULT_ADDRESS); //initialize the 9axis sensor
init_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS, MPU9250_DEFAULT_ADDRESS); //initialize the 9axis sensor
//while(twi_busy()){};
PORTD &=~(1<<sensor1_cs);
//uint8_t response[1];
//SPIinit_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS); //initialize the 9axis sensor on SPI
//SPIreadReg(0x75, response);

PORTD |=(1<<sensor1_cs);

uint16_t i=0;//used for for loops
//*****Fix me*******  
//should set to int 0 not char '0' but the ascii zero prints better for now
for(i=0;i<512;i++){sd_buf[i]='0';}//sets inital buffer to zero values

PORTB &=~(1<<2);
_delay_ms(2000);
PORTB |=(1<<2);
PORTB &=~(1<<1);
//char time_buf[10];
while(1)
{
uint16_t shots=record_shot();
char shots_s[10];
itoa(shots,shots_s,10);
uart_puts("In 20 seconds The number of shots was: ");
uart_puts(shots_s);
uart_putc('\n');
while(1){}
  //sd_write(1,sd_buf);
  //uart_puts("sd_write called....");
//  uart_puts("\2 00001,04502,12403,04204,12005,13576,06507,65008,99909,13010,11111,\4");
//  uart_puts("00001,00002,00003,00004,00005,\8\8\8\8,00007,00008,00009,00010,00011,\4");
  //uart_putc(4);
  //_delay_ms(1000);
  //continue;
  uart_puts("Starting Testing\n\n");
  uart_putc('\r');
  rx_char=uart_getc(); 
  if(rx_char=='c')
  {
    uart_puts("Command line:\n\n");
    rx_char=uart_getc();
    while(rx_char!='c')
    {
      if(rx_char=='s'){sd_init();}
     if(rx_char=='w')
     {
       
       //for(arrayI=0;arrayI<0xFF;arrayI++)
       //{
       sd_cmd(0x10,0,512,CRC);  //sets block size
       //}
     }
      if(rx_char=='r'){sd_read(1);uart_puts(sd_buf);}//buffer gets set to sector!!
      if(rx_char=='i'){i2c_test();}
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
      uart_puts("passed\n");
    }//Byte read back correct  GREEN light
    else{PORTB &=~(1<<2);uart_puts("FAILED!!\n");}//Byte Read back was incorrect   RED light
  }//end for loop for checking values

  PORTB |=(1<<1)|(1<<2);//both lights off

  uart_puts("****Tesst finished*****\r\r");
  _delay_ms(1500);//wait 1.5 seconds before starting again
} //end while 
} //end main
