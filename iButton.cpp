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
uint8_t iarray[8];
#pragma optimize=none
void checkIButton()
{
	switch(iBut)
	{
		case 1: //���������, �������� ���������.
		case 2: //������ ������� ��������� ����
		{
			if(readKey())
			{ //���������� ����
				if(keyIsGood())
				{
				  disableInterrupts();
					//������ ��������� ������/�����
					switch(protection)
					{
						case 0: //����� � ������ - ������
							timerProt = 10000;
							protection = 1;
							break;
						case 1: //���������� �� ������
							timerProt = 0;
							ledOff();
							protection = 0;
							break;
						case 2: //�������� ������, ����� � ������
							protection = 0;
							ledOff();
							break;
						case 4: //����� �� ������
							timerProt = 1000;
							protection = 0;
							break;
						case 3://���� �������, ���������
							protection = 0;
							break;
					}
					iBut = 4;
					protectPause = 1000;
					enableInterrupts();
				}
			}
			else
				iBut++;
		}
			break;
		case 3: //�� ������ 2 ���� ���������. ������� �����
			break;
		case 4: //�������� ����� � 3 �������
			if(protectPause == 0)
			{
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
			}
			break;
	}
}

bool keyIsGood()
{
	return true;
	for(uint8_t i = 1; i < 7; i++)
		if(iarray[i] != 0)
			return false;
	return true;
}

bool readKey()
{
	delayMs(2);
	if(GPIOB->IDR & GPIO_Pin_3)
	{
		//��������� �������

		OWReadKey();
		uint8_t y = iButtonCrc();
		if(y == 0)
			return true;
		GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
	}
	return false;
	//else
	{
		iBut = 0;
		GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
	}
}

#pragma optimize=none
void OWWriteByte(uint8_t byte)
{
	disableInterrupts();
	for(uint8_t i = 0; i < 8; i++)
	{
		iarray[i] = (byte >> i) & 1;
	}
	for(uint8_t i = 0; i < 8; i++)
	{
		GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0

		for(uint8_t y = 0; y < 3; y++)
			; //15 ��� �����

		if(iarray[i])
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
		;
	GPIOB->ODR |= GPIO_Pin_3; //���� � 1
	//enableInterrupts();

	for(uint8_t y = 0; y < 40; y++)
		; //100 us
	if(GPIOB->IDR & GPIO_Pin_3)
	{
		enableInterrupts();
		return;
	}
	for(uint8_t y = 0; y < 30; y++)
		;
	enableInterrupts();

	OWWriteByte(0x33);

	for(uint8_t i = 0; i < 8; i++)
	{
		iarray[i] = 0;
		disableInterrupts();
		for(uint8_t j = 0; j < 8; j++)
		{
			GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0

			for(uint8_t y = 0; y < 2; y++)
				; //10 ��� �����

			GPIOB->ODR |= GPIO_Pin_3; //���� � 1

			for(uint8_t y = 0; y < 3; y++)
				; //10 ��� �����
			uint8_t bit = GPIOB->IDR & GPIO_Pin_3; //���� � 1
			//GPIOB->ODR &= ~GPIO_Pin_3; //���� � 0
			//GPIOB->ODR |= GPIO_Pin_3; //���� � 1
			iarray[i] |= (bit ? 1 : 0) << j;

			for(uint8_t y = 0; y < 5; y++)
				; //10
		}
		enableInterrupts();
	}
}

uint8_t iButtonCrc()
{
	uint8_t crc = 0;
	for(int i = 0; i < 8; i++)
	{
		crc ^= iarray[i];
		for(int j = 0; j < 8; j++)
		{
			if(crc & 1)
				crc = (crc >> 1) ^ 0x8c;
			else
				crc >>= 1;
		}
	}
	return crc;
}
