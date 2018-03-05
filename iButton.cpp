/*
 * iButton.cpp
 *
 *  Created on: 3 ���. 2018 �.
 *      Author: juvf
 */
#include "iButton.h"
#include "timerJ.h"
#include "main.h"

extern uint8_t iBut;
uint8_t array[8];
#pragma optimize=none
void checkIButton()
{
	switch(iBut)
	{
		case 1: //���������, �������� ���������.
		{
//			�������� 2 ��
			delayMs(2);
			if(GPIOB->IDR & GPIO_Pin_3)
			{
				//��������� �������

				OWReadKey();
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������

			}
			else
			{
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
			}

		}
			break;
	}

}
#pragma optimize=none
void OWWriteByte(uint8_t byte)
{
	disableInterrupts();
	uint8_t array[8];
	for(uint8_t i = 0; i < 8; i++)
	{
		array[i] = (byte >> i) & 1;
	}
	for(uint8_t i = 0; i < 8; i++)
	{
		GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0

		for(uint8_t y = 0; y < 3; y++)
			; //15 ��� �����

		if(array[i])
			GPIOB->ODR |= GPIO_Pin_3; //���� � 1

		for(uint8_t y = 0; y < 20; y++)
			; //15 ��� �����
		GPIOB->ODR |= GPIO_Pin_3; //���� � 1
		for(uint8_t y = 0; y < 2; y++)
			;
	}
	enableInterrupts();
}
#pragma optimize=none
void OWReadKey()
{   
	disableInterrupts();
	GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0
	for(uint16_t y = 0; y < 250; y++)
				; //10
	GPIOB->ODR |= GPIO_Pin_3; //���� � 1
	enableInterrupts();
	
	OWWriteByte(0x33);

	for(uint8_t i = 0; i < 8; i++)
	{
		disableInterrupts();
		for(uint8_t j = 0; j < 8; j++)
		{
			GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0

			for(uint8_t y = 0; y < 2; y++)
				; //10 ��� �����

			GPIOB->ODR |= GPIO_Pin_3; //���� � 1

			for(uint8_t y = 0; y < 8; y++)
				; //10 ��� �����
			uint8_t bit = GPIOB->IDR & GPIO_Pin_3; //���� � 1
			array[i] |= (bit ? 1 : 0) << j;

			for(uint8_t y = 0; y < 2; y++)
				; //10
		}
		enableInterrupts();
	}
}
