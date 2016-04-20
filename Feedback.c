/*
 * Feedback.c
 *
 * Created: 4/15/2016 5:09:48 PM
 *  Author: gbone
 */ 
#ifndef F_CPU
#define F_CPU 8000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "Feedback.h"

volatile unsigned long count_t=0;

void init_Feedback(void)
{
	DDRD |= (1<<R_LED)|(1<<G_LED)|(1<<Vibration);
	PORTD |= (1<<R_LED)|(1<<G_LED)|(0<<Vibration);
}

void LED(uint8_t LED_Sel, uint8_t Pattern)
{
	switch (Pattern)
	{
		case Pulse_1:
			PinLOW(PORTD, LED_Sel);
			_delay_ms(200);
			PinHIGH(PORTD, LED_Sel);
			break;
		case Pulse_3:
			PinLOW(PORTD, LED_Sel);
			_delay_ms(200);
			PinHIGH(PORTD, LED_Sel);
			_delay_ms(200);
			PinLOW(PORTD, LED_Sel);
			_delay_ms(200);
			PinHIGH(PORTD, LED_Sel);
			_delay_ms(200);
			PinLOW(PORTD, LED_Sel);
			_delay_ms(200);
			PinHIGH(PORTD, LED_Sel);
			break;
		case Pulse_Long:
			PinLOW(PORTD, LED_Sel);
			_delay_ms(1000);
			PinHIGH(PORTD, LED_Sel);
			break;
		case Alternate:
			PinLOW(PORTD, R_LED);
			_delay_ms(200);
			PinHIGH(PORTD, R_LED);
			_delay_ms(10);
			PinLOW(PORTD, G_LED);
			_delay_ms(200);
			PinHIGH(PORTD, G_LED);
			PinLOW(PORTD, R_LED);
			_delay_ms(200);
			PinHIGH(PORTD, R_LED);
			_delay_ms(10);
			PinLOW(PORTD, G_LED);
			_delay_ms(200);
			PinHIGH(PORTD, G_LED);
		default:
			break;
	}
	return;
}

void Vibrate(uint8_t Pattern)
{
	switch (Pattern)
	{
		case Pulse_1:
		PinLOW(PORTD, Vibration);
		_delay_ms(200);
		PinHIGH(PORTD, Vibration);
		break;
		case Pulse_3:
		PinHIGH(PORTD, Vibration);
		_delay_ms(200);
		PinLOW(PORTD, Vibration);
		_delay_ms(200);
		PinHIGH(PORTD, Vibration);
		_delay_ms(200);
		PinLOW(PORTD, Vibration);
		_delay_ms(200);
		PinHIGH(PORTD, Vibration);
		_delay_ms(200);
		PinLOW(PORTD, Vibration);
		break;
		case Pulse_Long:
		PinLOW(PORTD, Vibration);
		_delay_ms(1000);
		PinHIGH(PORTD, Vibration);
		break;
		default:
		break;
	}
	return;
}

void init_tcnt2()
{
	ASSR |=(0<<AS2);//Run of 32khz osc
	TIMSK2=0x00;  //reset TIMSK
	TIMSK2 |=(1<<TOIE2);//turns on comp match interupt
	TCCR2A=0;
	TCCR2B=(1<<CS21)|(1<<CS20);//Normal mode prescale 32 should give a 1ms count
}

void get_ms(unsigned long *timestamp){
	timestamp = count_t;
}

ISR(TIMER2_OVF_vect)
{
	count_t++;
}
