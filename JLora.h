/*
 * JLora.h
 *
 *  Created on: 19 θώνÿ 2017 γ.
 *      Author: anisimov.e
 */

#ifndef SRC_JLORA_H_
#define SRC_JLORA_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm8l15x.h"
#include "JRfm95.h"

#define LORA_HEADER_LEN	7

class JLora
{
public:

	JLora();

	bool sendPayload(uint8_t type, const uint8_t *data, uint8_t len);
	bool waitAvailableTimeout(uint16_t timeout);
	void printRegOfRfm95();
	bool recive(uint8_t* buf, uint8_t* len);
	
	JRfm95 rfm95;

private:
	uint16_t numPack;
	uint8_t numChanel;
};

#ifdef __cplusplus
}
#endif

#endif /* SRC_JLORA_H_ */
