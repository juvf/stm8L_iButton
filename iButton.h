/*
 * iButton.h
 *
 *  Created on: 3 ���. 2018 �.
 *      Author: juvf
 */

#ifndef IBUTTON_H_
#define IBUTTON_H_

#include "stm8l15x.h"


void checkIButton();
void OWWriteByte(uint8_t byte);
uint8_t OWReadByte();


#endif /* IBUTTON_H_ */
