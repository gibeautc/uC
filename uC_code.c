#define F_CPU  8000000UL
#include <avr/io.h>
#include <util/delay.h>


int main()
{
 DDRB = 0xFF; 
while(1)
  { 
PORTB=(0<<1);
 _delay_ms(500);
PORTB=(1<<1);
  _delay_ms(500);  
 } //end while 
} //end main
