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

//Definition of Motor speed PWM
//Greater than max duty cycle to avoid hurting the motors
#define PWM_TOP 120

//Bias definitions- must manually set for own bot
#define BIAS_L_CCW 0
#define BIAS_L_CW 0
#define BIAS_R_CCW 0
#define BIAS_R_CW 0

/*
movement: 1=CW, 2=CCW, 3=F, 4=R
speed: 1=S, 2=M, 3=F
time: the int is in units of 1/10 s
commands: Amount of commands set, commands will be ran in a row
*/
uint8_t movement[4];
uint8_t speed[4];
//uint8_t time[4];
uint8_t commands;

//Motor speed variables
uint8_t duty_cycle = 15;
uint8_t pwm_counter_L = 0;//must be set to 0 before each new command
uint8_t pwm_counter_R = 0;//must be set to 0 before each new command

//Time variables
//uint8_t run_timer = 0; //Perhaps increment this each time a delay is ran?
//Testing speed of bot
uint16_t run_timer = 0;
uint16_t time[4];


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

void init_motors()
{
	DDRD |= (1<<5);//PD5: Left, CCW 
	PORTD &= ~(1<<5); // was orginally DDRD, same for all others

	DDRD |= (1<<6);//PD6: Left, CW
	PORTD &= ~(1<<6);

	DDRD |= (1<<3);//PD3: Right, CCW
	PORTD &= ~(1<<3);

	DDRB |= (1<<3);//PB3: Right, CW
	PORTB &= ~(1<<3);
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

void test_motors(){
	unsigned int a_pressed = 0;
	unsigned int b_pressed = 0;
	unsigned int c_pressed = 0;

	while(1){
		if(btn_a){
			if(a_pressed == 0){
				PORTD |= (1<<5); 
				a_pressed = 1;
			}
		}else{
			PORTD &= ~(1<<5);
			a_pressed = 0;
		}
		if(btn_b){
			if(b_pressed == 0){
				PORTD |= (1<<6); 
				b_pressed = 1;
			}
		}else{
			PORTD &= ~(1<<6);
			b_pressed = 0;
		}
		if(btn_c){
			if(c_pressed == 0){
				PORTD |= (1<<3); 
				c_pressed = 1;
			}
		}else{
			PORTD &= ~(1<<3);
			c_pressed = 0;
		}
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

	int menuIndex;
	if(movement[i] == 0){menuIndex = 0;}
	else{menuIndex = movement[i] - 1;}
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

	int menuIndex;
	if (speed[i] == 0){menuIndex = 0;}
	else{menuIndex = speed[i] - 1;}

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
	uint16_t tim = time[i]; // Highest duration is 5.5 seconds, including processing time
	//increments by 1000

	uint8_t seld = 0;
	while(!seld){
		//Format of number on top LCD would be ' xy.z s '
		LCD_move_cursor_to_col_row(1, 0);
		LCD_print_hex4(tim/100); //unchanged for now, don't quite understand it without testing
		LCD_print_hex4((tim%100)/10);
		LCD_print_String(".");
		LCD_print_hex4(tim%100);
		LCD_move_cursor_to_col_row(6, 0);
		LCD_print_String("s");
		LCD_move_cursor_to_col_row(0, 1);
		LCD_print_String("<  --  >");
		switch(get_input()){
			case 0: // Left
				if(tim >= 1000){
					tim = tim - 1000;
				}
				break;
			case 2: // Right
				if(tim <= 49000){
					tim = tim + 1000;
				}
				break;
			case 1: // Center
				seld = 1;
		}
		waitForNoInput();
	}

	time[i] = tim;
}
/*
	@returns 0 to continue looping, 1 to stop loop
*/
uint8_t goMenu(){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	//Check if all commands are filled
	if(movement[commands] == 0 || speed[commands] == 0 || time[commands] == 0){
		LCD_print_String("COMMAND");
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_String("NOT SET");
		get_input();
		return 0;
	}
	// Ask user if they would like to add another command
	LCD_print_String("ADD COM?");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("< NO YES");
	switch(get_input()){
		case 0:
			return 0;
			break;
		case 1:
			if(commands == 3){
				LCD_execute_command(CLEAR_DISPLAY);
				LCD_execute_command(MOVE_CURSOR_HOME);
				LCD_print_String("CANT MAKE 5 COMS");
				get_input();
				commands++;
				return 1;
			}
			commands++;
			return 0;
			break;
		case 2:
			commands++;
			return 1;
	}
	return 0;
}

/*
	Creates a UI to set the arrays of movement, speed, and time
*/
void create_comm(){
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
						direcMenu(commands);
						break;
					case 1:
						speedMenu(commands);
						break;
					case 2:
						timeMenu(commands);
						break;
					case 3:
						if (goMenu()){ // 0 is continue
							finUI = 1;
						}
				}
		}
	}
}


