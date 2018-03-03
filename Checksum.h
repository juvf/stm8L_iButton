#ifndef CHECKSUM__H
#define CHECKSUM__H

#include "stm8l15x.h"

class Checksum
{
public:
/*
  Name  : CRC-16
  Poly  : 0x8005   x^16 + x^15 + x^2 + 1
  Init  : 0x0000
  Revert: true
  XorOut: 0x0000
  Check : 0x4B37 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
	static unsigned short crc16(const unsigned char * pcBlock, unsigned short len);
    static void addCrc16(unsigned char * pcBlock, unsigned short len);
    static void clearCrc();
    static uint16_t calcCrc(uint8_t byte);

private:
    static uint16_t crcIn;
     
};
unsigned int crc16_byte(unsigned int crc, unsigned int data);

#endif /*CHECKSUM_H_*/
