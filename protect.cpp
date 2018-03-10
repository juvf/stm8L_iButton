/*
 * protect.cpp
 *
 *  Created on: 10 мар. 2018 г.
 *      Author: juvf
 */
#include "protect.h"
#include "main.h"
#define PIN_PROTECT_1 GPIO_Pin_1 //вход охрана 1
#define PIN_PROTECT_2 GPIO_Pin_2 //вход охрана 2

uint8_t oldState[2];

void protect()
{
	if((GPIOB->IDR & PIN_PROTECT_1) == 0) //порт 1 сработал
	{
		if(oldState[0] == 0)
		{
			if(protection == 2)
			{
				if(timerProt == 0)
					timerProt = 10000;
			}
			oldState[0] = 1;
			isSendLora = true;
		}
	}

	if((GPIOB->IDR & PIN_PROTECT_2) == 0) //порт 1 сработал
	{
		if(oldState[1] == 0)
		{
			if(protection == 2)
			{
				if(timerProt == 0)
					timerProt = 10000;
			}
			oldState[1] = 1;
			isSendLora = true;
		}
	}
}

void clearProtect()
{
	oldState[0] = 0;
	oldState[1] = 0;
}

uint8_t getProtect()
{
	uint8_t result = 0;
	if(oldState[0])
		result |= (1 << 0);
	if(oldState[1])
		result |= (1 << 1);
	if(GPIOB->IDR & PIN_PROTECT_1)
		result |= (1 << 2);
	if(GPIOB->IDR & PIN_PROTECT_2)
		result |= (1 << 3);
	return result;
}
