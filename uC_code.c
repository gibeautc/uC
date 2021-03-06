#define F_CPU  8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart_functions.h"
#include "MPU9250.h"
#include "twi_master.h"
//Define the chip select bits for 1  sensor, sram, SD
//also define the two sensor addresses for the I2C bus
//all bits are on PORTB
#define sensor1_cs 2
//sram cs will be in port c
#define sram_cs 0

//sRam Instruction codes
#define sram_READ 0x03
#define sram_WRITE 0x02
#define sram_RDMR 0x05
#define sram_WRMR 0x01 

uint8_t add_l=0;//Address pointers for the SRAM
uint8_t add_m=0;//Address pointers
uint8_t add_h=0;// only zero are used, any other bits are ignored
char sd_buf[512];
volatile uint16_t delay=500;
uint8_t sd_add[4];
uint8_t arrayI=0;
uint32_t data_count=0;
volatile long int count_t=0;
uint16_t num_records=0;


void vibrate(const uint32_t vib_time)
{ //Vibrate for 100ms
  uint16_t vib_delay=0;
  PORTD|=(1<<3);
  for(vib_delay=0;vib_delay<vib_time;vib_delay++)
  {
    _delay_ms(1);
  }
  PORTD&=~(1<<3);  
}



//SPI init will set up SPI 
void spi_init()
{
//DDRB houses SPI pins SCK-5 MOSI-3 MISO-4 used for programing
DDRB|=(1<<5)|(1<<3)|(0<<4)|(1<<2)|(1<<1);
//DDRD|=(1<<SD_cs)|(1<<sensor1_cs);//sets chip select for SD
PORTD |=(1<<sensor1_cs);//deselect 
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
}//End SPI_send

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
	if(SPDR==128){uart_puts("SRAM init: PASSED");}//got correct byte
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
	//data_count++;
        //uart_putc('.');
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



//records on shot in sram (20seconds) returns the number of records
//need to add error checking on high byte of address (only 0 and 1)
void add_inc()
{
  if(add_l==255 && add_m==255 && add_h==1){vibrate(1000);uart_puts("MEM OVERFLOW");while(1){}}
  
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

}//end add_inc


void mem_test()
{
 add_l=0;
 add_m=0; //reset sram addresses
 add_h=0;
 char print[10];
 uint32_t mem=0;
 for(mem=0;mem<140000;mem++)
 {
  if(mem>240 && mem<300)
  {
     itoa(add_l,print,10);
     uart_puts(print);
     uart_puts("   ");
     itoa(add_m,print,10);
     uart_puts(print);
     uart_puts("   ");
     itoa(add_h,print,10);
     uart_puts(print);
     uart_putc('\n');
     uart_putc('\n');
     _delay_ms(1000);
   } 
  add_inc();

  if(mem%100==0)
  {
  itoa(mem,print,10);
  uart_puts(print);
  uart_putc('\n');
  }//end if
 }//end for loop
while(1){}
}//end mem_test
 
