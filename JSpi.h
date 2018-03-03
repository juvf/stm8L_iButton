#ifndef J_SPI_H
#define J_SPI_H

#include "stm8l15x.h"

#ifdef __cplusplus
extern "C"
{
#endif

class JSpi
{
public:
  JSpi();
  virtual void initial();
  virtual uint8_t read(uint8_t numReg) const;
  virtual void read(uint8_t numReg, uint8_t *buffer, uint8_t len) const;
  virtual void write(uint8_t numReg, uint8_t data) const;
  virtual void write(uint8_t numReg, const uint8_t *buffer, uint8_t len) const;
private:
  
};

#ifdef __cplusplus
}
#endif

#endif
