/*
 * iButton.cpp
 *
 *  Created on: 3 мар. 2018 г.
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
		case 1: //сработало, возможно каснулись.
		case 2: //вторая попытка прочитать ключ
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
							break;
						case 1: //постановка на охрану
							timerProt = 0;
							ledOff();
							protection = 0;
							break;
						case 2: //сработал датчик, сняте с охраны
							protection = 0;
							ledOff();
							break;
						case 4: //стоит на охране
							timerProt = 1000;
							protection = 0;
							break;
						case 3://была тревога, отключаем
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
		case 3: //не смогли 2 раза прочитать. ложимси спать
			break;
		case 4: //защитная пауза в 3 секунды
			if(protectPause == 0)
			{
				iBut = 0;
				GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //разрешим прерывания
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
		//записывем команду

		OWReadKey();
		uint8_t y = iButtonCrc();
		if(y == 0)
			return true;
		GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //разрешим прерывания
	}
	return false;
	//else
	{
		iBut = 0;
		GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); //разрешим прерывания
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
