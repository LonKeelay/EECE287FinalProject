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

//Definitions for sensors
#define sen_LM (~(PINC) & (1<<0))
#define sen_LC (~(PINC) & (1<<1))
#define sen_CM (~(PINC) & (1<<2))
#define sen_RC (~(PINC) & (1<<3))
#define sen_RM (~(PINC) & (1<<4))

//Definitions for directions
#define Clockwise 1
#define CounterClockwise 2
#define Forward 3
#define Reverse 4

#define WALL 1
#define BUNKER 2

#define WhiteDelay 250

#define rightBias 8
#define leftBias 0
#define basePWM 40
#define clkMAX 255

//Bunker Variables
uint8_t bunkers;
uint8_t claimed_bunkers;
//Speed Variable
uint8_t speed;
//


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
	DDRD &= ~(1<<5);
    PORTD &= ~(1<<5);

	DDRD &= ~(1<<6);
    PORTD &= ~(1<<6);

	DDRD &= ~(1<<3);
    PORTD &= ~(1<<3);

	DDRB &= ~(1<<3);
    PORTB &= ~(1<<3);
}

void init_buzzer(){
    DDRB &= ~(1<<2);
    PORTB &= ~(1<<2);
}

void tone(int period){
    PORTB |= (1>>2);
    _delay_ms(1000);
    PORTB &= ~(1>>2);
    _delay_ms(1);
}

void deact_motors()
{
	PORTD &= ~(1<<5);
	PORTD &= ~(1<<6);
	PORTD &= ~(1<<3);
	PORTB &= ~(1<<3);	
}

void hard_brake(){
    PORTD |= (1<<5);
	PORTD |= (1<<6);
	PORTD |= (1<<3);
	PORTB |= (1<<3);
}

