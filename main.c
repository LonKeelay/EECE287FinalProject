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
    PORTD &= ~(1<<3);
}

void init_buzzer(){
    DDRB |= (1>>2);
}

void tone(int period){
    PORTB |= (1>>2);
    _delay_ms(1);
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
            _delay_ms(100);
            deact_motors();
            while(!(reflecc & (3<<3)) && (reflecc & 3)){
                stepBack(CounterClockwise);
                reflecc = digital_Reflecc();
                deact_motors();
            }
            break;

            case 0b10000:
            case 0b11000:
            case 0b01000:
            hard_brake();
            _delay_ms(100);
            deact_motors();
            while((reflecc & (3<<3)) && !(reflecc & 3)){
                stepBack(Clockwise);
                reflecc = digital_Reflecc();
                deact_motors();
            }
            break;

            case 0b110: // 6
            case 0b1100: // C
            case 0b01110:
            case 0b01111:
            case 0b00111:
            case 0b11100:
            case 0b11110:
            hard_brake();
            _delay_ms(100);
            deact_motors();
            scooch();
            hard_brake();
            reflecc = digital_Reflecc();
            if(reflecc != 0b11111 && (reflecc & (1<<2)) && !((reflecc & (1<<4))>>4 && (reflecc & 1))){ // Make sure robot has not hit wall or corner
                return BUNKER;
            }
            break;

            case 0b10001:
            case 0b11011:
            case 0b11111:
            //case 0b01111:
            //case 0b00111:
            //case 0b11100:
            //case 0b11110:
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

int main(){
    init_LCD();
    init_motors();
    get_input();
    LCD_execute_command(CLEAR_DISPLAY);
    while (1)
    {
        LCD_execute_command(MOVE_CURSOR_HOME);
        switch (decidSpin()){
            case BUNKER:
                LCD_print_String("Bunker");
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