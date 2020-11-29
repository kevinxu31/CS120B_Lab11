/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:022
 *	Assignment: Lab 11  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/cWlnuOUVSGM
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


typedef struct task{
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int(*TickFct)(int);
}task;

task tasks[2];
const unsigned short tasksNum=2;
const unsigned long tasksPeriod=100;
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void TimerOn() {
	TCCR1B 	= 0x0B;
	OCR1A 	= 125;
	TIMSK1 	= 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B 	= 0x00;
}

void TimerISR() {
	unsigned char i;
	for(i=0;i<tasksNum;++i){
		if(tasks[i].elapsedTime>=tasks[i].period){
			tasks[i].state=tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime=0;
		}
		tasks[i].elapsedTime+=tasksPeriod;
	}
}
ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}



void transmit_data(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
   	 PORTC = 0x08;
   	 PORTC |= ((data >> i) & 0x01);
   	 PORTC |= 0x02;  
    }
    PORTC |= 0x04;
    PORTC = 0x00;
}
#define A0 (~PINA&0x01)
#define A1 ((~PINA>>1)&0x01)

enum States1{Start1,RA,PA_0,PA_1,Reset};
unsigned char led=0;
int Tick1(int state1){
	switch(state1){
		case Start1:
			led=0x00;
			state1=RA;
			break;
		case RA:
			if(A0&(!A1)){
				state1=PA_0;
				if(led<0xFF){
					led=led+1;
				}
			}
			else if(A0&A1){
				state1=Reset;
				led=0;
			}
			else if((!A0)&A1){
				state1=PA_1;
				if(led>0){
					led=led-1;;
				}
			}
			else{state1=RA;}
			break;
		case PA_0:
			if(A0&(!A1)){state1=PA_0;}
			else if((!A0)&(!A1)){state1=RA;}
			else if(A0&A1){
				state1=Reset;led=0;
			}
			break;
                case PA_1:
                        if(A1&(!A0)){state1=PA_1;}
                        else if((!A0)&(!A1)){state1=RA;}
                        else if(A0&A1){
				state1=Reset;led=0;
			}
                        break;
		case Reset:
			if(!A1&!A0){state1=RA;}
			else{state1=Reset;}
			break;
		default:
			break;
	}
	return state1;
}
enum CombineLEDsSM{C_start,C};
int Tick_C(int C_state){
	switch(C_state){
		case C_start:
			C_state=C;
			break;
		case C:
			C_state=C;
			break;
		default:
			break;
	}
	switch(C_state){
		case C_start:
			break;
		case C:
			transmit_data(led);
			break;
		default:
			break;
	}
	return C_state;
}

int main(void) {
	DDRA=0x00;PORTA=0xFF;
	DDRC=0xFF;PORTC=0x00;
	unsigned char i=0;
	tasks[i].state=Start1;
	tasks[i].period=100;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Tick1;
	i++;
	tasks[i].state=C_start;
        tasks[i].period=100;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_C;
	TimerSet(100);
	TimerOn();
	while (1) {
	}
	return 1;
}
