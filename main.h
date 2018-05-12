#ifndef MAIN_H
#define MAIN_H
#include "stm8l15x.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define EEPROM_ADR_MODUL	(DATA_EEPROM_BASE + 14)	//2 байта
#define EEPROM_ADR_SERVER	(DATA_EEPROM_BASE + 16)	//2 байта
#define EEPROM_LORA_FREQ	(DATA_EEPROM_BASE + 18)	//1 байт
#define EEPROM_LORA_MODE	(DATA_EEPROM_BASE + 19)	//1 байт

#define PORT_RELE GPIOA
#define PIN_RELE GPIO_Pin_2

#define PORT_LED GPIOB
#define PIN_LED GPIO_Pin_0
	
  
#define releOn()	(GPIOA->ODR |= PIN_RELE)
#define releOff()	(GPIOA->ODR &= ~PIN_RELE)
	
#define ledOn()	(PORT_LED->ODR |= PIN_LED)
#define ledOff()	(PORT_LED->ODR &= ~PIN_LED)

extern uint16_t timeToSleep;
extern uint16_t timeToSleepUart;

/*������� ��������� ������
 * 0 - ����� � ������
 * 1 - ���������� �� ������
 * 2 - ����� �� ������
 * 3 - �������
 * 4 - �������� ������, ������� ������, ����� �� ������
 */
extern uint8_t protection;
extern uint16_t timerProt;
extern uint16_t protectPause;

extern volatile bool isSendLora; //����� ������� ����� ��������� �� ������
extern volatile bool isReadingKey; //������� ������ �� ������ ������ �����
extern volatile bool keyIsReaded; //���� ��������
extern uint16_t timerReadKye; //������ ��� ������ ������ �����

extern uint16_t numPack; //����� ������. ��� ������ �� ��������� �������� �� ����� � ��


void initPeref();
void checkSleep();

#ifdef __cplusplus
}
#endif


#endif
