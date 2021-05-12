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

//Definitions for buttons
#define sen_LM (~(PINC) & (1<<0))
#define sen_LC (~(PINC) & (1<<1))
#define sen_CM (~(PINC) & (1<<2))
#define sen_RC (~(PINC) & (1<<3))
#define sen_RM (~(PINC) & (1<<4))

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
#define BIAS_L_CW 4
#define BIAS_R_CCW 5
#define BIAS_R_CW 0

/*
movement: 1=CW, 2=CCW, 3=F, 4=R
speed: 1=S, 2=M, 3=F
time: in us, min0 max 50000
commands: Amount of commands set, commands will be ran in a row
*/
uint8_t movement[4];
uint8_t speed[4];
uint16_t time[4];
uint8_t commands;
uint8_t bunkers;
uint8_t claimed_bunkers;

//Motor speed variables
uint8_t duty_cycle = 15;
uint8_t pwm_counter_L = 0;//must be set to 0 before each new command
uint8_t pwm_counter_R = 0;//must be set to 0 before each new command

//Time variables
uint16_t run_timer = 0; //Increments after each pass of run_motors
uint8_t time_disp = 0; //Displays set time

uint16_t r_counter = 10;
uint16_t l_counter = 10;

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
	//PD5: Left, CCW 
	DDRD &= ~(1<<5);
	PORTD &= ~(1<<5);
	//PD6: Left, CW
	DDRD &= ~(1<<6);
	PORTD &= ~(1<<6);
	//PD3: Right, CCW
	DDRD &= ~(1<<3);
	PORTD &= ~(1<<3);
	//PB3: Right, CW
	DDRB &= ~(1<<3);
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

void init_sensors()
{
	//LM sensor
	DDRC &= ~(1<<0);
	PORTC |= (1<<0);
	//LC sensor
	DDRC &= ~(1<<1);
	PORTC |= (1<<1);
	//CM sensor
	DDRC &= ~(1<<2);
	PORTC |= (1<<2);
	//RC sensor
	DDRC &= ~(1<<3);
	PORTC |= (1<<3);
	//RM sensor
	DDRC &= ~(1<<4);
	PORTC |= (1<<4);
}

/*
	Turns off all motors, prevents glitches where motors keep running after command execution
*/
void deact_motors()
{
	PORTD &= ~(1<<5);
	PORTD &= ~(1<<6);
	PORTD &= ~(1<<3);
	PORTB &= ~(1<<3);	
}


void test_sensors()
{

	uint8_t LM_pressed = 0;
	uint8_t LC_pressed = 0;
	uint8_t CM_pressed = 0;
	uint8_t RC_pressed = 0;
	uint8_t RM_pressed = 0;
	
	char finStr[] = "     ";

	LCD_execute_command(CLEAR_DISPLAY);

	while(1){
		if(sen_LM){
			if(LM_pressed == 0){
				finStr[0] = '0';
				LM_pressed = 1;
			}
		}else{
			finStr[0] = ' ';
			LM_pressed = 0;
		}
		if(sen_LC){
			if(LC_pressed == 0){
				finStr[1] = '1';
				LC_pressed = 1;
			}
		}else{
			finStr[1] = ' ';
			LC_pressed = 0;
		}

		if(sen_CM){
			if(CM_pressed == 0){
				finStr[2] = '2';
				CM_pressed = 1;
			}
		}else{
			finStr[2] = ' ';
			CM_pressed = 0;
		}

		if(sen_RC){
			if(RC_pressed == 0){
				finStr[3] = '3';
				RC_pressed = 1;
			}
		}else{
			finStr[3] = ' ';
			RC_pressed = 0;
		}
		if(sen_RM){
			if(RM_pressed == 0){
				finStr[4] = '4';
				RM_pressed = 1;
			}
		}else{
			finStr[4] = ' ';
			RM_pressed = 0;
		}

		LCD_execute_command(MOVE_CURSOR_HOME);
		LCD_print_String(finStr);
	}
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
	
	uint16_t tim = time[i]; // Highest duration is 5.5s, including processing time(5s without)
	//Each unit is 100 microseconds + processing, so it requires alot of numbers.
	//1000 is .1 seconds

	time_disp = 0;//simpler way to mark the time increasing/decreasing, especially now that the nums are in the 1000s

	uint8_t seld = 0;
	while(!seld){
		//Format of number on top LCD would be ' x.y s '
		LCD_move_cursor_to_col_row(2, 0);
		LCD_print_hex4((time_disp)/10);
		LCD_print_String(".");
		LCD_print_hex4(time_disp%10);
		LCD_move_cursor_to_col_row(6, 0);
		LCD_print_String("s");
		LCD_move_cursor_to_col_row(0, 1);
		LCD_print_String("<  --  >");
		switch(get_input()){
			case 0: // Left
				if(tim >= 1000){//Cannot go below 0s
					tim = tim - 1000;
					time_disp = time_disp - 1;
				}
				break;
			case 2: // Right
				if(tim <= 49000){//cannot go above 5s
					tim = tim + 1000;
					time_disp = time_disp + 1;
				}
				break;
			case 1: // Center
				seld = 1;
		}
		waitForNoInput();
	}

	time[i] = tim;
}

void bunkerMenu(uint8_t i){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);

	uint8_t bunk = bunkers; //Max amt will be 5 bunkers
	
	uint8_t seld = 0;
	while(!seld){
		LCD_move_cursor_to_col_row(2, 0);
		LCD_print_hex4(bunk);
		LCD_print_String(" BNKRS");
		LCD_print_String("<  --  >");
		switch(get_input()){
			case 0: // Left
				if(bunk >= 1){//Cannot go below 0
					bunk = bunk - 1;
				}
				break;
			case 2: // Right
				if(bunk <= 4){//cannot go above 5
					bunk = bunk + 1;
				}
				break;
			case 1: // Center
				seld = 1;
		}
		waitForNoInput();
	}
	bunkers = bunk;
}


