/*
 * timerJ.h
 *
 *  Created on: 27 февр. 2017 г.
 *      Author: juvf
 */

#ifndef TIMERJ_H_
#define TIMERJ_H_

#ifdef __cplusplus
extern "C"
{
#endif	
  
#include "stm8l15x.h"

void intiTimerJ();
void delayMs(uint32_t ms);
uint16_t millis();
void tim4Handler();

#ifdef __cplusplus
}
#endif

#endif /* TIMERJ_H_ */
