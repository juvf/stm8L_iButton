#include "utils.h"
#include "Serial.h"
#include "parser42.h"
#include "timerJ.h"
#include "varInEeprom.h"
#include "main.h"
#include "Checksum.h"

extern uint16_t timeOpros;
extern bool isSacsesfulSend; //���� false, �� �������� ��������� ����� �� ����� ��� ����� ���
extern volatile bool isSendLora;
extern uint8_t buffer42[MAX_SIZE];
extern uint8_t iarray[8];
#define MAX_OPROS 60

char *itoa(uint16_t number, char *destination, int base)
{
	int count = 0;
	do
	{
		uint16_t digit = number % base;
		destination[count++] = (digit > 9) ? digit - 10 + 'A' : digit + '0';
	} while((number /= base) != 0);
	destination[count] = '\0';
	int i;
	for(i = 0; i < count / 2; ++i)
	{
		char symbol = destination[i];
		destination[i] = destination[count - i - 1];
		destination[count - i - 1] = symbol;
	}
	return destination;
}

/*
 void RTC_WakeUpClockConfig(RTC_WakeUpClock_TypeDef RTC_WakeUpClock);
 void RTC_SetWakeUpCounter(uint16_t RTC_WakeupCounter);
 uint16_t RTC_GetWakeUpCounter(void);
 ErrorStatus RTC_WakeUpCmd(FunctionalState NewState);*/
void initWakeup()
{
	/* Enable RTC clock */
	CLK->CRTCR = (uint8_t)((uint8_t)CLK_RTCCLKSource_LSI
			| (uint8_t)CLK_RTCCLKDiv_64);
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);

	/* Disable the write protection for RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->WUTRH = (uint8_t)(rtcWakeupCounter >> 8);
	RTC->WUTRL = (uint8_t)(rtcWakeupCounter);

	//RTC_SetWakeUpCounter(10);

	//uint16_t aa= RTC_GetWakeUpCounter();

	RTC->CR1 = RTC_WakeUpClock_RTCCLK_Div8;
	RTC->CR2 = RTC_CR2_WUTIE | RTC_CR2_WUTE;

	RTC->WPR = 0xFF;
}
#pragma optimize=none
void wakeup()
{
	if(++timeOpros >= periodOprosa)
	{
		timeOpros = 0;
		isSendLora = true;
	}
}

void initHalt()
{
	GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_IT); //
	USART1->CR2 = 0;
}

void memcpyN(uint8_t *dis, const uint8_t *source, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		dis[i] = source[len - i - 1];
}
void sendReplay()
{
	serial.clearIsResive();
	bool isReset = (buffer42[3] == L_WriteMem);
	parser42(buffer42);
	if(buffer42[2] > 0)
	{
		serial.sentBytes(buffer42, buffer42[2]);
		if(isReset)
		{
			delayMs(500);
			WWDG->CR = WWDG_CR_WDGA;
		}
	}
}

void checkReadKey()
{
	if(isReadingKey)
	{
		if(keyIsReaded)
		{
			//����� �����
			isReadingKey = false;
			buffer42[0] = 0x19;
			buffer42[1] = 0x02;
			buffer42[2] = 13;
			buffer42[3] = 16;
			buffer42[4] = 2;
			buffer42[5] = iarray[1];
			buffer42[6] = iarray[2];
			buffer42[7] = iarray[3];
			buffer42[8] = iarray[4];
			buffer42[9] = iarray[5];
			buffer42[10] = iarray[6];
			Checksum::addCrc16(buffer42, buffer42[2] - 2);
			serial.sentBytes(buffer42, buffer42[2]);
			timerProt = 0;
		}
		else if((millis() - timerReadKye) > 10000)
		{
			//����� ������� �������� �����, ����� �����
			isReadingKey = false;
			buffer42[0] = 0x19;
			buffer42[1] = 0x02;
			buffer42[2] = 7;
			buffer42[3] = 16;
			buffer42[4] = 3;
			Checksum::addCrc16(buffer42, buffer42[2] - 2);
			serial.sentBytes(buffer42, buffer42[2]);
			timerProt = 0;
		}
	}
}