void record_shot()
{
  
  int16_t ax=0,ay=0,az=0;
 // int16_t gx,gy,gz;
 // int16_t mx,my,mz;
  add_l=0;
  add_m=0;//reset addresses to zero
  add_h=0;
  data_count=0; 
  uart_puts("Shot Started...\n");
  
  //get 20 seconds of data
  //each sensor is 14 bytes of data
  //each byte is roughtly 2.5 ms 
  //uint16_t i=0;
  //int16_t x=0;
  uint8_t data[50];
  char addL[10];
  //char addM[5];
  //char addH[5];
  count_t=0;
  num_records=0;
  
  while(1)
  {
    _delay_ms(500);
    
    PORTB&=~(1<<sensor1_cs);//select sensor
    SPIgetAccel(&ax,&ay,&az);
    PORTB|=(1<<sensor1_cs);//deselect sensor
    //getAccel(&ax,&ay,&az,MPU9250_DEFAULT_ADDRESS);
    itoa(ax,addL,10);
    uart_puts("X axis: ");
    uart_puts(addL);
    
    itoa(ay,addL,10);
    uart_puts("Y axis: ");
    uart_puts(addL);
    
    itoa(az,addL,10);
    uart_puts("Z axis: ");
    uart_puts(addL);
    
    
  }
  
  //*********************************stop*******************
  while(count_t<20000)//for full shot change back to 20000************
  {
    num_records++;
    //_delay_us(1000000); 
   // sram_write(add_l,add_m,add_h,(uint8_t)(count_t>>8));
    add_inc();
    data_count++;
   // sram_write(add_l,add_m,add_h,(uint8_t)(count_t));
    add_inc();   
    data_count++; 
//	uart_puts("\nPre-getAccel");
   
    uint8_t i;
    for(i=0;i<3;i++)
    {
    _delay_us(1250);
    getAccel(&ax,&ay,&az, MPU9250_DEFAULT_ADDRESS);//fetch all axis compass readings
	data[0]=(int8_t)(ax>>8); 
    data[1]=(int8_t)ax;
    data[2]=(int8_t)(ay>>8);
    data[3]=(int8_t)ay;
    data[4]=(int8_t)(az>>8);
    data[5]=(int8_t)az;
    
/*
    uart_puts("\nX- ");
    itoa(ax,addL,10);
    uart_puts(addL);
    uart_puts("\tY- ");
    itoa(ay,addL,10);
    uart_puts(addL);
    uart_puts("\tZ- ");
    itoa(az,addL,10);
    uart_puts(addL);
  */  

/*
    getGyro(&ax,&ay,&az, MPU9250_ALT_DEFAULT_ADDRESS);//fetch all axis compass readings
    data[6]=(int8_t)(ax>>8); 
    data[7]=(int8_t)ax;
    data[8]=(int8_t)(ay>>8);
    data[9]=(int8_t)ay;
    data[10]=(int8_t)(az>>8);
    data[11]=(int8_t)az;  

    getAccel(&ax,&ay,&az, MPU9250_DEFAULT_ADDRESS);//fetch all axis compass readings
    data[12]=(int8_t)(ax>>8); 
    data[13]=(int8_t)ax;
    data[14]=(int8_t)(ay>>8);
    data[15]=(int8_t)ay;
    data[16]=(int8_t)(az>>8);
    data[17]=(int8_t)az;
*/
/*
    uart_puts("\nX2- ");
    itoa(ax,addL,10);
    uart_puts(addL);
    uart_puts("    Y2- ");
    itoa(ay,addL,10);
    uart_puts(addL);
    uart_puts("    Z2- ");
    itoa(az,addL,10);
    uart_puts(addL);
*/
  /*  getGyro(&ax,&ay,&az, MPU9250_DEFAULT_ADDRESS);//fetch all axis compass readings
 
    data[18]=(int8_t)(ax>>8); 
    data[19]=(int8_t)ax;
    data[20]=(int8_t)(ay>>8);
    data[21]=(int8_t)ay;
    data[22]=(int8_t)(az>>8);
    data[23]=(int8_t)az;  
*/    
uint8_t _i;

 
 for(_i=0;_i<18;_i++)
    {
     sram_write(add_l,add_m,add_h,data[_i]);
     add_inc(); 
     data_count++;
    }
}//end for loop
  }//end timing while loop 
}//end record_shot

void ADC_init()
{
//ADCSRB|=(1<<ACME);
//ADMUX=0;
//ADCSRA|=(1<<ADEN)|(1<<ADSC)|(0<<ADFR);


}

void print_shot()
{
  char print[20];

  uart_puts("Number of Bytes: ");
  ltoa(data_count,print,10);
  uart_puts(print);
  uart_putc('\n');
  _delay_ms(5000);  
  add_l=0;
  add_m=0;
  add_h=0;
  uint32_t x;
  for(x=0;x<data_count;x++)
  {
    itoa(sram_read(add_l,add_m,add_h),print,10);
    ltoa(x,print,10);
    add_inc();
    //uart_puts(print);
    //uart_putc(',');
    if(x%100==0){uart_puts(print);
    uart_putc('\n');}
    
  }
}

