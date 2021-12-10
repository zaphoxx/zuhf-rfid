/* Title:       ZUHF_CC1101
 * Description: x
 * Author:      Zaphoxx (Manfred Heinz)
 * Version:     1
 * License:     x
 */


//#define _PLATFORM _SAMX

#ifndef CC1101_h
#define CC1101_h

#include "Arduino.h"
#include <SPI.h>

//SPI PINS

#ifdef SAMX
  #define SCK_PIN 76
  #define MISO_PIN 74
  #define MOSI_PIN 75

  // PINS FOR TX MODULE
  #define RF_PIN 52
  #define GDO0_PIN 24 // PORTA 15
  #define GDO2_PIN 26 // PORTD 1
  #define GDRX_PIN 28
  #define GDO0_PIO 15
  #define GDO2_PIO 1
  #define GDRX_PIO 3

  #define GDO0_STATE (((PIOA->PIO_PDSR) & (1 << GDO0_PIO)) >> GDO0_PIO)
  #define GDRX_STATE (((PIOD->PIO_PDSR) & (1 << GDRX_PIO)) >> GDRX_PIO)
  #define GDO2_STATE (((PIOD->PIO_PDSR) & (1 << GDO2_PIO)) >> GDO2_PIO)

  #define GDO0_OUT (PIOA->PIO_OER = (PIOA->PIO_OER | (1 << GDO0_PIO)))
  #define GDO0_IN (PIOA->PIO_ODR = (PIOA->PIO_ODR | ((1 << GDO0_PIO))))

  #define GDO0_HIGH (PIOA->PIO_ODSR = (PIOA->PIO_ODSR | (1 << GDO0_PIO)))
  #define GDO0_LOW (PIOA->PIO_ODSR = (PIOA->PIO_ODSR & (~(1 << GDO0_PIO))))

  #define GDTX_PIN GDO0_PIN
  #define GDTX_STATE GDO0_STATE
  #define GDTX_HIGH GDO0_HIGH
  #define GDTX_LOW GDO0_LOW

#else
  #define SCK_PIN 13
  #define MISO_PIN 12
  #define MOSI_PIN 11
  #define SS_PIN 10
  
  #define GDO0_PIN 2
  #define GDO2_PIN 3
  
  #define GDO0_STATE (((PIND) & (1 << GDO0_PIN)) >> GDO0_PIN)
  #define GDO2_STATE (((PIND) & (1 << GDO2_PIN)) >> GDO2_PIN)
  
  #define GDO0_HIGH (PORTD = ((PORTD) | (1 << GDO0_PIN))) 
  #define GDO0_LOW (PORTD = ((PORTD) & (~(1 << GDO0_PIN))))

  #define GDO2_HIGH (PORTD = ((PORTD) | (1 << GDO2_PIN)))
  #define GDO2_LOW (PORTD = ((PORTD) & (~(1 << GDO2_PIN))))
#endif

#define SERIAL_CLK GDO2_PIN
#define SERIAL_DATA GDO0_PIN

//************************************* class **************************************************//
class CC1101
{
	private:
	  byte PaTable[8] = {0x00,0x80,0x27,0x67,0x50,0x80,0xc0,0x00}; //
	  byte CW[1] = {0xff};
	  byte IDLE[1] = {0x00};
	  byte OFF[1] = {0x00};
    
		void SpiInit(void);
		void SpiMode(byte config);
		byte SpiTransfer(byte value);
		void GDO_Set (void);
		void Reset(void);
		void RegConfigSettings(void);
		
	public:
		void SpiWriteReg(byte addr, byte value);
		void SpiWriteBurstReg(byte addr, byte *buffer, byte num);
		void SpiStrobe(byte strobe);
		byte SpiReadReg(byte addr);
		void SpiReadBurstReg(byte addr, byte *buffer, byte num);
		byte SpiReadStatus(byte addr);
		
		byte Init();
	/* Custom Functions */
		void UpdateFifo(byte *data, int nbytes);
		void SendCW(byte duration);
		void SendCWBurst(byte duration);
		void SendByte(byte value);
		void SendIdle(byte duration);
		int16_t ReadRSSI(void);
		
		void DecodeFM0(byte *bitarray, byte *data, byte data_size);
		int PieEncodeData(byte *encoded, const byte *data, byte data_size);
};

extern CC1101 RFMOD;

#endif
