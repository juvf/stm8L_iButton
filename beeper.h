/*
 * beeper.h
 *
 *  Created on: 19 мая 2018 г.
 *      Author: anisimov.e
 */

#ifndef BEEPER_H_
#define BEEPER_H_

#include "stm8l15x.h"

void beeperInit();
void beepreRoutine();

void beepOn(uint16_t timePulse1, uint16_t timePulse2 = 0);
extern uint8_t curentPulse;

#endif /* BEEPER_H_ */
