#ifndef FLASH_EEPROM_H
#define FLASH_EEPROM_H
#include "stm8l15x.h"
#include "stm8l15x_flash.h"

#define FLASH_RASS_KEY1    ((uint8_t)0x56)        // First RASS key
#define FLASH_RASS_KEY2    ((uint8_t)0xAE)        // Second RASS key

//Unlock EEPROM for write
inline void EEPROM_Unlock(void)
{
  FLASH->DUKR = FLASH_RASS_KEY2;
  FLASH->DUKR = FLASH_RASS_KEY1;
}

//Lock EEPROM
inline void EEPROM_Lock(void)
{
  FLASH->IAPSR &= (uint8_t)(uint8_t)FLASH_MemType_Data;
//  FLASH->IAPSR &= (uint8_t)FLASH_MEMTYPE_DATA;
}

#endif //FLASH_EEPROM_H
