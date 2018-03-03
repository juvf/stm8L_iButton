#include "Serial.h"
#include "Checksum.h"
#include "main.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>


#define F_CPU 2000000.0
#define BaudRate 19200.0
#define Divider ((uint16_t)(F_CPU/BaudRate + 0.5))
  


uint8_t buffer42[MAX_SIZE];
char tempString[32];
extern uint16_t countTim2[5];

Serial serial;
Serial::Serial()
{
	countTx = 0;
	countBuf = 0;
	isTransmoin = false;
	count42 = 0;
	isResive = false;
}

void Serial::init()
{
	/*Set baud rate */
/*      UART1_Init(19200,
                 UART1_WORDLENGTH_8D,
                 UART1_STOPBITS_1,
                 UART1_PARITY_NO,
                 UART1_SYNCMODE_CLOCK_DISABLE,
                 UART1_MODE_TXRX_ENABLE); */
	//UART1_CR1 = 0; //закоментировал, т.к. после ресета CR1 равен 0
  	CLK->PCKENR1 |= CLK_PCKENR1_USART1;
	GPIO_Init(GPIOC, GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast ); // PC5 TX USART1 transmit 
	GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_No_IT );

  /* USART configuration */
 /* USART_Init(USART1, 19200,
             USART_WordLength_8b,
             USART_StopBits_1,
             USART_Parity_No,
             USART_Mode_Tx);*/
  
   // USART1->BRR2 = 8;
  	//USART1->BRR1 = 6;
	USART1->BRR2 = (Divider&0x0F) | ((Divider>>8)&0xF0);
  	USART1->BRR1 = (Divider>>4);
	USART1->CR2 = USART_CR2_REN | USART_CR2_TEN | USART_CR2_RIEN | USART_CR2_TCIEN;
	USART1->SR = 0;
  
/*    UART1->BRR2 = (Divider&0x0F) | ((Divider>>8)&0xF0);
  	UART1->BRR1 = (Divider>>4);
    UART1->CR2 = UART1_CR2_TCIEN | UART1_CR2_REN
	  | UART1_CR2_TEN | UART1_CR2_RIEN;//UART1_ITConfig(UART1_IT_TC, ENABLE);
    UART1->SR = 0;*/

}

void Serial::print(const char *text, bool printLn)
{
	uint16_t len = strlen(text);
	if(add2Buf(text, len))
	{
		if(printLn)
	  		add2Buf("\n\r", 2);
	  startTransmit();
	}
}

bool Serial::add2Buf(const char *array, int len)
{
  if(len > MAX_SIZE)
    return false;
  uint8_t ostatok = MAX_SIZE - countBuf;
	if(len <= ostatok)
		memcpy(buffer + countBuf, array, len);
	else
	{
		memcpy(&buffer[countBuf], array, ostatok);
		array += ostatok;
		ostatok = len - ostatok;
		memcpy(buffer, array, ostatok);
	}
	countBuf += len;
    countBuf &= 0x3f;
	return true;
}

void Serial::sentBytes(const uint8_t *array, uint8_t len)
{
	if(add2Buf((const char*)array, len))
	  startTransmit();
}

void Serial::println(unsigned int val, int s)
{
             itoa(val, tempString, s);
			this->print(tempString, true);
}

void Serial::startTransmit()
{
	disableInterrupts();
	if(!isTransmoin)
	{
		USART1->DR = buffer[countTx++];
                if(countBuf > (MAX_SIZE - 1))
                    countBuf = 0;
//		UCSR0B |= (1<<TXCIE0);//USART1->CR1 |= USART_CR1_TCIE;
		isTransmoin = true;
//		//uint8_t avr = UCSR0B;
//		//if(avr & (1<<TXCIE0))
	}
	enableInterrupts();
}

void Serial::print(int val, int s)
{
    itoa(val, tempString, s);
    this->print(tempString, false);
}

void Serial::flush()
{
	while(isTransmoin)
		;
}

void Serial::setDebugMode(uint8_t mode)
{
	debugMode = mode;
}

//volatile uint8_t tempBuf[20];
//volatile uint8_t yy = 0;

//#pragma optimize=none
void Serial::reciveByte(uint8_t byte)
{
 //tempBuf[yy++] = byte;
//  if(yy == 20)
//	yy = 0;
	switch(count42)
	{
		case 0:
			if(byte == 0x02)
			{
				buffer42[0] = byte;
				++count42;
			}
			break;
		case 1:
			if(byte == 0x19)
			{
				buffer42[1] = byte;
				++count42;
			}
			else
				count42 = 0;
			break;
		case 2: //длинна пакета
			if(byte < MAX_SIZE)
			{
				buffer42[2] = byte;
				++count42;
			}
			else
				count42 = 0;
			break;
		default:
			buffer42[count42++] = byte;
			if(count42 >= buffer42[2])
			{
				if(Checksum::crc16(buffer42, count42) == 0)
				{
					isResive = true; //приняли весь пакет
sendReplay();
this->startTransmit();
					//timeToSleepUart = 2;
					//USART1->CR1 &= ~USART_CR1_RXNEIE;
				}
				else
				  isResive = false;
				count42 = 0;
			}
			break;
	}
}

bool Serial::isGetCommand() const
{
	return isResive;
}

void Serial::clearIsResive()
{
	isResive = false;
}

void handlerUsartTxD()
{
  USART1->SR &= ~USART_FLAG_TC;
	if(serial.countTx != serial.countBuf)
        {
		USART1->DR = serial.buffer[serial.countTx];
                if( ++serial.countTx >= MAX_SIZE )
                  serial.countTx = 0;
        }
	else
		serial.isTransmoin = false;
	if(timeToSleepUart < 10)
		timeToSleepUart = 10;
}

void handlerUsartRxD()
{ 
  if(USART1->SR & USART_FLAG_OR)
	serial.reciveByte(USART1->DR);
  if(USART1->SR & USART_FLAG_RXNE)
	serial.reciveByte(USART1->DR); //при чтении флаг прерывания сам почиститься
  
  timeToSleepUart = 400;
	
}

void isrSerial()
{
  	timeToSleepUart = 400;
#ifndef USE_HALT	
	GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_No_IT );
	USART1->CR2 = USART_CR2_TCIEN | USART_CR2_REN  | USART_CR2_TEN | USART_CR2_RIEN;
#endif
}

