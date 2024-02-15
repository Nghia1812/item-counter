#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lcd.h"


#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) //BAUD_PRESCALE - UBRR (16 bit)
char str[20] = {}; // product count str
volatile unsigned long count = 0; 
volatile bool available = false; //Data rcv check
volatile char a = '\0'; // Rcv data
volatile bool k = false; // Check whether there are items - EXTI

void UART_init()
{
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE); 
	UCSRC = (1 << UCSZ0) | (1 << UCSZ1); //8 bit data
	UBRRH = (unsigned char)(BAUD_PRESCALE >> 8); //BAUD_PRESCALE (BAUD>255)
	UBRRL = (unsigned char)BAUD_PRESCALE; //LSB
}


void UART_TxChar(char ch)
{
	while (!(UCSRA & (1 << UDRE))); 
	UDR = ch; // send ch
}


void UART_SendString(char *str)
{
	unsigned char j = 0;
	while (str[j] != 0)
	{
	UART_TxChar(str[j]);
	j++;
	}
}


int main(void)
{
	UART_init();
	LCD_Init();
	PORTD |=(1<<2); // Pull-up res (INT0 connected with IR)
	MCUCR |= (1<<ISC01); //Falling edge
	GICR |= (1<<INT0); //Interrupt activate
	sei(); 
	DDRA |= (1<<7) | (1<<6); //2 LEDs
	LCD_Clear();
	while (1)
	{
		while(available) //Data avail
		{
			if (a=='0')
			{
				PORTA &= ~(1<<6); // LED1 off
			}
			if (a == '1')
			{
				PORTA |= (1<<6); //LED1 on
			}
			if (a == '2') //Reset counter
			{
				count = 0;
				LCD_Clear();
				LCD_String_xy(0,0," TONG SAN PHAM");
				sprintf(str, "%lu", count); 
				int column = (16-strlen(str))/2;
				LCD_String_xy(1,column,str);
				UART_SendString(str);
			}
			available = false;
		}
		if ((PIND & (1<<2)) == 0) // LED2 on per item counted
		{
			PORTA |= (1<<7);
		}
		else
		{
			PORTA &= ~(1<<7);
		}
		if(k==true)
		{*
			sprintf(str, "%lu", count);
			UART_SendString(str);
			LCD_Clear();
			LCD_String_xy(0,0," TONG SAN PHAM");
			int column = (16-strlen(str))/2;
			LCD_String_xy(1,column,str);
			k = false;
		}
	}
}

ISR(INT0_vect)
{
	count++;
	k=true;
}

ISR(USART_RXC_vect)
{
	available = true;
	a = UDR;
}

