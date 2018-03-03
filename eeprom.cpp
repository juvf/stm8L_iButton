#include "eeprom.h"

// The user must implement the three extern-declared functions below
// in order for the compiler to be able to automatically write to the
// EEPROM memory when __eeprom variables are assigned to.

#ifdef __cplusplus
extern "C" {
#endif
  
#pragma language=extended
 
// Wait for the last data EEPROM operation to finish.  Return 0 if the
// operation failed, otherwise non-zero.  You may want to handle
// errors here, since the utility functions below simply ignore
// errors, aborting multi-write operations early.
int __eeprom_wait_for_last_operation(void)
{
  if(FLASH->IAPSR & FLASH_IAPSR_WR_PG_DIS) return 0;
  while((FLASH->IAPSR & FLASH_IAPSR_HVOFF) == 0); // Wait until operation completion
  return 1;
}

// Write one byte to the data EEPROM memory.
void __eeprom_program_byte(uint8_t __near * dst, uint8_t v)
{
  *dst = v;
}

// Write one 4-byte long word to the data EEPROM memory.  The address must be 4-byte aligned.
void __eeprom_program_long(uint8_t __near * dst, uint32_t v)
{
  FLASH->CR2 |= FLASH_CR2_WPRG;
  *(dst++)       = *((uint8_t*)(&v));   
  *(dst++) = *((uint8_t*)(&v) + 1);
  *(dst++) = *((uint8_t*)(&v) + 2);
  *dst = *((uint8_t*)(&v) + 3);  
}

#pragma language=default

#ifdef __cplusplus
}
#endif
