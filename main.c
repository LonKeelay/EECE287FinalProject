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
#define Clockwise 1
#define CounterClockwise 2
#define Forward 3
#define Reverse 4
#define Slow 1
#define Medium 2
#define Fast 3
/*
movement: 1=CW, 2=CCW, 3=F, 4=R
speed: 1=S, 2=M, 3=F
time: the int is in units of 1/10 s
commands: Amount of commands set
*/
uint8_t movement[4];
uint8_t speed[4];
uint8_t time[4];
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
	uint8_t chosen = 0;

	int menuIndex = 0;
	char menuItems[4][8] = {"   CW   ", "  CCW   ", "   FW   ", "  REV   "};
	char arrowUI[] = "<  --  >";

	while(!chosen){
		waitForNoInput();
		LCD_execute_command(CLEAR_DISPLAY);
		LCD_execute_command(MOVE_CURSOR_HOME);
		LCD_print_String(menuItems[menuIndex]);
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_String(arrowUI);

		switch(get_input()){
			case 0: // Left
				if(menuIndex == 0){
					menuIndex = 3;
				}else{
					menuIndex--;
				}
				break;
			case 2: // Right
				if(menuIndex == 3){
					menuIndex = 0;
				}else{
					menuIndex++;
				}
				break;
			case 1:
				chosen = 1;
				break;
		}
	}

	movement[i] = menuIndex + 1; //Index is offset by 1 due to 0 meaning no command
}

void speedMenu(uint8_t i){
	waitForNoInput();
	uint8_t chosen = 0;

	int menuIndex = 0;
	char menuItems[3][8] = {"  SLOW  ", " MEDIUM ", "  FAST  "};
	char arrowUI[] = "<  --  >";

	while(!chosen){
		waitForNoInput();
		LCD_execute_command(CLEAR_DISPLAY);
		LCD_execute_command(MOVE_CURSOR_HOME);
		LCD_print_String(menuItems[menuIndex]);
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_String(arrowUI);

		switch(get_input()){
			case 0: // Left
				if(menuIndex == 0){
					menuIndex = 2;
				}else{
					menuIndex--;
				}
				break;
			case 2: // Right
				if(menuIndex == 2){
					menuIndex = 0;
				}else{
					menuIndex++;
				}
				break;
			case 1:
				chosen = 1;
		}
	}

	speed[i] = menuIndex + 1; //Index is offset by 1 due to 0 meaning no command
}

void timeMenu(uint8_t i){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	//Have to reinvent the wheel for this, as there is nothing in the given library to print a decimal value
	uint8_t tim = 0; // Highest duration is 25.5 seconds, though why would you subject yourself to such torture

	uint8_t seld = 0;
	while(!seld){
		//Format of number on top LCD would be ' xy.z s '
		LCD_move_cursor_to_col_row(1, 0);
		LCD_print_hex4(tim/100);
		LCD_print_hex4((tim%100)/10);
		LCD_print_String(".");
		LCD_print_hex4(tim%10);
		LCD_move_cursor_to_col_row(6, 0);
		LCD_print_String("s");
		LCD_move_cursor_to_col_row(0, 1);
		LCD_print_String("<  --  >");
		switch(get_input()){
			case 0: // Left
				tim--;
				break;
			case 2: // Right
				tim++;
				break;
			case 1: // Center
				seld = 1;
		}
		waitForNoInput();
	}

	time[i] = tim;
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

	// Erase all residual commands
	commands = 0;
	for(int i = 0; i < 4; i++){
		movement[i] = 0;
		speed[i] = 0;
		time[i] = 0;
	}

	//Initialize variables used for UI
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