/*
 * iButton.h
 *
 *  Created on: 3 мар. 2018 г.
 *      Author: juvf
 */

#ifndef IBUTTON_H_
#define IBUTTON_H_

#include "stm8l15x.h"


void checkIButton();
void OWWriteByte(uint8_t byte);
void OWReadKey();
uint8_t iButtonCrc();
bool readKey();
bool keyIsGood();

#endif /* IBUTTON_H_ */
