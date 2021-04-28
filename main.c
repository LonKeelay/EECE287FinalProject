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
commands: Amount of commands set
*/
uint8_t movement[4];
uint8_t speed[4];
uint16_t time[4];
uint8_t commands;

/*
	Initializes the buttons and makes sure they start out at 0
*/
void init_buttons(){
	DDRB &= ~(1<<1);
	PORTB |= (1<<1);
	DDRB &= ~(1<<4);
	PORTB |= (1<<4);
	DDRB &= ~(1<<5);
	PORTB |= (1<<5);
}

/*
	Initializes the LCD and displays a boot message
	Group 42 just happens to take up 8 characters which is nice
*/
void init_LCD(){
	initialize_LCD_driver();
	LCD_execute_command(TURN_ON_DISPLAY);
	LCD_print_String("Group 42");
}

/*
	Code to test the LCD and buttons
*/
void test_comm(){
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

/*
	Gets the button input
	@returns 0 if a, 1 if b, 2 if c
*/
int get_input(){
	while(1){
		if(btn_a){
			return 0;
		}
		if(btn_b){
			return 1;
		}
		if(btn_c){
			return 2;
		}
	}
}

void waitForNoInput(){
	while(btn_a || btn_b || btn_c){}
}

void direcMenu(uint8_t i){
	waitForNoInput();
}

void speedMenu(uint8_t i){
	waitForNoInput();
}

void timeMenu(uint8_t i){
	waitForNoInput();
}

uint8_t goMenu(){
	waitForNoInput();
	return 0;
}

/*
	Creates a UI to set the arrays of movement, speed, and time
*/
void create_comm(){
	uint8_t instCreated = 0;
	uint8_t finUI = 0;

	int commIndex = 0;
	char commElements[4][8] = {" DIREC  ", " SPEED  ", "  TIME  ", "   GO   "};
	char arrowUI[] = "<  --  >"; // Fancy bottom arrows
	
	// Here comes the spaghet
	while(!finUI){
		waitForNoInput();
		LCD_execute_command(CLEAR_DISPLAY);
		LCD_execute_command(MOVE_CURSOR_HOME);
		LCD_print_String(commElements[commIndex]);
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_String(arrowUI);

		switch(get_input()){
			case 0: // Left
				if(commIndex == 0){
					commIndex = 3;
				}else{
					commIndex--;
				}
				break;
			case 2: // Right
				if(commIndex == 3){
					commIndex = 0;
				}else{
					commIndex ++;
				}
				break;
			case 1: // Center
				switch(commIndex){
					case 0:
						direcMenu(instCreated);
						break;
					case 1:
						speedMenu(instCreated);
						break;
					case 2:
						timeMenu(instCreated);
						break;
					case 3:
						if (goMenu()){ // 0 is continue
							finUI = 1;
						}
				}
		}
	}

	commands = instCreated;
}

int main(){

	init_buttons();
	init_LCD();
	create_comm();

	return 0;
}