/*
	@returns 0 to continue looping, 1 to stop loop
*/
uint8_t goMenu(){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	//Check if all commands are filled
	if(speed[0] == 0 || bunkers == 0){
		LCD_print_String("COMMAND");
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_String("NOT SET");
		get_input();
		return 0;
	}
	
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	LCD_print_String("RUN COM?");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("< NO YES");
	switch(get_input()){
		case 0://Illusion of choice
			return 0;
			break;
		case 1:
			return 0;
			break;
		case 2:
			return 1;
			break;
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
	speed[0] = 0;
	bunkers = 0;

	//Initialize variables used for UI
	int commIndex = 0;
	char commElements[3][8] = {" SPEED  ", "  BNKR  ", "   GO   "};
	char arrowUI[] = "<  --  >"; 
	
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
					commIndex = 2;
				}else{
					commIndex--;
				}
				break;
			case 2: // Right
				if(commIndex == 2){
					commIndex = 0;
				}else{
					commIndex ++;
				}
				break;
			case 1: // Center
				switch(commIndex){
					case 0:
						speedMenu(commands);
						break;
					case 1:
						bunkerMenu(commands);
						break;
					case 2:
						if (goMenu()){ // 0 is continue
							finUI = 1;
						}
				}
		}
	}
}


void run_motor_L_CCW(int cmd, int bias)
{
	int change = 0;
	if(l_counter < 10)
	{
		l_counter = l_counter + 1;
		change = 0;
	}
	else{
		change = bias;
		l_counter = 0;
	}
	pwm_counter_L = pwm_counter_L + 1;
	if(pwm_counter_L >= PWM_TOP){
		pwm_counter_L = 0;
	}
	if(pwm_counter_L < (duty_cycle*speed[cmd])+bias){
		PORTD |= (1<<5);
	}
	else{
		PORTD &= ~(1<<5);
	}
	_delay_us(50);
}

void run_motor_L_CW(int cmd, int bias)
{
	int change = 0;
	if(l_counter < 10)
	{
		l_counter = l_counter + 1;
		change = 0;
	}
	else{
		change = bias;
		l_counter = 0;
	}
	pwm_counter_L = pwm_counter_L + 1;
	if(pwm_counter_L >= PWM_TOP){
		pwm_counter_L = 0;
	}
	if(pwm_counter_L < (duty_cycle*speed[cmd])+change){
		PORTD |= (1<<6);
	}
	else{
		PORTD &= ~(1<<6);
	}
	_delay_us(50);
}

void run_motor_R_CCW(int cmd, int bias)
{
	int change = 0;
	if(r_counter < 10)
	{
		r_counter = r_counter + 1;
		change = 0;
	}
	else{
		change = bias;
		r_counter = 0;
	}
	pwm_counter_R = pwm_counter_R + 1;
	if(pwm_counter_R >= PWM_TOP){
		pwm_counter_R = 0;
	}
	if(pwm_counter_R < (duty_cycle*speed[cmd])+change){
		PORTD |= (1<<3);
	}
	else{
		PORTD &= ~(1<<3);
	}
	_delay_us(50);
}

void run_motor_R_CW(int cmd, int bias)
{
	int change = 0;
	if(r_counter < 10)
	{
		r_counter = r_counter + 1;
		change = 0;
	}
	else{
		change = bias;
		r_counter = 0;
	}
	pwm_counter_R = pwm_counter_R + 1;
	if(pwm_counter_R >= PWM_TOP){
		pwm_counter_R = 0;
	}
	if(pwm_counter_R < (duty_cycle*speed[cmd])+change){
		PORTB |= (1<<3);
	}
	else{
		PORTB &= ~(1<<3);
	}
	_delay_us(50);
}

/*
	Runs the motors in the command
	@param cmd integer defining the current command index
	@param movement integer defining the current movement direction (why?)
*/
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
			LCD_execute_command(CLEAR_DISPLAY);
			LCD_execute_command(MOVE_CURSOR_HOME);
			LCD_print_String("ERROR");
			break;
	}
}

void reset_pwm()
{
	pwm_counter_L = 0;
	pwm_counter_R = 0;
	r_counter = 10;
	l_counter = 10;
}

void display_command(int cmd){
	LCD_execute_command(CLEAR_DISPLAY);
		LCD_execute_command(MOVE_CURSOR_HOME);
		switch(speed[cmd]){
			case 1:
				LCD_print_String("S ");
				break;
			case 2:
				LCD_print_String("M ");
				break;
			case 3:
				LCD_print_String("F ");
				break;
		}
		LCD_print_hex4(bunkers);
		LCD_print_String("BNKR");
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_hex4(claimed_bunkers);
		LCD_print_String(" CAPPED");
		
}



void run_commands()
{
	display_command(cmd);
	reset_pwm();
	
}


int main(){

	init_buttons();
	init_LCD();
	init_motors();
	init_sensors();

	test_sensors();
	/*
	//Testing
	_delay_ms(2000);
	commands = 1;
	speed[0] = 1;
	movement[0] = 3;
	time[0] = 25000;
	run_commands();
	deact_motors();
	*/
	/*
	while(1){
		create_comm();
		run_commands();
		deact_motors();
	}
	*/
	
	return 0;
}