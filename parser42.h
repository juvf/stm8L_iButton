/*
 * parser42.h
 *
 *  Created on: 8 мая 2017 г.
 *      Author: anisimov.e
 */

#ifndef APPLICATION_USER_PARSER42_H_
#define APPLICATION_USER_PARSER42_H_

#include "stm8l15x.h"

typedef enum
{
	L_Echo = 1, L_SetPeriod, //2
	L_SetThisAddress,
	L_SetServerAddress,
	L_SetImplCounter, //5
	L_SetLoraMode,
	L_SetFreqChanel, //7
	L_GetPeriod,
	L_GetThisAddress,
	L_GetServerAddress,
	L_GetImplCounter,
	L_GetLoraMode,
	L_GetFreqChanel,// = 13
	L_WriteMem,
	L_ReadMem//15

} LoraCommand;

void parser42(uint8_t *buffer);
void setLoraFreq(uint8_t numChannel);

#endif /* APPLICATION_USER_PARSER42_H_ */
