/*  ZUHF-RFID - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
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

#include <ZUHF_CC1101.h>
#include <ZUHF_CC1101_REGS.h>
#include <ZUHF_VARS.h>
#include <ZUHF_CRC.h>
#include <SPI_UART_CC1101.h>
#include <SPI.h>
/* Make sure you are using the DUE specific SPI library */
/* You need to install the Arduino IDE for WIN10 - do not install one of the hourly builds */
#include "UHF-RFID.h"
#include "ConsoleMenu.h"

#include <FastLED.h>

//#define DEBUG 1
//#define QUERYSIZE 22
//#define MAXQUERY 1024
/* SHIELD SPECIFIC SETTINGS */
#define LED_BLUE 53
#define LED_RED 51
#define LED_GREEN 49 
#define LEDDATA_PIN 50 // to control two WS2812B LEDs onboard for fun
#define NUM_LEDS    2
#define BRIGHTNESS  16
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
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

/* fast digital write for LEDS */
#define LED_BLUE_ON (PIOB->PIO_SODR = (1 << 14))
#define LED_RED_ON  (PIOC->PIO_SODR = (1 << 12))
#define LED_BLUE_OFF (PIOB->PIO_CODR = (1 << 14))
#define LED_RED_OFF (PIOC->PIO_CODR = (1 << 12))
#define LED_GREEN_ON  (PIOC->PIO_SODR = (1 << 14))
#define LED_GREEN_OFF (PIOC->PIO_CODR = (1 << 14))

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
/* syncronization word - preamble + Framesync */
byte sync1        = 0xAD;
byte sync0        = 0x23;
/* ******************************************** */

TAG_INFO tags_read[1000];
byte RN16_Bits[16];
byte HANDLE_Bits[16];

uint32_t counter = 0;
byte rx_version = 0;
byte tx_version = 0;

int tags_found = 0;
int n_tag_responses = 0;
byte words = 0;

M_STATES current_Menu;

void setup()
{
  Serial.begin(250000);
  /* LED PIN SETTINGS */
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LEDDATA_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, LEDDATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(BRIGHTNESS);
  reader_state = R_WAIT;
  counter = 0;
  current_Menu = M_MAIN;
}

