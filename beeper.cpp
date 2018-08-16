/*
 * beeper.cpp
 *
 *  Created on: 19 мая 2018 г.
 *      Author: anisimov.e
 */

#include "beeper.h"
#include "stm8l15x.h"
#include "timerJ.h"

#define TIME_PAUSE	100

#define beepEn() (BEEP->CSR2 |= BEEP_CSR2_BEEPEN)
#define beepDis() (BEEP->CSR2 &= ~BEEP_CSR2_BEEPEN)

uint8_t curentPulse = 0;

uint16_t timers[2] = { 0, 0 };

void beeperInit()
{
	/* Wait for BEEP switch busy flag to be reset */
	while(CLK_GetFlagStatus(CLK_FLAG_BEEPSWBSY) == SET)
		;
	/* Select LSI clock as source for BEEP */
	CLK->CBEEPR = (uint8_t)(CLK_BEEPCLKSource_LSI);
	/* Enable BEEP clock to get write access for BEEP registers */
	CLK_PeripheralClockConfig(CLK_Peripheral_BEEP, ENABLE);

	BEEP->CSR2 &= ~0x1d; //делитель бипера на 4
	BEEP->CSR2 |= 0x80; //select = 1x: BEEPCLK/(2 x BEEP DIV ) kHz output
}
uint16_t beeperTimer;
void beepreRoutine()
{
	//номер если номер текущего импульса не 0, то проверить вермя
	switch(curentPulse)
	{
		case 0:
		default:
			return;
		case 1:
			if((millis() - beeperTimer) >= timers[0])
			{
				++curentPulse;
				beepDis();
				beeperTimer = millis();
			}
			break;
		case 2:
			if((millis() - beeperTimer) >= TIME_PAUSE)
			{
				if(timers[1])
				{
					++curentPulse;
					beepEn();
					beeperTimer = millis();
				}
				else
					curentPulse = 0;
			}
			break;
		case 3:
			if((millis() - beeperTimer) >= timers[1])
			{
				curentPulse = 0;
				beepDis();
			}
			break;
	}
}

void beepOn(uint16_t timePulse1, uint16_t timePulse2)
{
	timers[0] = timePulse1;
	timers[1] = timePulse2;
	beeperTimer = millis();
	curentPulse = 1;
	beepEn();
}

