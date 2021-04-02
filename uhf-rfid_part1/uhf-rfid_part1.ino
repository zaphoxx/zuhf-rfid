#include <ZUHF_CC1101.h>
#include <ZUHF_CC1101_REGS.h>
#include <ZUHF_VARS.h>
#include <ZUHF_BUFFER.h>
#include <ZUHF_CRC.h>
#include <SPI_UART_CC1101.h>
#include <SPI.h>
/* Make sure you are using the DUE specific SPI library */
/* You need to install the Arduino IDE for WIN10 - do not install one of the hourly builds */
#include "UHF-RFID.h"
/* ************************ */

//#define RN16_LEN 4 // FM0 encoded RN16 length is here 4Bytes 

/* ************* MAIN DEFAULT CONTROL SETTINGS ************* */
#define TX_POWER    0x16 // 0x11 --> 0x11/0x80 (5.2 dBm) 0x12/0x27 (-9.8 dBm) 0x13/0x67 (-5.0 dBm) 0x14/0x50 (-0.3 dBm) 0x16/0xc0 (9.8 dBm) byte PaTable[8] = {0x00,0x80,0x27,0x67,0x50,0x80,0xc0,0x00}; //
#define REPETITIONS 100
#define AGC2        0x03
#define AGC0        0x90
#define DELAY       200
#define TAG_SETTLE  15
#define CW1         14 // CW after QUERY ~14 * 100us (1.4 ms)
#define CW2         40 // CW after ACK ~40 * 100us (4.0 ms)
/* ******************************************** */

/* configuration variables */
byte      tx_power      = TX_POWER;
uint32_t  repetitions   = REPETITIONS;                                                         
byte      agc2          = AGC2;
byte      agc0          = AGC0;
uint32_t  packet_delay  = DELAY;

/* others */
byte tag_settle   = TAG_SETTLE;
byte cw1          = CW1;
byte cw2          = CW2;
/* ******************************************** */
byte tx_version = 0;
int counter = 0;
byte CWA[128];
String cmd_string = "";

void setup()
{
  // delay to avoid known reset issue - see also https://forum.arduino.cc/index.php?topic=256771.75
  delay(1000);
  /* SERIAL CONNECT */
  Serial.begin(250000);
  for (int i = 0; i < sizeof(CWA); i++) CWA[i]=0xff;
  reader_state = R_INIT;
}

void loop()
{
  switch(reader_state)
  {
    case R_INIT:
      counter = 0;
      tx_version = TX_UNIT.Init();
      delay(10);
      if ((tx_version == 0x14 or tx_version == 0x04))
      {
        Serial.println("[CC1101] Modules Check - OK");
        TX_UNIT.SpiStrobe(CC1101_SFSTXON);
        delay(500);
        Serial.println("**** START RUN ****");
        reader_state = R_START;  
      }
      else
      {
        Serial.println("[CC1101] Error on CC1101 module initialization");
        reader_state = R_WAIT;
        delay(100);
      }
      break;
      
    case R_START:
      /* LET FIFOBUFFER EMPTY FROM A PREVIOUS RUN AND SET TX INTO IDLE MODE */
      while ((TX_UNIT.SpiReadStatus(CC1101_TXBYTES) & 0x7f) > 0);
      TX_UNIT.SpiStrobe(CC1101_SIDLE);
      delay(5);
      /* ********** START TX AND SEND CW FOR TAG SETTLE ********** */
      TX_UNIT.SpiStrobe(CC1101_SFTX);
      delay(5);
      TX_UNIT.SpiStrobe(CC1101_STX);
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 20);
      /* ********** *********************************** ********** */
      reader_state = R_QUERY;
      break;

    case R_QUERY:
      send_default_query();
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 40);
      reader_state = R_START;
      delay(20);
      break;

    default:
      reader_state = R_INIT;
      break;
    }
}
