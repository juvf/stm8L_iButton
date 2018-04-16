//Программа для модуля на STM8L051, проект Альтиума voda_stm8l_1
//Охранный модуль

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
#include "protect.h"

#include <string.h>

uint16_t timeToSleep = 0;
uint16_t timeToSleepUart = 250;
volatile uint16_t timeOpros = 0; //счетчик, при достижении которым будет отправка на распбери
uint8_t iBut = 0; //состояние работы с iButton
//int offset = -200;

volatile bool isSendLora; //пакет нужно отправить по охране
volatile bool isReadingKey; //получен запрос на чтение нового ключа
volatile bool keyIsReaded; //ключ прочитан
uint16_t timerReadKye; //таймер для чтения нового ключа
uint8_t protection; //текущее состояние охраны
extern uint8_t stateLora; //текущее состояние передачи по lora

uint16_t timerProt = 0; //таймер снятия постановки на охрану, мс
uint16_t protectPause = 0; //защита от дребезга iButton, мс

//#pragma optimize=none
int main()
{
	isSendLora = true;
	protection = 0;
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
	//int tik = 0;
//#ifdef SX1272
//	jLora.rfm95.setFrequency((uint32_t)(14166526));//14167244 - 500 ));//- 564));
//#endif

	while(1)
	{
		if(iBut)
			checkIButton();
		if(isSendLora)
		{
			while(timeToSleepUart > 0)
				;
			if(enTransmit)
			{
				if(stateLora == 0)
				{
					serial.print("\n\rSend to rf95", true);
					serial.print("protection = ", false);
					serial.println(protection);
					isSendLora = false;
				}
				loraRutine();
			}
			else
				isSendLora = false;
		}
		else if(stateLora != 0)
			loraRutine();

		switch(protection)
		{
			case 0:
				break;
			case 1: //ставим на охрану
				timeToSleep = 1000;
				if(timerProt == 0)
				{
					isSendLora = true;
					clearProtect();
					protection = 2;
				}
				break;
			case 4: //снимаем с охраны
			  	timeToSleep = 1000;
				if(timerProt == 0)
				{
				  	isSendLora = true;
					protection = 3;
				}
				break;
			case 3://тревога
				protect();
				break;
			case 2: //охраняем
				protect();
				if(getProtect() & 0x3)
				{
					timeToSleep = 1000;
					protection = 4;
					isSendLora = true;
				}
				break;
		}

		//waitResive();
		//serial.println(timeOpros);
		//serial.print(".");
		checkReadKey();
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
	EXTI->CR1 = (EXTI_Trigger_Rising << 6) //прерывание по переднему фронту для порта PB3 (1-Wire)
	| (EXTI_Trigger_Falling << 2) //прерывание по заднему фронту для порта PB1 (охрана)
			| (EXTI_Trigger_Falling << 4); //прерывание по заднему фронту для порта PB2 (охрана)

	GPIO_Init(GPIOB, GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_In_FL_IT); // PB1 PB2 на прерывание (охрана)

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
	if((timeToSleep == 0) && (timeToSleepUart == 0) && (stateLora == 0)
			&& !serial.isGetCommand() && !isSendLora && (timerProt == 0)
			&& (iBut == 0))
	{
		//releOn();
		jLora.rfm95.setMode(RHModeSleep);
		//serial.print("\n\rSleep  ", true);
		//serial.flush();

		delayMs(500);
#ifdef USE_HALT
		initHalt();
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

