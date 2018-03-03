/*
 * Serial.h
 *
 *  Created on: 28 февр. 2017 г.
 *      Author: juvf
 */

#ifndef SERIAL_H_
#define SERIAL_H_
//#include <stdint.h>
#include "stm8l15x.h"
   
#ifdef __cplusplus


#define DEC	10
#define HEX 16
#define MAX_SIZE	64
#define BAUDRATE   19200

extern uint8_t buffer42[MAX_SIZE];

class Serial
{
public:
	Serial();
	void init();
	void print(const char *text, bool printLn = false);
	void println(unsigned int val, int s = DEC);
	void print(int val, int s = DEC);
	void flush();
	void reciveByte(uint8_t byte);
	void setDebugMode(uint8_t mode);
	bool isGetCommand() const;
	void clearIsResive();
	void sentBytes(const uint8_t *array, uint8_t len);

	char buffer[64];
	uint8_t countTx;
	uint8_t countBuf;
	bool isTransmoin;
private:
	uint8_t count42;
	uint8_t debugMode;
	bool isResive;
    void startTransmit();
	bool add2Buf(const char *array, int len);
};

extern Serial serial;

extern "C"
{
#endif

void handlerUsartTxD();
void handlerUsartRxD();
void isrSerial();

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H_ */
