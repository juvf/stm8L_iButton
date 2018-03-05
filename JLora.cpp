/*
 * JLora.cpp
 *
 *  Created on: 19 июня 2017 г.
 *      Author: anisimov.e
 */

#include "JLora.h"
#include "Checksum.h"
#include "Serial.h"
#include "varInEeprom.h"
#include "timerJ.h"
#include "utils.h"
#include "main.h"

#include <string.h>

#define ATTEMPT	10

JLora::JLora()
{
	numPack = 0;
}

void JLora::printRegOfRfm95()
{
	for(int i = 0; i < 100; i++)
	{
		serial.print("reg adr = 0x");
		serial.print(i, HEX);
		serial.print("\tvalue = 0x");
		serial.println(rfm95.getReg(i), HEX);
		serial.flush();
	}
}
#pragma optimize=none
bool JLora::sendPayload(uint8_t protect)
{
	uint8_t array[64];
	uint8_t attempt = ATTEMPT;
	numPack++;
//uint32_t  frf = 14167244;
	do
	{
		array[0] = config.addressOfServer;
		array[1] = config.addressOfServer >> 8;
		array[2] = config.addressOfModul;
		array[3] = config.addressOfModul >> 8;
		array[4] = numPack;
		array[5] = numPack >> 8;
		array[6] = 4;
		array[7] = protect;
		array[8] = 0; //состояние входов
		array[9] = (uint8_t)config.countStarts; //состояние входов

		Checksum::addCrc16(array, 10);
		bool wasSended = false;
		serial.print("try to send ", false);
		serial.println(attempt);
//while(1)
		wasSended = rfm95.send(array, 12);

		if(wasSended)
		{
			serial.print("Send sucsesful\n\r");
			uint8_t len = 64;
			if(rfm95.reciveWithTimeout(array, &len, 3000))
			{
				serial.print("recived replay\n\r");
				if(Checksum::crc16(array, len) == 0)
				{
					serial.print("crc ok!\n\r");

					//uint16_t adr = WORD_FROM_ARRAY(&array[0]);
					//uint16_t sourc = WORD_FROM_ARRAY(&array[2]);
					//uint16_t pack = WORD_FROM_ARRAY(&array[4]);
					//uint8_t type = array[6];

					/*serial.print("src= ");
					serial.println(adr);
					serial.print("sourc= ");
					serial.println(sourc);
					serial.print("pack= ");
					serial.println(pack);
					serial.print("type= ");
					serial.println(type);*/

					if(((WORD_FROM_ARRAY(&array[0])) == config.addressOfModul)
							&& (WORD_FROM_ARRAY(&array[2])
									== config.addressOfServer)
							&& (WORD_FROM_ARRAY(&array[4]) == numPack)
							&& (array[6] == 4))
					{
						serial.print("recived ACK\n\r");
						return true;
					}
				}
				delayMs(millis() % 20 * 100);
			}
		}
	} while(--attempt > 0);
	return false;
}

bool JLora::waitAvailableTimeout(uint16_t timeout)
{
	return rfm95.waitAvailableTimeout(timeout);
}
bool JLora::recive(uint8_t* buf, uint8_t* len)
{
	return rfm95.recive(buf, len);
}
