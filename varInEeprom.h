#ifndef VAR_IN_EEPROM__H
#define VAR_IN_EEPROM__H
#include "stm8l15x.h"
#include "eeprom.h"
#include "varInEeprom.h"

typedef struct
{
	uint16_t countStarts;
	uint16_t periodOprosa; //период, через который сигнализация будет передавать сигналы keepalive
	uint16_t addressOfModul;
	uint16_t addressOfServer;
	uint8_t numChanel; //номер частотного канала
	uint8_t powerTx;
	uint8_t crSf; //10 в мл тетраде sf, в старшей cr
	uint8_t bw; //11 старший бит - 0=sx1272, 1=1276
	uint32_t freq; //12-15
	float coeff; //16-19
	uint8_t isProgramm;
	uint8_t reserv;
} Eeprom;

extern __no_init uint16_t periodOprosa;

extern __eeprom __no_init Eeprom config;
extern uint32_t countImp[4];

#endif
