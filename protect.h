/*
 * protect.h
 *
 *  Created on: 10 мар. 2018 г.
 *      Author: juvf
 */

#ifndef PROTECT_H_
#define PROTECT_H_
#include "stm8l15x.h"

#ifdef __cplusplus
extern "C"
{
#endif

void callBackExti();
void protect();
uint8_t getProtect();
void clearProtect();

#ifdef __cplusplus
}
#endif



#endif /* PROTECT_H_ */
