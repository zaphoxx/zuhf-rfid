/* SPI_UART_CC1101.H
 * The Code Programs the Arduino Due to use the Serial2 USART as SPI
 * From https://forum.arduino.cc/index.php?topic=283766.0 I used some
 * of the code snippets for that.
 * The Library is build to communicate with a CC1101 and not an all purpose
 * SPI_UART Library. But feel free to copy and use the code as needed.
 * The Library makes the following assumptions
 * CC1101 is used as TX Module (w/ Arduino Due Atmel SAM3X8E)
 *  --> CSS Pin is called TX_PIN
 *  --> Clock Divider is set to 42 --> 2MHz (or 21 --> 4MHz)
 *  --> MOSI : TX2 (A.12)
 *  --> MISO : RX2 (A.13)
 *  --> SCK  : SCK1/A0 (A.16)
 *  --> CSS  : PIN 23 (A.14)
 */

/*  SPI_UART_CC1101.h - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
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
#ifndef SPI_UART_CC1101_h
#define SPI_UART_CC1101_h
#include <arduino.h>

/********************** GDO PINS ********************************/
#define TX_GDO0_PIN 34 // PORTC2
#define TX_GDO2_PIN 36 // PORTC4
#define TX_SS_PIN 23
#define TX_GDO0_PIO 2
#define TX_GDO2_PIO 4

/* fast digital read for GDO pins */
#define TX_GDO0_STATE (((PIOC->PIO_PDSR) & (1 << TX_GDO0_PIO)) >> TX_GDO0_PIO)
#define TX_GDO2_STATE (((PIOC->PIO_PDSR) & (1 << TX_GDO2_PIO)) >> TX_GDO2_PIO)
/****************************************************************/

class SPI_UART_CC1101 {
	private:
      byte TXPaTable[8] = {0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00};
      byte CW[64];
      byte IDLE[1] = {0x00};
      byte OFF[1]  = {0x00};
			
      byte Transfer(byte b);
      byte RegCmd(byte b, byte val);
      byte Transfer(byte *_buf, size_t _count);
      
  public:
      byte Init(void);
      byte SpiReadStatus(byte cmd);
      void SpiWriteReg(byte addr, byte value);
      void SpiWriteBurstReg(byte addr, byte *buffer, byte num);
      void SpiStrobe(byte strobe);
      byte SpiReadReg(byte addr);
      void SpiReadBurstReg(byte addr, byte *buffer, byte num);
      
      void RegConfigSettings(void);
      void UpdateFifo(byte *data, int nbytes);
      void SendCW(byte duration);
      void SendCWBurst(byte duration);
      void SendByte(byte value);
      void SendIdle(byte duration);
      void SearchRN16(byte *buffer);
};

extern SPI_UART_CC1101 TX_UNIT;

#endif