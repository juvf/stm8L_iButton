#ifndef RF_CLIENT
#define RF_CLIENT

#include "JLora.h"

#ifdef __cplusplus
extern "C"
{
#endif
 

#define MAX_PAYLOAD 251
  
 typedef struct PackJ
{
	uint16_t dst;
    uint8_t type;
	uint16_t src;
	uint16_t packnum;
	uint8_t payloadlength;
	uint8_t data[MAX_PAYLOAD];
	uint8_t retry;
}PackJ;

extern JLora jLora;

void setupRf95();
bool rfClientLoop();
void setNumChanel(uint8_t val);
void waitResive();
void parserLoraProtocol(uint8_t *buffer, uint8_t len);
void getPackJ(PackJ *pack, uint8_t *buffer);

extern volatile bool isSendLora; //если true, то пора отправлять данные на сервер

#ifdef __cplusplus
}
#endif

#endif