void init_AD()
{

}

void check_voltage()
{//Read System Voltage
 //if below 3.8 halt and spin on blinking red light
//if below 3.7 shutdown



}

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

//uint8_t test_result=0;
//************************( 1 )*******************************************************
DDRD|=(1<<6)|(1<<7);//set LED pins as output
//PORTD&=~(1<<3);//vibration off
uart_init();//Keep this as first init so that text can be sent out in others
uart_puts("Starting up....");
_delay_ms(500);
PORTD|=1<<6;
_delay_ms(500);
PORTD&=~(1<<6);
spi_init(); //initialize SPI bus as master
init_tcnt2();//set up timer (RTC)
init_twi(); //initialize TWI interface
sei();
uart_puts("Pre Init...");
  
PORTB&=~(1<<sensor1_cs);//select sensor
SPIinit_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS);//init sensor
PORTB|=(1<<sensor1_cs);//deselect sensor
//init_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS, MPU9250_DEFAULT_ADDRESS); //initialize the 9axis sensor
//init_MPU(0,0, 0xD1); //initialize the 9axis sensor
//init_MPU(MPU9250_FULL_SCALE_4G,MPU9250_GYRO_FULL_SCALE_500DPS, MPU9250_DEFAULT_ADDRESS); //initialize the 9axis sensor
sram_init();//initialize sram
uart_puts("Post Init...");
//vibrate(100);	//Send feedback showing complete setup
PORTD |=(1<<6)|(1<<7);
PORTD &=~(1<<7);
_delay_ms(200);
PORTD |=(1<<7);
PORTD &=~(1<<6);// blinks both lights to show the program is starting
_delay_ms(200);
PORTD |=(1<<6);
_delay_ms(1000);
record_shot();
while(1){}        //*****************STOP POINT**********************
/*
test_result = test_com(0, MPU9250_DEFAULT_ADDRESS);
if(test_result)
{
uart_puts("MPU Status: OK\n");
}
else
uart_puts("MPU Status: FAILURE... You suck!\n");
*/
char rx_char;
uint16_t i=0;//used for for loops
//*****Fix me*******  
//should set to int 0 not char '0' but the ascii zero prints better for now
for(i=0;i<512;i++){sd_buf[i]='0';}//sets inital buffer to zero values
uint16_t shot_count=0;
while(1)
{
//************************( 2 )*******************************************************

PORTD &=~(1<<7);
_delay_ms(2000);//Show red light for 2 sec, then turn green and start shot
PORTD |=(1<<7);
PORTD &=~(1<<6);
//************************( 3 )*******************************************************
//vibrate(100);

record_shot();//record a shot
char shots_s[10];
itoa(num_records,shots_s,10);
uart_puts("In 20 seconds The number of records was: ");
uart_puts(shots_s);
uart_putc('\n');

print_shot();
shot_count++;
itoa(shot_count,shots_s,10);
uart_puts("Shot Number: ");
uart_puts(shots_s);
uart_putc('\n');
//print_shot();


PORTD |=(1<<6);//turn off light
//vibrate(100);_delay_ms(100);vibrate(100);  //Double vibration showing end of shot
//check_voltage();//Check system voltage
_delay_ms(5000);//wait 60 seconds
//************************( 6 )*******************************************************
continue; //start over and take another shot

  uart_puts("Starting Testing\n\n");
  uart_putc('\r');
  rx_char=uart_getc(); 
  if(rx_char=='c')
  {
    uart_puts("Command line:\n\n");
    rx_char=uart_getc();
    while(rx_char!='c')
    {
      if(rx_char=='s'){}
     if(rx_char=='w'){}
      if(rx_char=='r'){}//buffer gets set to sector!!
      if(rx_char=='i'){}
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
