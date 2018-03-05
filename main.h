#ifndef MAIN_H
#define MAIN_H
#include "stm8l15x.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define EEPROM_ADR_MODUL	(DATA_EEPROM_BASE + 14)	//2 Р±Р°Р№С‚Р°
#define EEPROM_ADR_SERVER	(DATA_EEPROM_BASE + 16)	//2 Р±Р°Р№С‚Р°
#define EEPROM_LORA_FREQ	(DATA_EEPROM_BASE + 18)	//1 Р±Р°Р№С‚
#define EEPROM_LORA_MODE	(DATA_EEPROM_BASE + 19)	//1 Р±Р°Р№С‚

#define PORT_RELE GPIOA
#define PIN_RELE GPIO_Pin_2

#define PORT_LED GPIOB
#define PIN_LED GPIO_Pin_0
	
  
#define releOn()	(GPIOA->ODR |= PIN_RELE)
#define releOff()	(GPIOA->ODR &= ~PIN_RELE)
	
#define ledOn()	(PORT_LED->ODR |= PIN_LED)
#define ledOff()	(PORT_LED->ODR &= ~PIN_LED)

extern uint8_t timeToSleep;
extern uint16_t timeToSleepUart;

/*Текущее состояние
 * 0 - снято с охраны
 * 1 - постановка на охрану
 * 2 - сняте с охраны
 * 3 - тревога
 * 4 - стоит на охране
 */
extern uint8_t protection;
extern uint16_t timerProt;
extern uint16_t protectPause;

/*
 * 0 - ни чего не слать
 * 1 - постановка на охрану
 * 2 - взятие под охрану
 * 3 - сработка датчика (ожидание снятия с охраны)
 * 4 - снятие с охраны
 * 5 - тревога
 * 6 - включение, т.е. первая отправка пакета
 * 7 - keepalive
 */
extern uint8_t sendProtect; //пакет который нужно отправить по охране

void initPeref();
void checkSleep();

#ifdef __cplusplus
}
#endif


#endif
