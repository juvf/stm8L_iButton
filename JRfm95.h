/*
 * JRfm95.h
 *
 *  Created on: 19 θώνÿ 2017 γ.
 *      Author: anisimov.e
 */
#ifndef APPLICATION_USER_JRFM95_H_
#define APPLICATION_USER_JRFM95_H_

#ifdef __cplusplus
extern "C"
{
#endif
  
#include "stm8l15x.h"
	

#include "JSpi.h"
#include "Rfm95Header.h"
  
#define RFM_PIN_RESET    GPIO_Pin_4
#define RFM_PIN_INT      GPIO_Pin_0

class JRfm95
{
public:
	JRfm95();
	bool initial();

	void setPreambleLength(uint16_t priambule = 8);

	void setMode(RHMode mode);
	bool setFrequency(float centre);
	bool setFrequency(uint8_t chanel);
	//bool send(const uint8_t* data, uint8_t len);
	bool isChannelActive();
	bool isSending();
	bool reciveWithTimeout(uint8_t *buff, uint8_t *len, uint16_t timeout);  
	uint8_t getReg(uint8_t numReg) const;

	void handleInterrupt();
	bool available();
	bool waitAvailableTimeout(uint16_t timeout);
	bool recive(uint8_t* buf, uint8_t* len);
	
	static void isr0();
	void reset();
	void setFrequency(uint32_t centre);

	uint8_t waitCad();
	uint8_t startCad();
	void startSend(uint8_t *data, uint8_t len);
	uint8_t waitSend();
	uint8_t waitAck();

private:
	JSpi *spi;
	static JRfm95 *rfm;

	// The current transport operating mode
	volatile RHMode _mode;
	// True if we are using the HF port (779.0 MHz and above)
//	bool _usingHFport;

	// Channel activity detected
	volatile bool _cad;

	// Count of the number of bad messages (eg bad checksum etc) received
	volatile uint16_t _rxBad;

	// Count of the number of successfully transmitted messaged
	volatile uint16_t _rxGood;

	// Count of the number of bad messages (correct checksum etc) received
	volatile uint16_t _txGood;

	// Last measured SNR, dB
	int8_t _lastSNR;

	// Number of octets in the buffer
	volatile uint8_t _bufLen;

	// The receiver/transmitter buffer
	uint8_t _buf[RH_RF95_FIFO_SIZE];

	// The value of the last received RSSI value, in some transport specific units
	volatile int8_t _lastRssi;
	
	// True when there is a valid message in the buffer
    volatile bool       _rxBufValid;
    volatile uint16_t tempTime1;
    volatile uint16_t tempTime2;
};

void interruptDio();

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_JRFM95_H_ */