void init_LCD(){
	initialize_LCD_driver();
	LCD_execute_command(TURN_ON_DISPLAY);
	LCD_print_String("Group 42");
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


void create_comm(){
	uint8_t finUI = 0;
	// Erase residual commands
	speed = 0;
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
		LCD_print_String("BNKR");
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

//With 10 us time, a happy medium is probs 0x20 == 640 us
uint8_t analog_Reflecc(){
    DDRC |= (0x1F);
    PORTC |= (0x1F);
    _delay_us(10);
    PORTC &= ~(0x1F);
    
    //Turn on sensors
    DDRC &= ~(0x1F);
    
    uint8_t delay = 0;
    while((PINC & (0x1F)) > 1){
        delay++;
        _delay_us(5);
    }
    return delay;
}

uint8_t digital_Reflecc(){
    //Pulse IR LEDs
    DDRC |= (0x1F);
    PORTC |= (0x1F);
    _delay_us(10);
    PORTC &= ~(0x1F);

    DDRC &= ~(0x1F);
    _delay_us(WhiteDelay);
    return (PINC & 0x1F);

}

void push(){
    deact_motors();
    //Funky PWM, I guess go for 1ms?
    for(int i = 0; i < clkMAX; i++){
        int lef = ((basePWM + leftBias) - i > 0);
        int rig = ((basePWM + rightBias) - i > 0);
        motor_Driver(Forward, lef, rig);
        _delay_us(1);
        motor_Driver(Forward, 0, 0);
    }
}

void spin(int direc){
    deact_motors();
    //Funky PWM, I guess go for 1ms?
    for(int i = 0; i < clkMAX; i++){
        int lef = ((basePWM + leftBias) - i > 0);
        int rig = ((basePWM + rightBias) - i > 0);
        motor_Driver(direc, lef, rig);
        _delay_us(1);
        motor_Driver(direc, 0, 0);
    }
}

void stepBack(int direc){
    deact_motors();
    for(int i = 0; i < clkMAX*5; i++){
        int lef = ((basePWM + leftBias) - i%clkMAX > 0);
        int rig = ((basePWM + rightBias) - i%clkMAX > 0);
        switch (direc){
            case Clockwise:
            motor_Driver(Clockwise, 0, rig);
            break;

            case CounterClockwise:
            motor_Driver(CounterClockwise, lef, 0);
            break;
        }
        _delay_us(1);
        motor_Driver(direc, 0, 0);
    }
}

//Scooches the robot forward
void scooch(){
    for(int i = 0; i < 10; i++){
        push();
    }
}

int decidSpin(){
    while(1){
        deact_motors();
        int reflecc = (int)digital_Reflecc();
        LCD_execute_command(MOVE_CURSOR_HOME);
        LCD_print_hex8(reflecc);
        switch (reflecc){
            case 0b1:
            case 0b11:
            case 0b10:
            hard_brake();
            _delay_ms(300);
            deact_motors();
            while(!(reflecc & (3<<3)) && (reflecc & 3)){
                spin(CounterClockwise);
                reflecc = digital_Reflecc();
                deact_motors();
            }
            if (reflecc == 0){
                //return BUNKER;
            }
            break;

            case 0b10000:
            case 0b11000:
            case 0b01000:
            hard_brake();
            _delay_ms(300);
            deact_motors();
            while((reflecc & (3<<3)) && !(reflecc & 3)){
                spin(Clockwise);
                reflecc = digital_Reflecc();
                deact_motors();
            }
            if (reflecc == 0){
                //return BUNKER;
            }
            break;

            case 0b110: // 6
            case 0b1100: // C
            case 0b01110:
            case 0b01111:
            case 0b00111:
            case 0b11100:
            case 0b11110:
            case 0b00100:
            hard_brake();
            _delay_ms(300);
            deact_motors();
            scooch();
            hard_brake();
            reflecc = digital_Reflecc();
            if(reflecc != 0b11111 && reflecc != 0b11011 && reflecc != 0b10001){ // Make sure robot has not hit wall or corner
                return BUNKER;
            }else{
                return WALL;
            }
            break;

            case 0b10001:
            case 0b11011:
            case 0b11111:
            hard_brake();
            deact_motors();
            return WALL;
            break;

            default:
            push();
            break;
        }
    }
    return 0;

}

void turn_around(){
    deact_motors();
    while(!digital_Reflecc()){
        spin(Reverse);
    }
    for(int i = 0; i < 200; i++){
        spin(Clockwise);
    }
}

void turn_around_bunk(){
    deact_motors();
    while(!digital_Reflecc()){
        spin(Reverse);
    }
    for(int i = 0; i < 255; i++){
        spin(Clockwise);
    }
    hard_brake();
    _delay_ms(10);
    deact_motors();
}

void celebrate_cap(){//Insert buzzer noise, LCD message, spinning, etc here
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	LCD_print_String("BUNKER");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("CAPPED!");
}

void celebrate_win(){//Insert buzzer noise, LCD message, spinning, etc here
	LCD_execute_command(CLEAR_DISPLAY);
	LCD_execute_command(MOVE_CURSOR_HOME);
	LCD_print_String("ALL BNKR");
	LCD_move_cursor_to_col_row(0,1);
	LCD_print_String("CAPPED!");
}

void reset_var(){//Reset everything before next run-thru
	claimed_bunkers = 0;
	bunkers = 0;
	speed  = 0;
}

int main(){
    init_LCD();
    init_motors();
	while(1)
	{
		reset_var();
		deact_motors();
		get_input();
		//create_comm();
        bunkers = 1;
		LCD_execute_command(CLEAR_DISPLAY);
		while (claimed_bunkers < bunkers)
		{
			LCD_execute_command(MOVE_CURSOR_HOME);
			display_command();
			switch (decidSpin()){
				case BUNKER:
					LCD_print_String("Bunker");
					claimed_bunkers =  claimed_bunkers + 1;
                    turn_around_bunk();
					celebrate_cap();
					get_input();
					break;
				case WALL:
					LCD_print_String("Wall  ");
					turn_around();
					break;
				default:
					LCD_print_String("Error ");
					break;
			}
		}
		_delay_ms(500);//Prevent celebrate_win from getting skipped
		celebrate_win();
		get_input();
	}

    /*
    LCD_execute_command(CLEAR_DISPLAY);
    LCD_execute_command(MOVE_CURSOR_HOME);
    while(1){
        LCD_print_hex8(analog_Reflecc());
        LCD_move_cursor_to_col_row(0,0);
    }
    */
    return 0;

}