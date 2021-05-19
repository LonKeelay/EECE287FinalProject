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

#define Clockwise 1
#define CounterClockwise 2
#define Forward 3
#define Reverse 4

#define WALL 1
#define BUNKER 2

#define WhiteDelay 180

#define rightBias 0
#define leftBias 1
#define basePWM 40
#define clkMAX 255

//Global Variables for Commands
uint8_t bunkers;
uint8_t claimed_bunkers;
uint8_t speed;
uint8_t pwm_speed;


void intialize_robot(){
	//Buttons
	DDRB &= ~(1<<1);
	PORTB |= (1<<1);
	DDRB &= ~(1<<4);
	PORTB |= (1<<4);
	DDRB &= ~(1<<5);
	PORTB |= (1<<5);
	//Motors
	DDRD &= ~(1<<5);
    PORTD &= ~(1<<5);
	DDRD &= ~(1<<6);
    PORTD &= ~(1<<6);
	DDRD &= ~(1<<3);
    PORTD &= ~(1<<3);
	DDRB &= ~(1<<3);
    PORTB &= ~(1<<3);
	//LCD
	initialize_LCD_driver();
	LCD_execute_command(TURN_ON_DISPLAY);
	LCD_print_String("Group 42");
}

void deact_motors(){
	PORTD &= ~(1<<5);
	PORTD &= ~(1<<6);
	PORTD &= ~(1<<3);
	PORTB &= ~(1<<3);
}

void stop_bot(uint16_t ms){
	//Hard Break
	PORTD |= (1<<5);
	PORTD |= (1<<6);
	PORTD |= (1<<3);
	PORTB |= (1<<3);
	_delay_ms(ms);
	deact_motors();
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

void speedMenu(){
	waitForNoInput();
	uint8_t chosen = 0;
	int menuIndex;
	if (speed == 0){menuIndex = 0;}
	else{menuIndex = speed - 1;}
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
	speed = menuIndex + 1; //Index is offset by 1 due to 0 meaning no command
	pwm_speed = 3*menuIndex; //Affects PWM speed
}

void bunkerMenu(){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	uint8_t bunk = bunkers; //Max amt will be 5 bunkers
	
	uint8_t seld = 0;
	while(!seld){
		LCD_move_cursor_to_col_row(0, 0);
		LCD_print_hex4(bunk+1);
		LCD_print_String(" BNKRS");
		LCD_move_cursor_to_col_row(0, 1);
		LCD_print_String("<  --  >");
		switch(get_input()){
			case 0: // Left
				if(bunk >= 1){//Cannot go below 0(1)
					bunk = bunk - 1;
				}
				else{
					bunk = 4;
				}
				break;
			case 2: // Right
				if(bunk <= 3){//Cannot go above 4(5)
					bunk = bunk + 1;
				}
				else{
					bunk = 0;
				}
				break;
			case 1: // Center
				seld = 1;
		}
		waitForNoInput();
	}
	bunkers = bunk + 1; //Index is offset by 1 due to 0 meaning no command
}

uint8_t goMenu(){
	waitForNoInput();
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	//Check if all commands are filled
	if(speed == 0 || bunkers == 0){
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

void create_command(){
	uint8_t finUI = 0;
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
						speedMenu();
						break;
					case 1:
						bunkerMenu();
						break;
					case 2:
						if (goMenu()){ // 0 is continue in menus
							finUI = 1;
						}
				}
		}
	}
}

