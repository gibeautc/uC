/*
 * Feedback.h
 *
 * Created: 4/15/2016 5:35:12 PM
 *  Author: gbone
 */ 

#ifndef FEEDBACK_H_
#define FEEDBACK_H_

#define G_LED 6
#define R_LED 7
#define X_LED 0
#define Vibration 5
#define PinHIGH(port, pin) port |= (1<<pin)
#define PinLOW(port, pin) port &= ~(1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)
#define Pulse_1 0
#define Pulse_3 1
#define Pulse_Long 2
#define Toggle 3
#define Alternate 4

void init_Feedback(void);
void LED(uint8_t LED_Sel, uint8_t Pattern);
void Vibrate(uint8_t Pattern);

#endif /* FEEDBACK_H_ */