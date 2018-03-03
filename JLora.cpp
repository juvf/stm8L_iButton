/*
 * JLora.cpp
 *
 *  Created on: 19 θώνÿ 2017 γ.
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

#define ATTEMPT	3

JLora::JLora()
{
	numPack = 0;
}

void JLora::printRegOfRfm95()
{
	for(int i = 0; i<100; i++)
	{
		serial.print("reg adr = 0x");
		serial.print(i, HEX);
		serial.print("\tvalue = 0x");
		serial.println(rfm95.getReg(i), HEX);
		serial.flush();
	}
}
#pragma optimize=none
bool JLora::sendPayload(uint8_t type, const uint8_t *data, uint8_t len)
{
	uint8_t array[64];
	if((len + LORA_HEADER_LEN) > 63)
		return false;
	int attempt;
	if(data[8] | 0x80)
		attempt = ATTEMPT;
	else
		attempt = 1;
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
	array[6] = type;
//serial.print("offs = ");
//serial.println(offs);
//rfm95.setFrequency((uint32_t)(frf -  offs));
//offs += 2;
//if(offs > 5000)
//offs = 2000;

	memcpy(&array[LORA_HEADER_LEN], data, len);
	Checksum::addCrc16(array, LORA_HEADER_LEN + len );
	bool wasSended = false;
serial.print("try to send ", false); serial.println(attempt);
//while(1)
		wasSended = rfm95.send(array, LORA_HEADER_LEN + len + 2);


		if(wasSended)
		{
//serial.print("Send sucsesful\n\r");
		  uint8_t len = 64;
		  if(rfm95.reciveWithTimeout(array, &len, 3000))
		  {
serial.print("recived replay\n\r");			
			if(Checksum::crc16(array, len) == 0)
			{
serial.print("crc ok!\n\r");	

uint16_t adr = WORD_FROM_ARRAY(&array[0]);
uint16_t sourc = WORD_FROM_ARRAY(&array[2]);
uint16_t pack = WORD_FROM_ARRAY(&array[4]);
uint8_t type = array[6];

serial.print("src= "); serial.println(adr);
serial.print("sourc= "); serial.println(sourc);
serial.print("pack= "); serial.println(pack);
serial.print("type= "); serial.println(type);



				 if( ((WORD_FROM_ARRAY(&array[0])) == config.addressOfModul) && 
					(WORD_FROM_ARRAY(&array[2]) == config.addressOfServer) &&
					(WORD_FROM_ARRAY(&array[4]) ==  numPack) && 
					( array[6] == 4) )
				 {
serial.print("recived ACK\n\r");
				   return true;
				 }
			}
			delayMs(millis()%20 * 100);
		  }
		}
	}while(--attempt > 0);
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
