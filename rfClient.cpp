#include "rfClient.h"

#include "Serial.h"
#include "main.h"
#include "Checksum.h"
#include "JLora.h"
#include "timerJ.h"
#include "eeprom.h"
#include "varInEeprom.h"
#include "utils.h"

#include <string.h>

#define THIS_ADDRESS 2
#define SERVER_ADDRESS 1

const char *txt_isOk = "init is Ok";
volatile bool isSendLora = false; //если true, то пора отправлять данные на сервер

JLora jLora;

void setupRf95()
{
/*	serial.print("\n\rStart Program\n\r");
	serial.flush();
	serial.print(__DATE__);
	serial.print("   ");
	serial.print(__TIME__, true);
	serial.flush();*/
	
	
	bool isOk = jLora.rfm95.initial();
	if(!isOk)
	  isOk = jLora.rfm95.initial();
	
	if(isOk)
	{
	  serial.print(txt_isOk, true);
	  jLora.rfm95.setFrequency(config.freq);
	  serial.print("Listening on chanel frequency: ", false);
	serial.println(config.numChanel);
	}
	else
	  serial.print("init failed", true);
	serial.flush();	
	//jLora.printRegOfRfm95();
}

void waitResive()
{
	uint8_t buf[32];//RH_RF95_FIFO_SIZE];
	uint8_t len = sizeof(buf);
	if(0)//jLora.waitAvailableTimeout(100))
	{
		if(jLora.recive(buf, &len))
		{
		  serial.print("Got Packed. RSSI: ");
//			serial.println(rf95.lastRssi(), DEC);
			parserLoraProtocol(buf, len);
		}
		//else
		{
			//serial.println("recv failed");
		}
	}
}

void parserLoraProtocol(uint8_t *buffer, uint8_t len)
{
	if(Checksum::crc16(buffer, len) == 0)
	{
		PackJ pack;
		getPackJ(&pack, buffer);
		if((pack.dst == config.addressOfModul) && (pack.src == config.addressOfServer))
		{
			switch(pack.type)
			{
				default:
					break;
				case 1: //команда управление GPIO
				{
					if(pack.data[0] == 1)
					{
						if(pack.data[1])
						  	PORT_RELE->ODR &= (uint8_t)(~PIN_RELE);
						else
							PORT_RELE->ODR |= PIN_RELE;
					}
				}
					break;
			}
		}
	}
}

void getPackJ(PackJ *pack, uint8_t *buffer)
{
	pack->dst = buffer[0] | (buffer[1] << 8);
	pack->src = buffer[2] | (buffer[3] << 8);
	pack->packnum = buffer[4] | (buffer[5] << 8);
	pack->type = buffer[6];
	switch(pack->type)
	{
		case 1: //управление выходными GPIO
			pack->payloadlength = 2;
			break;
		default:
			pack->payloadlength = 0;
			break;
	}
	memcpy(pack->data, buffer + 7, pack->payloadlength);
}

