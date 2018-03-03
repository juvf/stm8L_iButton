#include "SpiStm8l051.h"

#define SPI_PIN_CS      GPIO_Pin_4
#define SPI_PIN_MOSI      GPIO_Pin_6
#define SPI_PIN_MISO      GPIO_Pin_7
#define SPI_PIN_SCK      GPIO_Pin_5

#define SPI_PIN_OUT     (GPIO_Pin_TypeDef)((uint8_t)SPI_PIN_MOSI | (uint8_t)SPI_PIN_SCK | (uint8_t)SPI_PIN_CS)

#define csOn()	GPIO_ResetBits(GPIOB, SPI_PIN_CS)
#define csOff()	GPIO_SetBits(GPIOB, SPI_PIN_CS)

#pragma optimize=none //без прагмы waitSpi(); не дожидается выполнениея
inline void waitSpi() {       while( (SPI1->SR & (uint8_t)SPI_FLAG_BSY));}

SpiStm8l051::SpiStm8l051()
{
}

void SpiStm8l051::initial()
{
GPIO_Init (GPIOB, SPI_PIN_MISO, GPIO_Mode_In_PU_No_IT);//GPIO_Mode_In_FL_No_IT); 
GPIO_Init (GPIOB, SPI_PIN_OUT, GPIO_Mode_Out_PP_High_Fast);//MOSI, CS, SCLK

/*SPI1->CR1 = SPI_FIRSTBIT_MSB | 
           SPI_BAUDRATEPRESCALER_8 |
           SPI_CLOCKPOLARITY_LOW | 
			SPI_CLOCKPOLARITY_LOW | 
			SPI_MODE_MASTER | 
                        SPI_CR1_SPE;*/
		  CLK->PCKENR1 |= CLK_PCKENR1_SPI1;
    /*      SPI_Init(SPI1,
				   SPI_FirstBit_MSB,
                   SPI_BaudRatePrescaler_2,
                   SPI_Mode_Master,
                   SPI_CPOL_Low,
                   SPI_CPHA_1Edge,
                   SPI_Direction_2Lines_FullDuplex,
                   SPI_NSS_Soft,
                   7);*/
		  SPI1->CR2 = SPI_CR2_SSM | SPI_CR2_SSI;
		  SPI1->CR1 = SPI_CR1_SPE | SPI_CR1_MSTR;
}

uint8_t SpiStm8l051::read(uint8_t numReg) const
{
           waitSpi();
		csOn();
		SPI1->DR = numReg & 0x7f;
                waitSpi();
                SPI1->DR = 0x00;
                waitSpi();
		csOff();
                uint8_t dataRx = SPI1->DR;
	return dataRx;
}

void SpiStm8l051::write(uint8_t numReg, uint8_t data) const
{
           waitSpi();
		csOn();
		SPI1->DR = numReg | 0x80;
                waitSpi();
                SPI1->DR = data;
                waitSpi();
		csOff();
}

void SpiStm8l051::read(uint8_t numReg, uint8_t *buffer, uint8_t len) const
{
	//ATOMIC_BLOCK_START
  waitSpi();
		csOn();
        SPI1->DR = numReg & ~0x80;
		waitSpi();
                do
                {
                  SPI1->DR = 0x00;
                  waitSpi();
                  *buffer++ = SPI1->DR;
                }while(--len);
		csOff();
		//ATOMIC_BLOCK_END
}

void SpiStm8l051::write(uint8_t numReg, const uint8_t *buffer,
		uint8_t len) const
{
//	ATOMIC_BLOCK_STAR
              waitSpi();
		csOn();
                SPI1->DR = numReg | 0x80;
waitSpi();
		
		do
                {
                  SPI1->DR = *buffer++;
                  waitSpi();
                }while(--len);
                
		csOff();
//		ATOMIC_BLOCK_END
}