void display_command(){
	LCD_execute_command(CLEAR_DISPLAY);
		LCD_execute_command(MOVE_CURSOR_HOME);
		switch(speed){
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
		LCD_print_String("BNKRS");
		LCD_move_cursor_to_col_row(0,1);
		LCD_print_hex4(claimed_bunkers);
		LCD_print_String(" CAPPED");
		
}


void motor_L_CW(int bool){
    if(bool){
        PORTD |= (1<<5);
    }else{
        PORTD &= ~(1<<5);
    }
}

void motor_L_CCW(int bool){
    if(bool){
        PORTD |= (1<<6);
    }else{
        PORTD &= ~(1<<6);
    }
}

void motor_R_CCW(int bool){
    if(bool){
        PORTD |= (1<<3);
    }else{
        PORTD &= ~(1<<3);
    }
}

void motor_R_CW(int bool){
    if(bool){
        PORTB |= (1<<3);
    }else{
        PORTB &= ~(1<<3);
    }

}

void motor_Fwd(int boolL, int boolR){
    motor_L_CCW(boolL);
    motor_R_CW(boolR);
}

void motor_Rev(int boolL, int boolR){
    motor_L_CW(boolL);
    motor_R_CCW(boolR);
}

void motor_CW(int boolL, int boolR){
    motor_L_CCW(boolL);
    motor_R_CCW(boolR);
}

void motor_CCW(int boolL, int boolR){
    motor_L_CW(boolL);
    motor_R_CW(boolR);
}

void motor_Driver(int mvmt, int boolL, int boolR){
    switch(mvmt){
        case Clockwise:
            motor_CW(boolL, boolR);
            break;
        case CounterClockwise:
            motor_CCW(boolL, boolR);
            break;
        case Forward:
            motor_Fwd(boolL, boolR);
            break;
        case Reverse:
            motor_Rev(boolL, boolR);
            break;
    }
}

uint8_t digital_reflection(){
    //Pulse IR LEDs
    DDRC |= (0x1F);
    PORTC |= (0x1F);
    _delay_us(10);
    PORTC &= ~(0x1F);
    DDRC &= ~(0x1F);
    _delay_us(WhiteDelay);
    return (PINC & 0x1F);

}

void push(uint8_t direc){
    deact_motors();
    for(int i = 0; i < clkMAX; i++){
        int lef = ((basePWM + leftBias + pwm_speed) - i > 0);
        int rig = ((basePWM + rightBias + pwm_speed) - i > 0);
        motor_Driver(direc, lef, rig);
        _delay_us(1);
        motor_Driver(direc, 0, 0);
    }
}

void spin(int direc){
    deact_motors();
    for(int i = 0; i < clkMAX; i++){
        int lef = ((basePWM + leftBias) - i > 0);
        int rig = ((basePWM + rightBias) - i > 0);
        motor_Driver(direc, lef, rig);
        _delay_us(1);
        motor_Driver(direc, 0, 0);
    }
}

//Slightly moves the robot in the chosen direction
void slight_movement(uint8_t dir){
    for(int i = 0; i < 10; i++){
        switch (dir)
		{
		case Clockwise:
			spin(Clockwise);
			break;
		case CounterClockwise:
			spin(CounterClockwise);
			break;
		case Forward:
			push(Forward);
			break;
		case Reverse:
			push(Reverse);
			break;
		default:
			break;
		}
    }
}

int decidSpin(){
    while(1){
        deact_motors();
        int reflection = (int)digital_reflection();
        LCD_execute_command(MOVE_CURSOR_HOME);
        switch (reflection){
			//Left Side Section Detection
            case 0b00001:
            case 0b00011:
            case 0b00010:
            stop_bot(300);
            while(!(reflection & (3<<3)) && (reflection & 3)){
                slight_movement(CounterClockwise);
                reflection = digital_reflection();
                deact_motors();
            }
            slight_movement(Forward);
			stop_bot(10);
            break;

			//Right Side Sensors Detection
            case 0b10000:
            case 0b11000:
            case 0b01000:
            stop_bot(300);
            while((reflection & (3<<3)) && !(reflection & 3)){
                slight_movement(Clockwise);
                reflection = digital_reflection();
                deact_motors();
            }
            slight_movement(Forward);
			stop_bot(10);
            break;

			//Front Sensors Detection
            case 0b00110:
            case 0b01100:
            case 0b01110:
            case 0b01111:
            case 0b00111:
            case 0b11100:
            case 0b11110:
            case 0b00100:
			case 0b01010:
            stop_bot(300);
            slight_movement(Forward);
            stop_bot(10);
			reflection = digital_reflection();
			//Ensures robot has not hit a wall or corner
            if(reflection != 0b11111 && reflection != 0b11011 && reflection != 0b10001 && reflection != 0b01011 && reflection != 0b11010 && reflection != 0b01111 && reflection != 0b11110){
                return BUNKER;
            }else{
                return WALL;
            }
            break;

			//All Sensors Detection
            case 0b10001:
            case 0b11011:
            case 0b11111:
            stop_bot(10);
            return WALL;
            break;

            default:
            push(Forward);
            break;
        }
    }
    return 0;

}

void turn_around(uint8_t angle){
    deact_motors();
    while(!digital_reflection()){
        spin(Reverse);
    }
    for(uint8_t i = 0; i < angle; i++){
        spin(Clockwise);
    }
	stop_bot(10);
}

void celebrate_cap(){
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	LCD_print_String("BUNKER");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("CAPPED!");
}

void celebrate_win(){
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	LCD_print_String("ALL BNKR");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("CAPPED!");
}

void reset_var(){
	claimed_bunkers = 0;
	bunkers = 0;
	speed  = 0;
}

int main(){
    intialize_robot();
	get_input();
	while(1)
	{
		reset_var();
		deact_motors();
		create_command();
		LCD_execute_command(CLEAR_DISPLAY);
		while (claimed_bunkers < bunkers)
		{
			LCD_execute_command(MOVE_CURSOR_HOME);
			display_command();
			switch (decidSpin()){
				case BUNKER:
					claimed_bunkers =  claimed_bunkers + 1;
                    turn_around(255);
					celebrate_cap();
					_delay_ms(2000); //Pause to remove found bunker
					break;
				case WALL:
					LCD_print_String(" Wall");
					turn_around(240);
					break;
				default:
					LCD_print_String(" Error");
					break;
			}
		}
		_delay_ms(500);//Prevent celebrate_win from getting skipped by button press
		celebrate_win();
		get_input();
	}
    return 0;
}