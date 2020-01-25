
#include <avr/io.h>
#include <util/delay.h>

//#define baud	51
/* Replace with your library code */
//int myfunc(void)
//{
//	return 0;
//}



/* Main - a simple test program*/
//int main( void )
//{

//}

/* Initialize UART */
void UART_Initialize( unsigned int baudrate )
{
	/* Set the baud rate */
	//UBRR0H = (unsigned char) (baudrate>>8);
	//UBRR0L = (unsigned char) (baudrate);
	//UBRR0H = 0;
	//UBRR0L = 3;
	/* Enable UART receiver and transmitter */
	
	/*UCSR0B |= (( 1 << RXEN0 ) | ( 1 << TXEN0 ));  //Enabling Transmit and Receive Pin
	
	UCSR0C |= ((1<<UCSZ01)|(1<<UCSZ00));							    //Setting to 8 bit Mode
	//UCSR0C &= ~((1<<UMSEL01)|(1<<UMSEL00)|(1<<UCSZ02));			    //Asynchronous USART
	UCSR0B&=~(1<<UCSZ02);
	UCSR0C &= ~((1<<UPM00)|(1<<UPM01)); //
	//UCSR0C |= (1<<USBS0);
	UCSR0C &= ~(1<<USBS0);
	UCSR0A  |= (1<< U2X0); 
	//UCSR0C = (1<<USBS0)|(3<<UCSZ00);*/
	
	
	UBRR0H = (unsigned char) (baudrate>>8);
	UBRR0L = (unsigned char) (baudrate);			                    
                                       				// Receiver Enabled
	UCSR0B |= (1 << RXCIE0);								// Receive Interrupt Enabled
	UCSR0B |= (( 1 << RXEN0 ) | ( 1 << TXEN0 ));  					//Enabling Transmit and Receive Pin
	
	UCSR0C |= ((1<<UCSZ01)|(1<<UCSZ00));							    //Setting to 8 bit Mode
	UCSR0B &=~(1<<UCSZ02);
	UCSR0C &= ~((1<<UPM00)|(1<<UPM01));
	UCSR0C |= (1<<USBS0);
	UCSR0A  |= (1<< U2X0); 
}



void USART0_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !(UCSR0A & (1<<UDRE0)) );
	//while(!(UCSR0A & (1<<TXC0)));
	
	/* Start transmission */
	UDR0 = data;    //
}
void UART_Transmit_string(char string[])
{
	int i=0;
	while ( string[i] !='\0')
	USART0_Transmit(string[i++]);
}

/* Read and write functions */
unsigned char USART0_Receive( void )
{
	/* Wait for incoming data */
	//UART_Transmit_string("\r\n Receiving data");
	while ( !(UCSR0A & (1<<RXC0)) );
	
	/* Return the data */
	return UDR0;    //Data Storage Buffer
}