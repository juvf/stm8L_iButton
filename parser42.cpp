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

void parser42(uint8_t *buffer)
{
	if(buffer[2] >= MAX_SIZE)
	{
		buffer[2] = 0;
		return;
	}
	switch(buffer[3])
	{
	/*	case L_Echo: //
			buffer[2] = 7;
			serial.setDebugMode(buffer[2]);
			break;
		case L_SetPeriod:
		{
			e_periodOprosa = buffer[4] + ((uint16_t)buffer[5] << 8);
			////flashProgram(EEPROM_PERIOD, (uint8_t*)&min, 2);
			isSendLora = true;
			// TODO setAlarm();
		}
			break;
		case L_SetThisAddress:
		{
			e_addressOfModul = buffer[4] + ((uint16_t)buffer[5] << 8);
			;
		}
			break;
		case L_SetServerAddress:
		{
			e_addressOfServer = buffer[4] + ((uint16_t)buffer[5] << 8);
		}
			break;
		case L_SetImplCounter:
		{
			uint8_t numCount = buffer[8];
			if(numCount < 4)
			{
				buffer[2] = 11;
				memcpyN((uint8_t*)&countImp[numCount], &buffer[4],
						sizeof(uint32_t));
				writeIml[numCount] = 1;
				///		if(numCount == 0)
				///			FLASH_ProgramU32( EEPROM_COUNT_IMPL_1, countImp[0]);
				///		else
				///			FLASH_ProgramU32(EEPROM_COUNT_IMPL_2, countImp[1]);
			}
			isSendLora = true;
			// TODO setAlarm();
		}
			break;
		case L_SetLoraMode:
			break;
		case L_SetFreqChanel:
			setLoraFreq(buffer[4]);
			break;
		case L_GetPeriod:
			buffer[2] = 8;
			buffer[4] = e_periodOprosa;
			buffer[5] = e_periodOprosa >> 8;
			break;
		case L_GetThisAddress:
			buffer[2] = 8;
			buffer[4] = e_addressOfModul;
			buffer[5] = e_addressOfModul >> 8;
			break;
		case L_GetServerAddress:
		{
			buffer[2] = 8;
			buffer[4] = e_addressOfServer;
			buffer[5] = e_addressOfServer >> 8;
		}
			break;
		case L_GetImplCounter:
		{
			uint8_t numCount = buffer[4];
			if(numCount < 4)
			{
				buffer[2] = 11;
				memcpyN(&buffer[4], (uint8_t*)&countImp[numCount],
						sizeof(uint32_t));
				buffer[8] = numCount;
			}
		}
			break;
		case L_GetLoraMode:
		case L_GetFreqChanel:
			buffer[2] = 7;
			buffer[4] = e_numChanel;		//jLora.getNumChanel();
			break;*/
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

