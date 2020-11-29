/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:022
 *	Assignment: Lab 11  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/mYO0QAreo5g
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

task tasks[5];
const unsigned short tasksNum=5;
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
unsigned char go=0;

enum States1{Start1,fest1};
unsigned char led=0;
unsigned char t1=0;
int Tick1(int state1){
	switch(state1){
		case Start1:
			led=0;
			if(go==1){
				state1=fest1;
				t1=0;
			}
			break;
		case fest1:
			if(go==1){
				state1=fest1;
			}
			else{
				state1=Start1;
			}
			break;
		default:
			break;
	}
	switch(state1){
		case Start1:
			break;
		case fest1:
			if(t1<8){
				led=(led|0x01<<t1);
			}
			else if((t1>=8)&&(t1<16)){
				led=(led>>1);
			}
			t1=t1+1;
			if(t1>=16){
				t1=0;
			}
			break;
		default:
			break;
	}
	return state1;
}

enum States2{Start2,fest2};
unsigned char led2=0;
unsigned char t2=0;
int Tick2(int state2){
        switch(state2){
                case Start2:
                        led2=0;
			if(go==2){
				t2=0;
				state2=fest2;
			}
                        break;
                case fest2:
			if(go==2){
                                state2=fest2;
                        }
                        else{
                                state2=Start2;
                        }
                        break;
                default:
                        break;
        }
        switch(state2){
                case Start2:
                        break;
                case fest2:
			led2=((0x01<<t2)|(0x80>>t2));
			if(t2<7){                 
				t2=t2+1;
			}
			else{
				t2=0;
			}
                        break;
                default:
                        break;
        }
        return state2;
}
enum States3{Start3,fest3};
unsigned char led3=0x55;
unsigned char t3=0;
int Tick3(int state3){
        switch(state3){
                case Start3:
                        led3=0;
			if(go==3){
				t3=0;
				state3=fest3;
			}
			else{
				state3=Start3;
			}
                        break;
                case fest3:
			if(go==3){
                                state3=fest3;
                        }
                        else{
                                state3=Start3;
                        }
                        break;
                default:
                        break;
        }
        switch(state3){
                case Start3:
                        break;
                case fest3:
                        if(t3%2==0){
				led3=0x55;
			}
			else{
				led3=0xAA;
			}
			t3++;
                        break;
                default:
                        break;
        }
        return state3;
}
unsigned char flag;
enum switchled{Sstart,Off,Off_P,On,On_P,P0,P1};
int Tick_switch(int s_state){
	switch(s_state){
		case Sstart:
			s_state=Off;
			go=0;
			break;
		case Off:
			if((!A0)&&(!A1)){
				s_state=Off;
			}
			else if(A0&&A1){
				s_state=Off_P;
			}
			break;
		case Off_P:
			if((!A0)&&(!A1)){
                                s_state=On;
				go=1;
                        }
                        else if(A0&&A1){
                                s_state=Off_P;
                        }
                        break;

		case On:
			if(A0&&A1){
				s_state=On_P;
			}
			else if((!A0)&&(!A1)){
				s_state=On;
			}
			else if(A0&&(!A1)){
				s_state=P0;
			}
			else if((!A0)&&A1){
				s_state=P1;
			}
			break;
		case On_P:
			if(A0&&A1){
                                s_state=On_P;
                        }
			else{
				s_state=Off;
				go=0;
			}
			break;
		case P0:
			if(A0&&(!A1)){
				s_state=P0;
			}
			else if((!A0)&&(!A1)){
				s_state=On;
				if(go<3){
                                        go=go+1;
                                }
                                else{
                                        go=1;
                                }
			}
			else if(A0&&A1){
				s_state=On_P;
				go=0;
			}
			break;
		case P1:
                        if(A1&&(!A0)){
                                s_state=P1;
                        }
                        else if((!A0)&&(!A1)){
                                s_state=On;
				if(go>1){
                                        go=go-1;
                                }
                                else{
                                        go=3;
                                }
                        }
			else if(A0&&A1){
				s_state=On_P;
				go=0;
			}
                        break;
		default:
			break;
	}
	switch(s_state){
		case Off:
			go=0;
			break;
		default:
			break;
	}
	return s_state;
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
			if(go==1){
				transmit_data(led);
			}
			else if(go==2){
				transmit_data(led2);
			}
			else if(go==3){
				transmit_data(led3);
			}
			else if(go==0){
				transmit_data(0x00);
			}
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
	tasks[i].period=200;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Tick1;
	i++;
        tasks[i].state=Start2;
        tasks[i].period=300;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick2;
	i++;
	tasks[i].state=Start3;
        tasks[i].period=200;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick3;
        i++;
	tasks[i].state=Sstart;
        tasks[i].period=100;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_switch;
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
