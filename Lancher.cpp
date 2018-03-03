/*
 * Lancher.cpp
 *
 *  Created on: 06 ����� 2017 �.
 *      Author: juvf
 */

#include "Lancher.h"
#include "Serial.h"
#include "parser42.h"
#include "rfClient.h"
#include "varInEeprom.h"
#include "main.h"
#include "utils.h"	 


//#define LOW_POWER
#define DREBEZG	8
#define DEV_IMPL	1000 //���-�� ���������, ����� �������� ���� ���������
#define PIN_IMPL1 GPIO_Pin_3 //������� ��������� 1
#define PIN_IMPL2 GPIO_Pin_1 //������� ��������� 2
#define PIN_IMPL3 GPIO_Pin_2 //������� ��������� 3
#define PIN_IMPL4 GPIO_Pin_0 //������� ��������� 4
#define PORT_IMPL	GPIOB

extern uint32_t msDelay;
extern uint16_t timeOpros;
extern uint8_t ver;

#define pinReset(pin) (PORTB &= ~(1<<pin))
#define pinSet(pin) (PORTB |= (1<<pin))

//0 - ������ ������������ ��� ����� 1
//1 - ������ ������������ ��� ����� 1
//4 - ������ ��� ������ 42 ��������� �� UART-�
uint16_t countTim2[5] = { 0, 0, 0, 0, 0 };
uint8_t gpioJ[4] = { 1, 1, 1, 1 };
uint8_t writeIml[4] = { 0 };
uint8_t IMPL_PIN[4] = { PIN_IMPL1, PIN_IMPL2, PIN_IMPL3, PIN_IMPL4 };
bool wasCheckedImpl = false;
bool wasAlarm = false; //������� ����, ��� ���������� ����� �� (��������, ������� ����� � �.�.)

bool isAlarm = true;  //���� �������� ���������, �� true

Lancher lancher;

Lancher::Lancher()
{
}

Lancher::~Lancher()
{
}

void Lancher::up(uint8_t port)
{
	if(ver == 0x12)  //���������� ����� ��� ������ rp1701
	{
		if((port == 1) || (port == 2))
			port ^= 3;
	}
	switch(port)
	{
		case 1: //������� 3
			//releOn();
			//GPIO_Init(PORT_IMPL, (GPIO_Pin_TypeDef)IMPL_PIN[port], GPIO_Mode_In_FL_No_IT );
		case 0: //������� 1
		case 2: //������� 2
		case 3: //������� 4
			//PCMSK0 &= ~(1<<PCINT0); //������ ����������
			if(countTim2[port] == 0)
				startTimer(port);
			break;
		case 4: //������ �����
			break;
	}
}

void Lancher::startTimer(uint8_t port)
{
	countTim2[port] = DREBEZG;
}

#pragma optimize=none
void Lancher::init()
{
	//EXTI->CONF1 |= EXTI_CONF1_PBLIS;
	EXTI->CR1 = 0xff; //EXTI_Trigger_Rising_Falling;
	GPIO_Init(PORT_IMPL,
			(GPIO_Pin_TypeDef)(PIN_IMPL1 | PIN_IMPL2 | PIN_IMPL3 | PIN_IMPL4),
			GPIO_Mode_In_FL_IT); //GPIO_Mode_In_PU_IT);
//	GPIO_Init(PORT_IMPL, (GPIO_Pin_TypeDef)(PIN_IMPL1 | PIN_IMPL2 | PIN_IMPL3 | PIN_IMPL4), GPIO_Mode_In_PU_IT);//� ����� ���������� � ���� ������ ���������� �� 50 ��� ������
}
#pragma optimize=none
void LancherCallback()
{
	for(int i = 0; i < 4; i++)
	{
		if(countTim2[i])
		{
			if(--countTim2[i] == 0)
			{
				uint8_t j = i;
				if(ver == 0x12)
					j = i ^ (((i == 1) || (i == 2)) ? 3 : 0);
				//     if(i == 1)
				//releOff();
				if(gpioJ[i] == 0)
				{	// �� ����� ��� 0
					if((PORT_IMPL->IDR & IMPL_PIN[j]) == 0)
					{
						gpioJ[i] = 1;
						countImp[i]++;
						wasCheckedImpl = true;
#ifdef STORE_COUNT
						if((countImp[i] % 1000) == 0)
						writeIml[i] = 1;
#endif
					}
				}
				else
				{	// �� ����� ���� 1
					//if((PINB & (1 << PIN_IMPL1)) != 0)
					if(PORT_IMPL->IDR & IMPL_PIN[j])
						gpioJ[i] = 0;
				}
				GPIO_Init(PORT_IMPL, (GPIO_Pin_TypeDef)IMPL_PIN[j],
						GPIO_Mode_In_PU_IT);	//�������� ���������� �� �����
				if(i > 1)
				{
					if(timeForAlarm[i-2] == 0)//���� ������ 30 ����� � �����������, �� ����� ��������
					{
						wasAlarm = (i == 2) ? 0x82 : 0x84;
						timeForAlarm[i-2] = 30;
						isSendLora = true;
					}
				}
			}
		}
	}
}

void isrImpl(uint8_t port)
{
	timeToSleep = 200;
#ifndef USE_HALT	
	GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_No_IT );
	USART1->CR2 = USART_CR2_TCIEN | USART_CR2_REN | USART_CR2_TEN | USART_CR2_RIEN;
#endif
	lancher.up(port);
}

uint8_t getInput()
{
	disableInterrupts();//��������� ����������
	uint8_t input = wasAlarm;
	if(GPIOA->ODR & PIN_RELE)
		input |= 1;
	wasAlarm = 0;
	enableInterrupts();//��������� ����������
	return input;
}

