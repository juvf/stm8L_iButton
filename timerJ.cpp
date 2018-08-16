#include "timerJ.h"
#include "Lancher.h"
#include "main.h"
#include "varInEeprom.h"
#include "beeper.h"

uint32_t msDelay = 0;
volatile uint16_t msMillis = 0;

#pragma optimize=none
void delayMs(uint32_t ms)
{
	msDelay = ms;
	while(msDelay > 0)
	{
	}
}

uint16_t millis()
{
	return msMillis;
}

void intiTimerJ()
{
	/* Enable TIM4 CLK */
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
	/* Time base configuration */
	TIM4_TimeBaseInit(TIM4_Prescaler_16, 125); //f = 2MHz. for 1 ms 2_000_000/16 * 125 = 1000 (1 kHz or period = 1 ms)
			/* Clear TIM4 update flag */
	TIM4_ClearFlag(TIM4_FLAG_Update);
	/* Enable update interrupt */
	TIM4_ITConfig(TIM4_IT_Update, ENABLE); // TIM4->IER |= (uint8_t)TIM4_IT_UPDATE;//TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

	/* Enable TIM4 */
	TIM4_Cmd(ENABLE);
}

void tim4Handler()
{
	if(msDelay)
		--msDelay;

	if(timeToSleep)
		--timeToSleep;

	++msMillis;
	if(timerProt)
	{
		--timerProt;
		if(timerProt % 1000 > 500)
			ledOn();
		else
			ledOff();
	}
	if(timeToSleepUart)
		--timeToSleepUart;
	if(protectPause)
		--protectPause;

	if(msMillis%10 == 0)
	{
		beepreRoutine();
	}
}

