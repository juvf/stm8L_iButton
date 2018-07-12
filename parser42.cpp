/*
 * parser42.cpp
 *
 *  Created on: 8 мая 2017 г.
 *      Author: anisimov.e
 */

#include "parser42.h"
#include "Serial.h"
#include "Checksum.h"
#include "main.h"
#include "Lancher.h"
#include "rfClient.h"
#include "JLora.h"
#include "varInEeprom.h"
#include "utils.h"
#include "timerJ.h"
#include <string.h>

extern JLora jLora;
//#pragma optimize=none
void parser42(uint8_t *buffer)
{
	if(buffer[2] >= MAX_SIZE)
	{
		buffer[2] = 0;
		return;
	}
	switch(buffer[3])
	{
		case L_WriteMem:
		{
			//__eeprom uint8_t *p;// = (__eeprom uint8_t *)buffer[4];
			uint8_t len = buffer[2] - 7;
			uint16_t adr = 0x1000 + buffer[4];
			EEPROM_Unlock();
			for(uint8_t i = 0; i < len; i++)
				FLASH_ProgramByte(adr++, buffer[6 + i]);
			EEPROM_Lock();
			buffer[2] = 5;
		}
			break;
		case L_ReadMem:
		{
			uint8_t len = buffer[5];
			uint16_t adr = 0x1000 + buffer[4];
			for(uint8_t i = 0; i < len; i++)
				buffer[4 + i] = FLASH_ReadByte(adr++);
			buffer[2] = len + 6;
		}
			break;
		case L_ReadKey://получили запрос на чтение ключа
			isReadingKey = true;
			keyIsReaded = false;
			timerReadKye = millis();
			timerProt = 10000;
			buffer[4] = 1;//тип 1 - старт чтения ключа
			buffer[2] = 7;
			break;
	}
	buffer[0] = 0x19;
	buffer[1] = 0x2;
	Checksum::addCrc16(buffer, buffer[2] - 2);
}

void setLoraFreq(uint8_t numCh)
{
	if((numCh >= 7) && (numCh <= 10))
	{
		config.numChanel = numCh;
		//jLora.setNumChanel(numCh);
		///jLora.rfm95.setFrequency(numCh);
	}
}

