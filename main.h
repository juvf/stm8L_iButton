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

extern uint16_t timeToSleep;
extern uint16_t timeToSleepUart;

/*Текущее состояние охраны
 * 0 - снято с охраны
 * 1 - постановка на охрану
 * 2 - стоит на охране
 * 3 - тревога
 * 4 - сработал датчик, запущен таймер, стоит на охране
 */
extern uint8_t protection;
extern uint16_t timerProt;
extern uint16_t protectPause;

extern volatile bool isSendLora; //пакет который нужно отправить по охране
extern volatile bool isReadingKey; //получен запрос на чтение нового ключа
extern volatile bool keyIsReaded; //ключ прочитан
extern uint16_t timerReadKye; //таймер для чтения нового ключа

extern uint16_t numPack; //номер пакета. Для защиты от повторных отправок из шлюза в БД


void initPeref();
void checkSleep();

#ifdef __cplusplus
}
#endif


#endif
