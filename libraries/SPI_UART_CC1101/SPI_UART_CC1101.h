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

#ifndef SPI_UART_CC1101_h
#define SPI_UART_CC1101_h
#include <Arduino.h>

/********************** GDO PINS ********************************/
//setup holistical
#define TX_GDO0_PIN 34 // PORTC2
#define TX_GDO2_PIN 36 // PORTC4

//setup v1b w/ tft
//#define TX_GDO0_PIN 27 // PORTD2
//#define TX_GDO2_PIN 29 // PORTD6

#define TX_SS_PIN 23
//setup holistical
#define TX_GDO0_PIO 2
#define TX_GDO2_PIO 4

//setup holistical
//#define TX_GDO0_PIO 2
//#define TX_GDO2_PIO 6

//setup holistical
/* fast digital read for GDO pins */
#define TX_GDO0_STATE (((PIOC->PIO_PDSR) & (1 << TX_GDO0_PIO)) >> TX_GDO0_PIO)
#define TX_GDO2_STATE (((PIOC->PIO_PDSR) & (1 << TX_GDO2_PIO)) >> TX_GDO2_PIO)

//setup v1b
//#define TX_GDO0_STATE (((PIOD->PIO_PDSR) & (1 << TX_GDO0_PIO)) >> TX_GDO0_PIO)
//#define TX_GDO2_STATE (((PIOD->PIO_PDSR) & (1 << TX_GDO2_PIO)) >> TX_GDO2_PIO)

/****************************************************************/

class SPI_UART_CC1101 {
	private:
      byte TXPaTable[8] = {0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00};
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
