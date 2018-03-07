//Программа для модуля на STM8L051, проект Альтиума voda_stm8l_1

#include "stm8l15x.h"
#include "eeprom.h"
#include "varInEeprom.h"
#include "timerJ.h"
#include "main.h"
#include "Serial.h"
#include "JRfm95.h"
#include "rfClient.h"
#include "utils.h"
#include "eeprom.h"
#include "iButton.h"

#include <string.h>

uint8_t timeToSleep = 0;
uint16_t timeToSleepUart = 250;
volatile uint16_t timeOpros = 0; //счетчик, при достижении которым будет отправка на распбери
uint8_t iBut = 0; //состояние работы с iButton
//int offset = -200;
bool isSacsesfulSend; //если false, то передать следующий пакет не позже чем через час
uint8_t protection = 0;

uint8_t sendProtect = 0; //пакет который нужно отправить по охране

uint16_t timerProt = 0;//таймер снятия постановки на охрану, мс
uint16_t protectPause = 0;//защита от дребезга iButton, мс

int main()
{
	isSacsesfulSend = true;
	disableInterrupts();

	EEPROM_Unlock();
	config.countStarts++;
	EEPROM_Lock();
	initPeref();
	ledOn();
	bool enTransmit = (config.isProgramm == 0x55);

	if(enTransmit)
		periodOprosa = config.periodOprosa;
	else
		periodOprosa = 0xffff;

	delayMs(1000);
	ledOff();
	//isSendLora = true;
	//int tik = 0;
//#ifdef SX1272
//	jLora.rfm95.setFrequency((uint32_t)(14166526));//14167244 - 500 ));//- 564));
//#endif

	while(1)
	{
		if( iBut )
			checkIButton();
		switch(protection)
		{
			case 0:
				break;
			case 1://ставим на охрану
				if(timerProt == 0)
					protection = 4;
				break;
			case 2://снимаем с охраны
				if(timerProt == 0)
					protection = 3;
				break;
			case 3:
				break;
			case 4:
				break;
		}

		//if(isSendLora)
		if(sendProtect)
		{
			while(timeToSleepUart > 0)
				;
			if(enTransmit)
				isSacsesfulSend = rfClientLoop(sendProtect);
			else
				isSacsesfulSend = true;
			if(periodOprosa)
				isSendLora = false;
			else
				delayMs(10);
		}
		//waitResive();
		//serial.println(timeOpros);
		//serial.print(".");
		if(serial.isGetCommand())
			sendReplay();
		checkSleep();
	}
}

void initPeref()
{
	GPIO_Init(PORT_LED, PIN_LED, GPIO_Mode_Out_PP_Low_Slow); // Порт управление Led PB_10
    GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); // Порт 1-Wire на iButton

    //настроим прерывания
    EXTI->CR1 = EXTI_Trigger_Rising << 6; //прерывание по переднему фронту для порта PB3 (1-Wire)


	serial.init();
	intiTimerJ();
	initWakeup();
	//GPIO_Init(GPIOC, RFM_PIN_RESET, GPIO_Mode_Out_PP_High_Slow );
	GPIO_Init(GPIOC, RFM_PIN_RESET, GPIO_Mode_Out_OD_HiZ_Slow);
	//неиспользуемые пины
	GPIO_Init(GPIOA, GPIO_Pin_1 | GPIO_Pin_0, GPIO_Mode_In_PU_No_IT); //GPIO_Mode_In_PU_No_IT
	GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_Mode_In_PU_No_IT);
	GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_In_PU_No_IT); //initial interupt port for Rfm95 Rising

	enableInterrupts();

	serial.println(config.countStarts);
	delayMs(1000);

	setupRf95();

	CLK->ICKCR |= CLK_ICKCR_SAHALT;
	PWR->CSR2 |= PWR_CSR2_ULP;
}

//#pragma optimize=none
void checkSleep()
{
	serial.flush();
	delayMs(50); //пауза, чтобы закончил работу уарт;
	if((timeToSleep == 0) && (timeToSleepUart == 0) && !jLora.rfm95.isSending()
			&& !serial.isGetCommand() && !isSendLora)
	{
		//releOn();
		jLora.rfm95.setMode(RHModeSleep);
		//serial.print("\n\rSleep  ", true);
		//serial.flush();

		delayMs(500);
#ifdef USE_HALT
		initHaly();
		//CLK->PCKENR1 = 0;
		//GPIO_Init(GPIOB, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_In_PU_No_IT );

		halt();

		GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_No_IT);
		USART1->CR2 = USART_CR2_TCIEN | USART_CR2_REN | USART_CR2_TEN
				| USART_CR2_RIEN;
#endif
		//serial.print("wakeup\n\r", true);
		//serial.flush();
		//releOff();
	}
}

