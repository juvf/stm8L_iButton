/*
 * iButton.cpp
 *
 *  Created on: 3 мар. 2018 г.
 *      Author: juvf
 */
#include "iButton.h"
#include "timerJ.h"
#include "main.h"
#pragma optimize=none
void checkIButton()
{
	//if((GPIOB->IDR & GPIO_Pin_3) == 0)//на детекторе 0
	{
		GPIOB->ODR &= ~GPIO_Pin_0; //включить питание
		//перевести шину в мастера
		GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_OD_HiZ_Slow); //шину в мастер
		//паузу 1 мс, чтоб устаканилось всё
		//delay_us(1000);

		//записывем команду
		OWWriteByte(0x33);
		uint8_t array[8] = {0,0,0,0,0,0,0,0};
		for(uint8_t i = 0; i < 8; i++)
		{
			disableInterrupts();
			for(uint8_t j = 0; j < 8; j++)
			{
				GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0

				for(uint8_t y = 0; y < 2; y++)
					; //10 мкс паузы

				GPIOB->ODR |= GPIO_Pin_3; //шину в 1

				for(uint8_t y = 0; y < 8; y++)
					; //10 мкс паузы
				uint8_t bit = GPIOB->IDR & GPIO_Pin_3; //шину в 1
				array[i] |= (bit ? 1 : 0)<<j;

				for(uint8_t y = 0; y < 2; y++)
					; //10
			}
			enableInterrupts();
		}
		//читаем данные

		//GPIOB->ODR |= GPIO_Pin_0;		//включить питание
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
		GPIOB->ODR &= ~GPIO_Pin_3; //шину в 0

		for(uint8_t y = 0; y < 3; y++)
			; //15 мкс паузы

		if(array[i])
			GPIOB->ODR |= GPIO_Pin_3; //шину в 1

		for(uint8_t y = 0; y < 20; y++)
			; //15 мкс паузы
		GPIOB->ODR |= GPIO_Pin_3; //шину в 1
		for(uint8_t y = 0; y < 2; y++)
			;
	}
	enableInterrupts();

}

uint8_t OWReadByte()
{

}
