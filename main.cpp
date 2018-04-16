//��������� ��� ������ �� STM8L051, ������ �������� voda_stm8l_1
//�������� ������

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
volatile uint16_t timeOpros = 0; //�������, ��� ���������� ������� ����� �������� �� ��������
uint8_t iBut = 0; //��������� ������ � iButton
//int offset = -200;

volatile bool isSendLora; //����� ����� ��������� �� ������
volatile bool isReadingKey; //������� ������ �� ������ ������ �����
volatile bool keyIsReaded; //���� ��������
uint16_t timerReadKye; //������ ��� ������ ������ �����
uint8_t protection; //������� ��������� ������
extern uint8_t stateLora; //������� ��������� �������� �� lora

uint16_t timerProt = 0; //������ ������ ���������� �� ������, ��
uint16_t protectPause = 0; //������ �� �������� iButton, ��

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
			case 1: //������ �� ������
				timeToSleep = 1000;
				if(timerProt == 0)
				{
					isSendLora = true;
					clearProtect();
					protection = 2;
				}
				break;
			case 4: //������� � ������
			  	timeToSleep = 1000;
				if(timerProt == 0)
				{
				  	isSendLora = true;
					protection = 3;
				}
				break;
			case 3://�������
				protect();
				break;
			case 2: //��������
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
	GPIO_Init(PORT_LED, PIN_LED, GPIO_Mode_Out_PP_Low_Slow); // ���� ���������� Led PB_10
	GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_IT); // ���� 1-Wire �� iButton

	//�������� ����������
	EXTI->CR1 = (EXTI_Trigger_Rising << 6) //���������� �� ��������� ������ ��� ����� PB3 (1-Wire)
	| (EXTI_Trigger_Falling << 2) //���������� �� ������� ������ ��� ����� PB1 (������)
			| (EXTI_Trigger_Falling << 4); //���������� �� ������� ������ ��� ����� PB2 (������)

	GPIO_Init(GPIOB, GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_In_FL_IT); // PB1 PB2 �� ���������� (������)

	serial.init();
	intiTimerJ();
	initWakeup();
	//GPIO_Init(GPIOC, RFM_PIN_RESET, GPIO_Mode_Out_PP_High_Slow );
	GPIO_Init(GPIOC, RFM_PIN_RESET, GPIO_Mode_Out_OD_HiZ_Slow);
	//�������������� ����
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
	delayMs(50); //�����, ����� �������� ������ ����;
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

