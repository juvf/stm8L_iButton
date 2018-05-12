#include "JRfm95.h"
#include "Rfm95Header.h"
#include "timerJ.h"
#include "SpiStm8l051.h"
#include "Serial.h"
#include "varInEeprom.h"

#include <string.h>

#define delay(ms)	delayMs(ms)
#define CAD_TIMEOUT	10000
#define TX_TIMEOUT	3000
#define RX_TIMEOUT	3000

SpiStm8l051 spiStm8l;
uint8_t ver;

JRfm95* JRfm95::rfm = 0;

JRfm95::JRfm95()
{
	_rxBad = 0;
	rfm = 0;
	_rxBufValid = false;
}

void JRfm95::reset()
{
	GPIO_ResetBits(GPIOC, RFM_PIN_RESET);
	delay(2);
	GPIO_SetBits(GPIOC, RFM_PIN_RESET);
}
#pragma optimize=none
bool JRfm95::initial()
{
	rfm = this;
	spi = &spiStm8l;
	spi->initial();
	this->reset();
	ver = spi->read(RH_RF95_REG_42_VERSION);
	bool isSx1272 = (config.bw & 0x80) == 0;

	if(!(((ver == 0x22) && isSx1272) || ((ver == 0x12) && !isSx1272)))
	{
		serial.print("Incorrect ver of module Lora\n\rver= 0x", false);
		serial.println(ver, HEX);
		return false;
	}

	// Set sleep mode, so we can also set LORA mode:
	spi->write(RH_RF95_REG_01_OP_MODE,
	RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);
	delay(10); // Wait for sleep mode to take over from say, CAD
	// Check we are in sleep mode, with LORA set
	if(spi->read(RH_RF95_REG_01_OP_MODE)
			!= (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE))
		return false; // No device present?

	spi->write(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
	spi->write(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

	setMode(RHModeIdle);

	uint8_t reg_1e, reg_1d;
	if(isSx1272)
	{
		uint8_t bw = config.bw << 6;
		uint8_t cr = (config.crSf >> 1) & (7 << 3);
		reg_1d = cr | bw;	//0x08;
		reg_1e = (config.crSf << 4);	//0x74;
		spi->write(RH_RF95_REG_09_PA_CONFIG, config.powerTx + 1);
	}
	else
	{	//Sx1276
		uint8_t bw = config.bw << 4;
		uint8_t cr = (config.crSf >> 3) & (7 << 1);
		reg_1d = bw | cr; //0x72;	//0x78;
		reg_1e = (config.crSf << 4); //0x70;	//0xC0;

		uint8_t power = config.powerTx;
		uint8_t mA;
		if(power > 17)
		{
			//150 mA
			mA = 0x15 | 0x20;
		}
		else if(power > 14)
			mA = 0x10 | 0x20; //130mA
		else
			mA = 0xB | 0x20; //100mA
		spi->write(RH_RF95_REG_0B_OCP, mA);

		if(power > 17)
		{
			spi->write(RH_RF95_REG_09_PA_CONFIG, RH_RF95_PA_SELECT | 0x0f);
			spi->write(RH_RF95_REG_4D_PA_DAC, RH_RF95_PA_DAC_ENABLE | 0x80);
			power = 17;
		}
		else
		{
			spi->write(RH_RF95_REG_09_PA_CONFIG,
			RH_RF95_PA_SELECT | (power - 2));
			spi->write(RH_RF95_REG_4D_PA_DAC, 0x84);
		}
	}
	spi->write(RH_RF95_REG_1D_MODEM_CONFIG1, reg_1d);
	spi->write(RH_RF95_REG_1E_MODEM_CONFIG2, reg_1e);

	spi->write(RH_RF95_REG_41_DIO_MAPPING2, 0x50);
	
	spi->write(RH_RF95_REG_39_SYNC_WORD, 0x12); //    Value 0x34 is reserved for LoRaWAN networks

	setPreambleLength(8); // Default is 8
	return true;
}

void JRfm95::isr0()
{
	if(rfm)
		rfm->handleInterrupt();
}

bool JRfm95::available()
{
	if(_mode == RHModeTx)
		return false;
	setMode(RHModeRx);
//return _bufLen > 0; // Will be set by the interrupt handler when a good message is received
	return _rxBufValid;
}

// Blocks until a valid message is received or timeout expires
// Return true if there is a message available
// Works correctly even on millis() rollover
bool JRfm95::waitAvailableTimeout(uint16_t timeout)
{
	unsigned long starttime = millis();
	while((millis() - starttime) < timeout)
	{
		if(this->available())
			return true;
	}
	return false;
}

void JRfm95::setPreambleLength(uint16_t bytes)
{
	spi->write(RH_RF95_REG_20_PREAMBLE_MSB, bytes >> 8);
	spi->write(RH_RF95_REG_21_PREAMBLE_LSB, bytes & 0xff);
}

/// Sets the transmitter and receiver
/// centre frequency.
/// \param[in] centre Frequency in MHz. 137.0 to 1020.0. Caution: RFM95/96/97/98 comes in several
/// different frequency ranges, and setting a frequency outside that range of your radio will probably not work
/// \return true if the selected frquency centre is within range
bool JRfm95::setFrequency(float centre)
{
// Frf = FRF / FSTEP
	/*	uint32_t frf = (uint32_t)((centre * 1000000.0)) / RH_RF95_FSTEP);
	 spi->write(RH_RF95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
	 spi->write(RH_RF95_REG_07_FRF_MID, (frf >> 8) & 0xff);
	 spi->write(RH_RF95_REG_08_FRF_LSB, frf & 0xff);
	 _usingHFport = (centre >= 779000000);*/
	return true;
}

void JRfm95::setFrequency(uint32_t centre)
{
	spi->write(RH_RF95_REG_06_FRF_MSB, (centre >> 16));
	spi->write(RH_RF95_REG_07_FRF_MID, (centre >> 8));
	spi->write(RH_RF95_REG_08_FRF_LSB, centre);
}

bool JRfm95::setFrequency(uint8_t chanel)
{
	uint32_t frf;
	switch(chanel)
	{
		case 9:
			frf = 14167244; //864.7 MHz
			break;
		case 7: //864100000
			frf = 14157414;
			break;
		case 8: //864400000
			frf = 14162329;
			break;
		case 10: //865200000
			frf = 14175436;
			break;
		default:
			return false;
	}
	spi->write(RH_RF95_REG_06_FRF_MSB, (frf >> 16));
	spi->write(RH_RF95_REG_07_FRF_MID, (frf >> 8));
	spi->write(RH_RF95_REG_08_FRF_LSB, frf);

//_usingHFport = 1;//(centre >= 779000000);
	return true;
}

void JRfm95::setMode(RHMode mode)
{
	switch((int)mode)
	{
		case RHModeIdle:
			if(_mode != RHModeIdle)
			{
				spi->write(RH_RF95_REG_01_OP_MODE, LORA_STANDBY_MODE);
				_mode = RHModeIdle;
			}
			break;
		case RHModeRx:
			if(_mode != RHModeRx)
			{
				spi->write(RH_RF95_REG_01_OP_MODE, LORA_RX_MODE); //RH_RF95_MODE_RXCONTINUOUS);
				//spi->write(RH_RF95_REG_40_DIO_MAPPING1, 0x00); // Interrupt on RxDone
				_mode = RHModeRx;
			}
			break;
		case RHModeTx:
			if(_mode != RHModeTx)
			{
				spi->write(RH_RF95_REG_01_OP_MODE, LORA_TX_MODE);
				uint8_t value;
				do
				{
					value = spi->read(RH_RF95_REG_12_IRQ_FLAGS);
				} while((value & RH_RF95_TX_DONE) == 0);
				spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags
				_mode = RHModeIdle;
			}
			break;
		case RHModeSleep:
			if(_mode != RHModeSleep)
			{
				spi->write(RH_RF95_REG_01_OP_MODE, LORA_SLEEP_MODE);
				_mode = RHModeSleep;
			}
			break;
	}
}

bool JRfm95::isSending()
{
	return _mode == RHModeTx;
}

bool JRfm95::isChannelActive()
{
// Set mode RHModeCad
	if(_mode != RHModeCad)
	{
		spi->write(RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_CAD);
		_mode = RHModeCad;
	}
	static uint8_t value;
	do
	{
		value = spi->read(RH_RF95_REG_12_IRQ_FLAGS);
		delay(20);
	} while((value & RH_RF95_CAD_DONE) == 0);
	_cad = (bool)(value & RH_RF95_CAD_DETECTED);
	spi->write(RH_RF95_REG_12_IRQ_FLAGS,
	RH_RF95_CAD_DONE | RH_RF95_CAD_DETECTED); // Clear CAD IRQ flags
	setMode(RHModeIdle);
	return _cad;
}
void interruptDio()
{
	JRfm95::isr0();
}

void JRfm95::handleInterrupt()
{
// Read the interrupt register
	uint8_t irq_flags = spi->read(RH_RF95_REG_12_IRQ_FLAGS);
	if(irq_flags == 0)
		return;
	if(_mode == RHModeRx
			&& irq_flags & (RH_RF95_RX_TIMEOUT | RH_RF95_PAYLOAD_CRC_ERROR))
	{
		_rxBad++;
	}
	else if(_mode == RHModeRx && irq_flags & RH_RF95_RX_DONE)
	{
		// Have received a packet
		uint8_t len = spi->read(RH_RF95_REG_13_RX_NB_BYTES);

		// Reset the fifo read ptr to the beginning of the packet
		spi->write(RH_RF95_REG_0D_FIFO_ADDR_PTR,
				spi->read(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR));
		spi->read(RH_RF95_REG_00_FIFO, _buf, len);
		_bufLen = len;
		spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff);		// Clear all IRQ flags

		// Remember the last signal to noise ratio, LORA mode
		// Per page 111, SX1276/77/78/79 datasheet
		_lastSNR = (int8_t)spi->read(RH_RF95_REG_19_PKT_SNR_VALUE) / 4;

		// Remember the RSSI of this packet, LORA mode
		// this is according to the doc, but is it really correct?
		// weakest receiveable signals are reported RSSI at about -66
		_lastRssi = spi->read(RH_RF95_REG_1A_PKT_RSSI_VALUE);
		// Adjust the RSSI, datasheet page 87
		if(_lastSNR < 0)
			_lastRssi = _lastRssi + _lastSNR;
		else
			_lastRssi = (int)_lastRssi * 16 / 15;
		//if(_usingHFport)
		_lastRssi -= 157;
		//else
		//	_lastRssi -= 164;

		_rxGood++;
	}
	else if(_mode == RHModeTx && irq_flags & RH_RF95_TX_DONE)
	{
		_txGood++;
		setMode(RHModeIdle);
	}
	else if(_mode == RHModeCad && irq_flags & RH_RF95_CAD_DONE)
	{
		_cad = irq_flags & RH_RF95_CAD_DETECTED;
		setMode(RHModeIdle);
	}

	spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags
}

uint8_t JRfm95::getReg(uint8_t numReg) const
{
	return spi->read(numReg);
}

/*
 * получает в аргументах len - максимальное кол-во принимаемых байт(размер буффера buff)
 * timeout в мс
 */
#pragma optimize=none
bool JRfm95::reciveWithTimeout(uint8_t *buff, uint8_t *len, uint16_t timeout)
{
	spi->write(RH_RF95_REG_01_OP_MODE, LORA_RX_MODE);
	uint16_t t = millis();
	do
	{
		if(millis() - t > timeout)
		{
			spi->write(RH_RF95_REG_01_OP_MODE, LORA_STANDBY_MODE);
			return false;
		}
		delay(millis() % 10 * 10);
	} while((spi->read(RH_RF95_REG_12_IRQ_FLAGS) & RH_RF95_RX_DONE) == 0);
	spi->write(RH_RF95_REG_01_OP_MODE, LORA_STANDBY_MODE);
	ATOMIC_BLOCK_START
		;
		uint8_t localLen = spi->read(RH_RF95_REG_13_RX_NB_BYTES);

		*len = localLen > *len ? *len : localLen;
		spi->write(RH_RF95_REG_0D_FIFO_ADDR_PTR,
				spi->read(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR)); // Reset the fifo read ptr to the beginning of the packet
		spi->read(RH_RF95_REG_00_FIFO, buff, *len);

		spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags

		_lastSNR = (int8_t)spi->read(RH_RF95_REG_19_PKT_SNR_VALUE) / 4;

		// Remember the RSSI of this packet, LORA mode
		// this is according to the doc, but is it really correct?
		// weakest receiveable signals are reported RSSI at about -66
		_lastRssi = spi->read(RH_RF95_REG_1A_PKT_RSSI_VALUE);
		// Adjust the RSSI, datasheet page 87
		if(_lastSNR < 0)
			_lastRssi = _lastRssi + _lastSNR;
		else
			_lastRssi = (int)_lastRssi * 16 / 15;
		//if(_usingHFport)
		_lastRssi -= 157;

		ATOMIC_BLOCK_END
	;
	return true;
}

uint8_t JRfm95::startCad()
{
// Wait for any channel activity to finish or timeout
// Sophisticated DCF function...
// DCF : BackoffTime = random() x aSlotTime
// 100 - 1000 ms
// 10 sec timeout
	if(_mode == RHModeSleep)
		setMode(RHModeIdle);
	tempTime1 = millis();
	tempTime2 = tempTime1;
	return isChannelActive() ? 2 : 1;
}

uint8_t JRfm95::waitCad()
{
	static uint8_t paus = 50;
	if((millis() - tempTime1) > CAD_TIMEOUT)
		return 4;
	if((millis() - tempTime2) > paus)
	{
		if(!isChannelActive())		//проверим, чтоб не было активности в канале
			return 2;
		else
		{
			tempTime2 = millis();
			paus = tempTime2 % 80;
		}
	}
	return 1;
}

void JRfm95::startSend(uint8_t *data, uint8_t len)
{
	if(_mode == RHModeSleep)
		setMode(RHModeIdle);

	// Position at the beginning of the FIFO
	spi->write(RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);
	for(uint8_t i = 0; i < len; i++)
		spi->write(RH_RF95_REG_00_FIFO, data[i]);

	spi->write(RH_RF95_REG_22_PAYLOAD_LENGTH, len);
	spi->write(RH_RF95_REG_01_OP_MODE, LORA_TX_MODE);
	_mode = RHModeTx;
	tempTime1 = millis();
}

uint8_t JRfm95::waitSend()
{
	if((millis() - tempTime1) > TX_TIMEOUT)
	{
		setMode(RHModeIdle);
		return 5;
	}
	serial.print("mode ", false);
	serial.println(_mode);

	uint8_t value = spi->read(RH_RF95_REG_12_IRQ_FLAGS);
	if((value & RH_RF95_TX_DONE) != 0)
	{
		serial.print("Stop send.", true);
		spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags
		setMode(RHModeIdle);
		setMode(RHModeRx);
		tempTime1 = millis();
		return 6;
	}
	return 3;
}

uint8_t JRfm95::waitAck(uint8_t *array)
{
	if((millis() - tempTime1) > RX_TIMEOUT)
	{
		setMode(RHModeIdle);
		return 5;
	}
	
	uint8_t value = spi->read(RH_RF95_REG_12_IRQ_FLAGS);
	if(value & RH_RF95_RX_DONE)
	{
		setMode(RHModeIdle);
		uint8_t localLen = spi->read(RH_RF95_REG_13_RX_NB_BYTES);

		if(localLen != 9)
			return 7;//гавно приняли
		spi->write(RH_RF95_REG_0D_FIFO_ADDR_PTR,
				spi->read(RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR)); // Reset the fifo read ptr to the beginning of the packet
		spi->read(RH_RF95_REG_00_FIFO, array, 9);

		spi->write(RH_RF95_REG_12_IRQ_FLAGS, 0xff); // Clear all IRQ flags

		_lastSNR = (int8_t)spi->read(RH_RF95_REG_19_PKT_SNR_VALUE) / 4;

		// Remember the RSSI of this packet, LORA mode
		// this is according to the doc, but is it really correct?
		// weakest receiveable signals are reported RSSI at about -66
		_lastRssi = spi->read(RH_RF95_REG_1A_PKT_RSSI_VALUE);
		// Adjust the RSSI, datasheet page 87
		if(_lastSNR < 0)
			_lastRssi = _lastRssi + _lastSNR;
		else
			_lastRssi = (int)_lastRssi * 16 / 15;
		//if(_usingHFport)
		_lastRssi -= 157;
		return 8;
	}
	return 6;
}

