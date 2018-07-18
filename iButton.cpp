/*
 * iButton.cpp
 *
 *  Created on: 3 ���. 2018 �.
 *      Author: juvf
 */
#include "iButton.h"
#include "timerJ.h"
#include "main.h"
#include "Serial.h"
#include "varInEeprom.h"
#include "beeper.h"

//const uint8_t goodKey[] = { 0x2c, 0xbb, 0xb4, 0x0c, 0, 0 };

extern uint8_t iBut; //��������� ������ ����������� iButton
uint8_t iarray[8];
//#pragma optimize=none
void checkIButton()
{
	GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_OD_HiZ_Slow);
	serial.print("i", false);
	serial.print(iBut);
	switch(iBut)
	{
		case 1: //���������, �������� ���������.
		case 2: //������ ������� ��������� ����
		case 3:
		case 4:
		case 5:
		{
			if(readKey())
			{ //���������� ����
				if(isReadingKey)
				{
					keyIsReaded = true;
					iBut = 7;
				}
				else if(keyIsGood())
				{
					disableInterrupts();
					//������ ��������� ������/�����
					switch(protection)
					{
						case 0: //����� � ������ - ������
							beepOn(300);
							timerProt = 10000;
							protection = 1;
							isSendLora = true;
							break;
						case 1: //���������� �� ������, ������
							timerProt = 0;
							beepOn(200, 200);
							ledOff();
							protection = 0;
							isSendLora = true;
							break;
						case 3:
						case 2: //����� �� ������, ����� � ������
						case 4:
							beepOn(200, 200);
							protection = 0;
							timerProt = 1000;
							isSendLora = true;
							break;

					}
					iBut = 7;
					protectPause = 1000;
					serial.print("p!", false);
					enableInterrupts();
				}
				ibut = 6;
			}
			else
				iBut++;
		}
			break;
		case 6:
//�� ������ ���� ���������. ������� �����
			disableInterrupts();
			GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
			iBut = 0;
			ledOff();
			enableInterrupts();
			break;
		case 7:
//�������� ����� � 3 �������
			disableInterrupts();
			if(protectPause == 0)
			{
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //�������� ����������
			}
			enableInterrupts();
			break;
	}
}

/*
 * �������� ������� ���� ��� ���. ���� ��� ����� � ��������� ������� iarray
 * ���� � ������ �����������, �� ���������� true, ����� false
 */
bool keyIsGood()
{
	for(uint8_t j = 0; j < 5; j++)
	{
		if(config.flags | (1 << j))
		{
			bool result = true;
			for(uint8_t i = 0; i < 6; i++)
			{
				if(iarray[i + 1] != config.iButton[j][i])
				{
					result = false;
					break;
				}
			}
			if(result)
				return true;
		}
	}
	return false;
}

/*
 * ������ ���� ����� � ������  iarray[]
 * ����� ��������� �������� CRC.
 * ���� ������ �������� � CRC �������, �� ���������� true, ����� false
 */
bool readKey()
{
	delayMs(2);
	if(GPIOB->IDR & GPIO_Pin_3)
	{
		OWReadKey();
		uint8_t y = iButtonCrc();
		if((y == 0) && (iarray[0] == 1))
			return true;
	}
	return false;
}

/*
 * ������ ����� �� ���� 1-Wire
 */
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

/*
 * ������ ������ ����� � ���� 1-Wire
 */
#pragma optimize=none
void OWReadKey()
{
	for(uint8_t i = 0; i < 8; i++)
		iarray[i] = 0;
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

/*
 * ������� CRC ��� ��������� ��� iButton
 */
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