void loop()
{
  switch(reader_state)
  {
    case R_INIT:
      /* reset all necessary variables */
      tags_found = 0;
      n_tag_responses = 0;
      /* INITIALISE CC1101 MODULES */
      tx_version = TX_UNIT.Init(); // Based on SPI_UART_CC1101.h lib
      rx_version = RX_UNIT.Init(); // Based on Arduino SPI.h and ZUHF_CC1101.h lib
      delay(10);
      //tx_version = TX_UNIT.SpiReadStatus(CC1101_VERSION);
      //rx_version = RX_UNIT.SpiReadStatus(CC1101_VERSION);
      
      if ((tx_version == 0x14 or tx_version == 0x04) and (rx_version == 0x14 or rx_version == 0x04))
      {
        Serial.println("[CC1101] Modules Check - OK");
        TX_UNIT.SpiStrobe(CC1101_SFSTXON);
        RX_UNIT.SpiStrobe(CC1101_SFSTXON);
        
        /* ************************ RX UNIT REG SETTINGS ************************* */
        RX_UNIT.SpiWriteReg(CC1101_MCSM0,0x28); 
        RX_UNIT.SpiWriteReg(CC1101_MCSM1,0x35); 
        RX_UNIT.SpiWriteReg(CC1101_FREND0,tx_power);
        RX_UNIT.SpiWriteReg(CC1101_PKTCTRL0, 0x00);
        RX_UNIT.SpiWriteReg(CC1101_PKTLEN, 0x0c);
        RX_UNIT.SpiWriteReg(CC1101_IOCFG0, 0x06); 
        RX_UNIT.SpiWriteReg(CC1101_MDMCFG2, 0x32);
        /* syncronization word - preamble + Framesync */
        RX_UNIT.SpiWriteReg(CC1101_SYNC1, sync1);
        RX_UNIT.SpiWriteReg(CC1101_SYNC0, sync0);
        RX_UNIT.SpiWriteReg(CC1101_AGCCTRL2, agc2); 
        RX_UNIT.SpiWriteReg(CC1101_AGCCTRL1, 0x00);
        RX_UNIT.SpiWriteReg(CC1101_AGCCTRL0, agc0);
        RX_UNIT.SpiWriteReg(CC1101_FIFOTHR, 0x07);
        /* ************************ RX UNIT REG SETTINGS ************************* */
  
        Serial.print("TX power      : ");Serial.println(tx_power,HEX);
        Serial.print("AGC2          : ");Serial.println(agc2,HEX);
        Serial.print("AGC0          : ");Serial.println(agc0,HEX);
        Serial.print("PktDelay [µs] : ");Serial.println(packet_delay,DEC);
        Serial.print("REPETITIONS   : ");Serial.println(repetitions,DEC);
        /* -------- sw2812 begin -------- */
        leds[0] = CRGB::Magenta;
        leds[1] = CRGB::Magenta;
        FastLED.show();
        /* -------- sw2812 end ---------- */
        delay(3000);
        Serial.println("**** START RUN ****");
        reader_state = R_START;
      }else{
        Serial.println("[CC1101] Error on CC1101 module initialization");
        reader_state = R_WAIT;
        delay(1000);
      }
      break;
      
    case R_START:
      LED_BLUE_OFF;
      LED_RED_OFF;
      leds[0] = CRGB::Magenta;
      leds[1] = CRGB::Magenta;
      FastLED.show();
      for (int i = 0; i < 16; i++) RN16_Bits[i] = 0;
      for (int i = 0; i < 16; i++) HANDLE_Bits[i] = 0;
      LED_GREEN_ON;
      /* ********** START TX AND SEND CW FOR TAG SETTLE ********** */
      TX_UNIT.SpiStrobe(CC1101_SFTX);
      RX_UNIT.SpiStrobe(CC1101_SFRX);
      delay(2);
      TX_UNIT.SpiStrobe(CC1101_STX);
      /* SEND CW FOR TAG SETTLE */
      TX_UNIT.SendCW(20);
      /* ********** *********************************** ********** */
      reader_state = R_QUERY;
      break;

    case R_QUERY:
      /* SEND A BASIC QUERY CMD */
      send_default_query();
      /* TRY TO READ RN16 FROM TAG REPLY AND SEND ACK BACK */
      if (read_RN16(RN16_Bits))
      {
        LED_BLUE_ON;
        /* ***************** SEND ACK **************** */
        send_ack(RN16_Bits);
        /* ******** CAPTURE TAGS EPC RESPONSE ******** */
        EPC_DATA tag_data;
        if(read_epc(&tag_data)) LED_RED_ON;
        /* ******** START ACCESS SEQUENCE ************ */

        /* REQUEST HANDLE NECESSARY FOR ACCESS COMMANDS */
        send_req_rn(RN16_Bits);
        if (read_Handle(HANDLE_Bits))
        { 
          /* 
          byte lock_mask[] = {0,0,0,0,0,0,1,1,0,0};
          byte lock_action[] = {0,0,0,0,0,0,0,0,0,0};
          send_lock(lock_mask, lock_action, HANDLE_Bits);
          TX_UNIT.SendCW(100);
          */
          byte memoryblock = 3;
          TX_UNIT.SendCW(50);
          /* *** WRITE COMMAND - REQUEST RN16 + READ RN16 + SEND WRITE CMD *** */
          send_req_rn(HANDLE_Bits);
          if(read_Handle(RN16_Bits)){
            send_write(memoryblock, 0, 0xaaaa, HANDLE_Bits, RN16_Bits);
            //TX_UNIT.SendCW(100);
            if (search_write_ack())
            {
              leds[0] = CRGB::Blue;
              leds[1] = CRGB::Green;
              FastLED.show();
            }
          }
           /* ******************** */
           /* *** READ COMMAND *** */
          for (byte i = 0; i < 8;i++)
          {
            words = 1;
            send_read(memoryblock,i,words,HANDLE_Bits);
            byte data[64];
            byte data_size = words * 2;
            read_data(data, words); //add 4 bytes for crc16 and handle + 1 byte because of the 0 header
          }
          /* ******************** */
          /* *** ACK  COMMAND *** */
          send_ack(HANDLE_Bits);
          /* ******************** */
        }
      }
      reader_state = R_POWERDOWN;
      break;
      
    case R_POWERDOWN:
      /* GRACEFULLY TERMINATE TX UNIT */
      /* wait till fifo is empty before going into standby */
      while ((TX_UNIT.SpiReadStatus(CC1101_TXBYTES) & 0x7f) > 0);
      /* Terminate TX Session - Go Into Standby */
      TX_UNIT.SpiStrobe(CC1101_SIDLE);
      RX_UNIT.SpiStrobe(CC1101_SIDLE);
      if (counter < repetitions){
        reader_state = R_START;
        LED_BLUE_OFF;
        LED_RED_OFF;
        LED_GREEN_ON;
        delay(10);
        Serial.println("-------------");
      }else{
        reader_state = R_WAIT;
        LED_BLUE_OFF;
        LED_RED_OFF;
        LED_GREEN_OFF;
        Serial.println("**** END RUN ****");
        Serial.println(n_tag_responses);
        
      }
      counter++;
      break;

    case R_WAIT:
      counter = 0;
      digitalWrite(LED_RED, HIGH);
      
      runMenu();
      reader_state = R_INIT;
      break;
    default:
      reader_state = R_WAIT;
      break;
    }
}


void logo()
{
  Serial.println();
  Serial.println();
  Serial.write(" @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@\n      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@\n    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!\n  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!\n :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : \n");
  Serial.println();
  Serial.println();
}
