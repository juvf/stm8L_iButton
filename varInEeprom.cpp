#include "varInEeprom.h"
/*
namespace Eeprom
{
#pragma location = 0x1000
__eeprom __no_init uint16_t *countStarts =(uint16_t *)0x1000; //0,1
__eeprom __no_init uint16_t *periodOprosa=(uint16_t *)0x1002; //2,3период просыпания с сбрасывания информации на серверв минутах.
#pragma data_alignment=4
__eeprom __no_init uint16_t *addressOfModul=(uint16_t *)0x1004;//4,5
__eeprom __no_init uint16_t *addressOfServer=(uint16_t *)0x1006;//6,7
__eeprom __no_init uint8_t *numChanel=(uint8_t *)0x1008;//8
__eeprom __no_init uint8_t *powerTx=(uint8_t *)0x1009;//9

__eeprom __no_init uint8_t *crSf=(uint8_t *)0x100a;//10 в мл тетраде sf, в старшей cr
__eeprom __no_init uint8_t *bw=(uint8_t *)0x100b;//11 старший бит - 0=sx1272, 1=1276
#pragma location = 0x100C
__eeprom __no_init uint32_t freq;//12-15

__eeprom __no_init float *coeff=(float *)0x1010;//16-19 коефф во флоат, для коррекции частьты. В коде он не участвует. его тут храит modConf
}*/

__eeprom __no_init Eeprom config;
__no_init uint16_t periodOprosa;

uint32_t countImp[4] = {0,0,0,0};
