/*
 * SPI.h
 *
 * Created: 4/16/2016 8:08:45 PM
 *  Author: gbone
 */ 
#ifndef SPI_H_
#define SPI_H_

//CS Devices
#define sensor1_cs 2 //PORTB
#define sensor2_cs 7 //PORTD
#define sensor3_cs 6 //PORTD
#define sram_cs 0 //PORTC

#define PinHIGH(port, pin) port |= (1<<pin)
#define PinLOW(port, pin) port &= ~(1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

void init_SPI(void);
char SPI_send(char chr);
void select(uint8_t slvdevice);
void deselect(uint8_t slvdevice);
int spi_writeRegs(unsigned char sel, unsigned char reg_addr,
unsigned char length, unsigned char const *data);
int spi_readRegs(unsigned char sel, unsigned char reg_addr,
unsigned char length, unsigned char *data);
#endif /* SPI_H_ */