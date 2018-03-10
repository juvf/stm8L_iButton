/*
 * iButton.cpp
 *
 *  Created on: 3 мар. 2018 г.
 *      Author: juvf
 */
#include "iButton.h"
#include "timerJ.h"
#include "main.h"
#include "Serial.h"

const uint8_t goodKey[] = { 0x2c, 0xbb, 0xb4, 0x0c, 0, 0 };

extern uint8_t iBut;
uint8_t iarray[8];
#pragma optimize=none
void checkIButton()
{
	serial.print("i", false);
	serial.print(iBut);
	switch(iBut)
	{
		case 1: //сработало, возможно каснулись.
		case 2: //вторая попытка прочитать ключ
		case 3:
		case 4:
		case 5:
		{
			if(readKey())
			{ //прислонили ключ
				if(keyIsGood())
				{
					disableInterrupts();
					//меняем состояние охрана/снято
					switch(protection)
					{
						case 0: //снято с охраны - ставим
							timerProt = 10000;
							protection = 1;
							isSendLora = true;
							break;
						case 1: //постановка на охрану, отмена
							timerProt = 0;
							ledOff();
							protection = 0;
							isSendLora = true;
							break;
						case 3:
						case 2: //стоит на охране, сняте с охраны
							protection = 0;
							timerProt = 1000;
							isSendLora = true;
							break;
					}
					iBut = 7;
					protectPause = 1000;
					serial.print("pp", false);
					enableInterrupts();
				}
			}
			else
				iBut++;
		}
			break;
		case 6: //не смогли 2 раза прочитать. ложимси спать
			disableInterrupts();
			GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //разрешим прерывания
			iBut = 0;
			ledOff();
			enableInterrupts();
			break;
		case 7: //защитная пауза в 3 секунды
			disableInterrupts();
			if(protectPause == 0)
			{
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //разрешим прерывания
			}
			enableInterrupts();
			break;
	}
}

bool keyIsGood()
{
	for(uint8_t i = 0; i < 6; i++)
		if(iarray[i + 1] != goodKey[i])
			return false;
	return true;
}

bool readKey()
{
	delayMs(2);
	if(GPIOB->IDR & GPIO_Pin_3)
	{
		//записывем команду

		OWReadKey();
		static uint8_t y = iButtonCrc();
		if((y == 0) && (iarray[0] == 1))
			return true;
	}
	return false;
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
		GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0

		for(uint8_t y = 0; y < 3; y++)
			; //15 мкс паузы

		if(iarray[i])
			GPIOB->ODR |= GPIO_Pin_3; //шину в 1

		for(uint8_t y = 0; y < 20; y++)
			; //15 мкс паузы
		GPIOB->ODR |= GPIO_Pin_3; //шину в 1
		for(uint8_t y = 0; y < 2; y++)
			;
	}
	enableInterrupts();
}
#pragma optimize=none
void OWReadKey()
{
	for(uint8_t i = 0; i < 8; i++)
		iarray[i] = 0;
	disableInterrupts();
	GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0
	for(uint16_t y = 0; y < 250; y++)
		;
	GPIOB->ODR |= GPIO_Pin_3; //шину в 1
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
			GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0

			for(uint8_t y = 0; y < 2; y++)
				; //10 мкс паузы

			GPIOB->ODR |= GPIO_Pin_3; //шину в 1

			for(uint8_t y = 0; y < 3; y++)
				; //10 мкс паузы
			uint8_t bit = GPIOB->IDR & GPIO_Pin_3; //шину в 1
			//GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0
			//GPIOB->ODR |= GPIO_Pin_3; //шину в 1
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
