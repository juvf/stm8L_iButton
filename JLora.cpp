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
#include "protect.h"

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

//bool JLora::waitAvailableTimeout(uint16_t timeout)
//{
//	return rfm95.waitAvailableTimeout(timeout);
//}

//bool JLora::recive(uint8_t* buf, uint8_t* len)
//{
//	return rfm95.recive(buf, len);
//}