//These run_motor funcs need to be seperate for two reasons:
//Too complex to compile into one
//Will be useful later to account for differnt biases
//Also it will eventually accept speed[i] values

void run_motor_L_CCW(int cmd, int bias)
{
	pwm_counter_L = pwm_counter_L + 1;
	if(pwm_counter_L >= PWM_TOP){
		pwm_counter_L = 0;
	}
	if(pwm_counter_L < duty_cycle*speed[cmd]){ //speed is set by user
		PORTD |= (1<<5);
	}
	else{
		PORTD &= ~(1<<5);
	}
	_delay_us(50);
}

void run_motor_L_CW(int cmd, int bias)
{
	pwm_counter_L = pwm_counter_L + 1;
	if(pwm_counter_L >= PWM_TOP){
		pwm_counter_L = 0;
	}
	if(pwm_counter_L < duty_cycle*speed[cmd]){
		PORTD |= (1<<6);
	}
	else{
		PORTD &= ~(1<<6);
	}
	_delay_us(50);
}

void run_motor_R_CCW(int cmd, int bias)
{
	pwm_counter_R = pwm_counter_R + 1;
	if(pwm_counter_R >= PWM_TOP){
		pwm_counter_R = 0;
	}
	if(pwm_counter_R < duty_cycle*speed[cmd]){
		PORTD |= (1<<3);
	}
	else{
		PORTD &= ~(1<<3);
	}
	_delay_us(50);
}

void run_motor_R_CW(int cmd, int bias)
{
	pwm_counter_R = pwm_counter_R + 1;
	if(pwm_counter_R >= PWM_TOP){
		pwm_counter_R = 0;
	}
	if(pwm_counter_R < duty_cycle*speed[cmd]){
		PORTB |= (1<<3);
	}
	else{
		PORTB &= ~(1<<3);
	}
	_delay_us(50);
}

void run_motors(int cmd, int movement)
{
	switch(movement)
	{
		case 1://CCW
			run_motor_L_CCW(cmd, BIAS_L_CCW);
			run_motor_R_CW(cmd, BIAS_R_CW);
			break;
		case 2://CW
			run_motor_L_CW(cmd, BIAS_L_CW);
			run_motor_R_CCW(cmd, BIAS_R_CCW);
			break;
		case 3://FWRD
			run_motor_L_CW(cmd, BIAS_L_CW);
			run_motor_R_CW(cmd, BIAS_R_CW);
			break;
		case 4://BKWRD
			run_motor_L_CCW(cmd, BIAS_L_CCW);
			run_motor_R_CCW(cmd, BIAS_R_CCW);
			break;
		default://Error
			printf("No.");
			break;
	}
}

void run_commands()
{
	
	for(uint8_t cur_cmd = 0; cur_cmd < commands; cur_cmd++)//will run thru each command after they are selected
	{
		
		pwm_counter_L = 0;
		pwm_counter_R = 0;
		run_timer = 0;
		while(run_timer <= time[cur_cmd])
		{
			run_motors(cur_cmd, movement[cur_cmd]);
			run_timer = run_timer + 1;
		}
	}
}


int main(){

	init_buttons();
	init_LCD();
	init_motors();
	while(1)
	{
		create_comm();
		run_commands();
	}
	/*
	speed[0] = 1;
	while(1)
	{
		run_motors(0,1);
	}
	*/
	/*
	commands = 1;
	speed[0] = 1;
	movement[0] = 3;
	time[0] = 50000;
	run_commands();
	*/
	
	return 0;
}