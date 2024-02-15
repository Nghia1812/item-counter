#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lcd.h"


#define USART_BAUDRATE 9600 //Toc do Baudrate
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) //BAUD_PRESCALE dung ghi vao thanh ghi UBRR (16 bit)
char str[20] = {}; // Chuoi chua so luong san pham
volatile unsigned long count = 0; //So luong san pham
volatile bool available = false; //Cho biet da nhan duoc du lieu hay chua
volatile char a = '\0'; // ki tu nhan duoc
volatile bool k = false; // bien cho biet da co san pham di den hay chua (xay ra ngay hay chua)

void UART_init()
{
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE); //Cho phep nhan du lieu,truyen du lieu, cho phep ngat khi nhan du lieu hoan tat
	UCSRC = (1 << UCSZ0) | (1 << UCSZ1); //8 bit data
	UBRRH = (unsigned char)(BAUD_PRESCALE >> 8); //Lay 8 bit cao cua Baurate (Dung khi BAUD>255)
	UBRRL = (unsigned char)BAUD_PRESCALE; //Lay 8 bit thap (unsigned char: 8bit)
}


void UART_TxChar(char ch)
{
	while (!(UCSRA & (1 << UDRE))); //Cho cho den khi UDRE len 1 moi gui dulieu
	UDR = ch; // gui ki tu ch
}


void UART_SendString(char *str)
{
	unsigned char j = 0;
	while (str[j] != 0)
	{
	UART_TxChar(str[j]); //Gui tung ki tu trong chuoi cho den khi gap khitu ket thuc chuoi
	j++;
	}
}


int main(void)
{
	/* Replace with your application code */
	UART_init(); //Khoi dong UART
	LCD_Init(); // Khoi dong LCD
	PORTD |=(1<<2); // Dung dien tro keo len o chan INT0 (Chan ngat ngoai noi voi OUT cua cam bien)
	MCUCR |= (1<<ISC01); //Kich ngat ngoai canh xuong
	GICR |= (1<<INT0); //Khoi dong ngat ngoai 0
	sei(); //Cho phep ngat toan cuc
	DDRA |= (1<<7) | (1<<6); //2 chan LED1 LED2 la dau ra
	LCD_Clear(); //Xoa toan bo LCD
	while (1)
	{
		while(available) //Khi co du lieu gui den
		{
			if (a=='0')
			{
				PORTA &= ~(1<<6); // Tat LED
			}
			if (a == '1')
			{
				PORTA |= (1<<6); //Bat lED
			}
			if (a == '2') //Resset so san pham
			{
				count = 0;
				LCD_Clear();
				LCD_String_xy(0,0," TONG SAN PHAM");
				sprintf(str, "%lu", count); // Chuyen so nguyen sang chuoi str
				int column = (16-strlen(str))/2;
				LCD_String_xy(1,column,str); // In ra LCD o dong 1 cot column
				UART_SendString(str);
			}
			available = false; //Cho available = false de ket thuc qua trinh
		}
		if ((PIND & (1<<2)) == 0) // Khi co san pham truoc cam bien thi bat den
		{
			PORTA |= (1<<7);
		}
		else
		{
			PORTA &= ~(1<<7);
		}
		if(k==true)
		{
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

