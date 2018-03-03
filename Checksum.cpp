#include "Checksum.h"
uint16_t Checksum::crcIn;

/* CRC algorithm */ 
#pragma optimize=none
unsigned int crc16_byte(unsigned int crc, unsigned int data)
{
	const unsigned int Poly16=0xA001;
	unsigned int LSB;
	crc = ((crc^data) | 0xFF00) & (crc | 0x00FF);
	for (uint8_t i=0; i<8; i++) 
	{
		LSB=(crc & 0x0001);
		crc >>= 1;
		if(LSB)
			crc=crc^Poly16;
	}
	return crc;
}
   
unsigned short Checksum::crc16(const unsigned char *pcBlock, unsigned short len)
{
	unsigned int crc = 0xFFFF;
	for (int i = 0; i < len; i++) {
		crc = crc16_byte(crc, pcBlock[i] );
	}
	return crc;
}

//len - длинна без црц
void Checksum::addCrc16(unsigned char * pcBlock, unsigned short len)
{
    unsigned short crc = crc16(pcBlock, len);
    pcBlock[len++] = crc & 0xff;
    pcBlock[len] = (crc >> 8) & 0xff;
}

void Checksum::clearCrc()
{
	crcIn = 0xFFFF;
}
uint16_t Checksum::calcCrc(uint8_t byte)
{
  crcIn = crc16_byte(crcIn, byte);
	return crcIn;
}

