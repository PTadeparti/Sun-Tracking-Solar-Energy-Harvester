/*
 * SolarTracking.c
 *
 * Created: 2020-08-02 1:58:39 PM
 * Author : Prashant Tadeparti
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>


#define MOTOR_LEFT 0x02;
#define MOTOR_RIGHT 0x04;
#define MOTOR_STOP 0x00;

float L_PR_Voltage;
float R_PR_Voltage;
float dutyCycle = 9;

// Set all Port B pins to output
void SET_GPIOB (void) 
{
	DDRB |= 0xFF;
	PORTB |= 0xFF;
	PINB |= 0x00;	
}


void Turn_Motor(int dir) 
{
	switch (dir) {
		case 0: 
			PINB = MOTOR_STOP;
			PORTB = MOTOR_STOP;
			break;
		case 1:
			PINB = MOTOR_LEFT;
			PORTB = MOTOR_LEFT;
			break;
		case 2:
			PINB = MOTOR_RIGHT;
			PORTB = MOTOR_RIGHT;
			break;
		default:
			PINB = MOTOR_STOP;
			PORTB = MOTOR_STOP;
			break;
	}
	
}

void startConversion(void) 
{
	ADCSRA |= (1 << ADSC);
}

void ADCinit(void) 
{
	ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
	ADCSRA |= (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	sei();
	startConversion();
}

ISR(ADC_vect) {	

	
	if(ADMUX == 0x45) {
		_delay_ms(10);
		L_PR_Voltage = (float)ADC*(float)5/(float)1024;
		ADMUX = 0x44;		
	} else if (ADMUX == 0x44) {
		_delay_ms(10);
		R_PR_Voltage = (float)ADC*(float)5/(float)1024;
		ADMUX = 0x45;
	}	
	
	startConversion(); 
}

void PWMinit(void) {
	DDRD = 1 << PORTD6;
	TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
	TIMSK0 |= (1 << TOIE0);
	OCR0A = (dutyCycle/100.0)*255;
	sei();
	ICR1 = 5000;
	TCCR0B |= (1 << CS01) | (1 << CS00);
}

ISR(TIMER0_OVF_vect) {
	OCR0A = (dutyCycle/100.0)*255;	
}

int main(void)
{
	SET_GPIOB();
	ADCinit();	
	PWMinit();
	Turn_Motor(0);
	
	if (dutyCycle < 3 || dutyCycle > 12) {
		dutyCycle = 6;
	}

    while (1) 
    {		
		if(L_PR_Voltage < R_PR_Voltage*0.9) 
		{			
			Turn_Motor(1);			
			_delay_ms(2);
			
			if(dutyCycle < 12) 
			{
			dutyCycle += 1;
			}
		} 
		else if (R_PR_Voltage < L_PR_Voltage*0.9) 
		{
			Turn_Motor(2);
			_delay_ms(2);
				
			if(dutyCycle > 3) {
				dutyCycle -= 1;
			}	
		} 
		else 
		{
			Turn_Motor(0);
			_delay_ms(100);
		}	
	}
}

