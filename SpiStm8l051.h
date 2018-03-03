#ifndef SPI_STM32L051_H
#define SPI_STM32L051_H

#include "stm8l15x.h"

#include "JSpi.h"
 #define ATOMIC_BLOCK_START { disableInterrupts();
 #define ATOMIC_BLOCK_END  enableInterrupts(); }

class SpiStm8l051 : public JSpi
{
public:
  SpiStm8l051();
  virtual void initial();
  virtual uint8_t read(uint8_t numReg) const;
  virtual void read(uint8_t numReg, uint8_t *buffer, uint8_t len) const;
  virtual void write(uint8_t numReg, uint8_t data) const;
  virtual void write(uint8_t numReg, const uint8_t *buffer, uint8_t len) const;
};

#endif
