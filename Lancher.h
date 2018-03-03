/*
 * Lancher.h
 *
 *  Created on: 06 марта 2017 г.
 *      Author: juvf
 */

#ifndef LANCHER_H_
#define LANCHER_H_
#include "stm8l15x.h"
#include "eeprom.h"

#ifdef __cplusplus
    
extern uint8_t writeIml[4];
extern bool wasCheckedImpl;
extern bool wasAlarm;

class Lancher
{
public:

	Lancher();
	~Lancher();
	void up(uint8_t port);
	static void init();
private:
	void startTimer(uint8_t port);
};

extern Lancher lancher;
void setAlarm();
void restoreFrom();
void preparationToStop();
void LancherCallback();
uint8_t getInput();
   
 extern "C" {
#endif

void isrImpl(uint8_t port);

#ifdef __cplusplus
 }
#endif

#endif /* LANCHER_H_ */
