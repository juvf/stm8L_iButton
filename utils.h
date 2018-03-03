#ifndef UTILS__H
#define UTILS__H
#include "stm8l15x.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
extern uint8_t timeForAlarm[2]; 
  
#define WORD_FROM_ARRAY(ARRAY) ((*ARRAY) | (*(ARRAY+1) << 8))
  
#define PER_SEC	5    
#define rtcWakeupCounter 4241 //1 минута
 // #define rtcWakeupCounter 4241/6 //10 секунд
 //#define rtcWakeupCounter (4241/15) // 4 секунды
 //#define rtcWakeupCounter ((4241 * PER_SEC) / 60) // 1 секунда
  
char *itoa(uint16_t number, char *destination, int base);
void initWakeup();
void wakeup();
void initHaly();
void sendReplay();
void memcpyN(uint8_t *dis, const uint8_t *source, uint8_t len);
  
#ifdef __cplusplus
}
#endif

#endif //UTILS__H