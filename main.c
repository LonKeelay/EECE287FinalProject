#include <stdint.h>
#include <stdio.h> 
#include <util/delay.h>
#include <avr/io.h>
#include "lcd_driver.h"
#include "port_macros.h"

//Definitions for buttons
#define btn_a (~(PINB) & (1<<1))
#define btn_b (~(PINB) & (1<<4))
#define btn_c (~(PINB) & (1<<5))

//Definitions for movement parameters
#define Clockwise 0
#define CounterClockwise 1
#define Forward 2
#define Reverse 3
#define Slow 0
#define Medium 1
#define Fast 2
/*
movement: 0=CW, 1=CCW, 2=F, 3=R
speed: 0=S, 1=M, 2=F
time: the int is in units of 1/10 s
*/
uint8_t movement[4];
uint8_t speed[4];
uint16_t time[4];

void init_buttons(){
	DDRB &= ~(1<<1);
	PORTB |= (1<<1);
	DDRB &= ~(1<<4);
	PORTB |= (1<<4);
	DDRB &= ~(1<<5);
	PORTB |= (1<<5);
}

void init_LCD(){
	initialize_LCD_driver();
	LCD_execute_command(TURN_ON_DISPLAY);
	LCD_print_String("Group 42");
}

void create_comm(){
	unsigned int a_pressed = 0;
	unsigned int b_pressed = 0;
	unsigned int c_pressed = 0;
	
	char finStr[] = "   ";

	LCD_execute_command(CLEAR_DISPLAY);

	while(1){
		if(btn_a){
			if(a_pressed == 0){
				finStr[0] = 'A';
				a_pressed = 1;
			}
		}else{
			finStr[0] = ' ';
			a_pressed = 0;
		}
		if(btn_b){
			if(b_pressed == 0){
				finStr[1] = 'B';
				b_pressed = 1;
			}
		}else{
			finStr[1] = ' ';
			b_pressed = 0;
		}
		if(btn_c){
			if(c_pressed == 0){
				finStr[2] = 'C';
				c_pressed = 1;
			}
		}else{
			finStr[2] = ' ';
			c_pressed = 0;
		}

		LCD_execute_command(MOVE_CURSOR_HOME);
		LCD_print_String(finStr);
	}
}

int main(){

	init_buttons();
	init_LCD();
	create_comm();

	return 0;
}