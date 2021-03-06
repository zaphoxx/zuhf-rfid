/*
	ZUHF_CC1101.h
	Basic CC1101 library for Arduino Due. It makes use of the Arduino SPI.h library.
*/
/*  ZUHF_CC1101.h - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
    Author:       Manfred Heinz
    Last Update:  06.03.2021
    Copyright (C) 2021  Manfred Heinz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ZUHF_CC1101_h
#define ZUHF_CC1101_h

#include "arduino.h"
#include <SPI.h>

//SPI PINS
#define SCK_PIN 76
#define MISO_PIN 74
#define MOSI_PIN 75

// PINS FOR TX MODULE
#define RX_PIN 52
#define RX_GDO0_PIN 24 // PORTA 15
#define RX_GDO2_PIN 26 // PORTD 1
#define RX_GDO0_PIO 15
#define RX_GDO2_PIO 1

#define RX_GDO0_STATE (((PIOA->PIO_PDSR) & (1 << RX_GDO0_PIO)) >> RX_GDO0_PIO)
#define RX_GDO2_STATE (((PIOD->PIO_PDSR) & (1 << RX_GDO2_PIO)) >> RX_GDO2_PIO)

//************************************* class **************************************************//
class ZUHF_CC1101
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

extern ZUHF_CC1101 RX_UNIT;

#endif