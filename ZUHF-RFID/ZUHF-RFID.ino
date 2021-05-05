/*  ZUHF-RFID - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
	Version: v1c - supporting CLI accessibility via zuhf-cli.py    
	Author:       Manfred Heinz
    Last Update:  06.05.2021
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
#include <ZUHF_BUFFER.h>
#include <ZUHF_CRC.h>
#include <SPI_UART_CC1101.h>
#include <SPI.h>
/* Make sure you are using the DUE specific SPI library */
/* You need to install the Arduino IDE for WIN10 - do not install one of the hourly builds */
#include "UHF-RFID.h"
//#include "ConsoleMenu.h"

#include <FastLED.h>

#define DEBUG 1
#define QUERYSIZE 22
#define MAXQUERY 1024
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
// holistical setup
#define LED_BLUE_ON (PIOB->PIO_SODR = (1 << 14))
#define LED_RED_ON  (PIOC->PIO_SODR = (1 << 12))
#define LED_BLUE_OFF (PIOB->PIO_CODR = (1 << 14))
#define LED_RED_OFF (PIOC->PIO_CODR = (1 << 12))
#define LED_GREEN_ON  (PIOC->PIO_SODR = (1 << 14))
#define LED_GREEN_OFF (PIOC->PIO_CODR = (1 << 14))

// v1b setup
/*
#define LED_BLUE_ON (PIOD->PIO_SODR = (1 << 1))
#define LED_RED_ON  (PIOD->PIO_SODR = (1 << 0))
#define LED_BLUE_OFF (PIOD->PIO_CODR = (1 << 1))
#define LED_RED_OFF (PIOD->PIO_CODR = (1 << 0))
#define LED_GREEN_ON  (PIOD->PIO_SODR = (1 << 3))
#define LED_GREEN_OFF (PIOD->PIO_CODR = (1 << 3))
*/
/* fast digital read for GDO pins */
//#define TX_GDO0_STATE (((PIOC->PIO_PDSR) & (1 << 2)) >> 2)
//#define TX_GDO2_STATE (((PIOC->PIO_PDSR) & (1 << 4)) >> 4)
//#define RX_GDO0_STATE (((PIOA->PIO_PDSR) & (1 << 15)) >> 15)
//#define RX_GDO2_STATE (((PIOD->PIO_PDSR) & (1 << 1)) >> 1)

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
byte sync1,sync0;
/* ******************************************** */



//DATA_BUFFER databuffer;
TAG_INFO tags_read[1000];
/* array to hold query in form of a bit array */
byte QUERY[QUERYSIZE];
/* array to hold tags response in form of a bit array */
byte TAG_RESPONSE[512];
/* array to hold the PIE encoded query in form of a bit array */
byte ENCODED_QUERY[MAXQUERY];
/* array holding the request in the final byte format which will be send to TX FIFO */
byte FIFOBUFFER[512];
/* array holding the ACK command in form of a bit array */
byte ACK_BITS[128];
/* array holding the RN16 bits */
byte RN16_Bits[16];
byte HANDLE_Bits[16];
byte tmp_bits[16];
byte REQ_RN_BITS[40]; // 1 byte command + 2 bytes RN16 + 2 bytes CRC16
  

/* buffer to hold data received on rx module */
byte RX_BUFFER[512];
byte EPC_BUFFER[32];

//bool RN16_Found = 0;
//bool EPC_Flag = 0;
uint64_t EPCID;

int  buffer_size = 0;
int  irounds = 1;
byte cw_counter = 0;
byte encoded_size = 0;
byte check = 0;
uint32_t counter = 0;

byte rx_version = 0;
byte tx_version = 0;

int tags_found = 0;

int n_tag_responses = 0;
byte CWA[64];

unsigned long timeStart;
unsigned long timeEnd;
byte words;
byte cmd_bytes[256];
byte serial_data;
String cmd_string;
//M_STATES current_Menu;
boolean write_flag=false, read_flag=false, block_flag=false, lock_flag=false;
EPC_DATA tag_data;

/* MEMBLOCK READ/WRITE VARS */
byte memoryblock = 3;
byte blockaddr = 0;
byte nWords = 1;
word dataword = 0;
byte data[512];
byte mask_bits[20];


void(* resetFunc) (void) = 0;


void setup()
{
  // delay to avoid known reset issue - see also https://forum.arduino.cc/index.php?topic=256771.75
  delay(1000);
  Serial.begin(250000);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LEDDATA_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, LEDDATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(BRIGHTNESS);
  int byteblocks;
  /* fill CWA array */
  for (int i = 0; i < 64; i++)
  {
    CWA[i] = 0xff;
  }
  
  //logo();
  reader_state = R_WAIT;
  counter = 0;
//  current_Menu = M_MAIN;
  read_flag = false;
  write_flag = false;
  block_flag = false;
  lock_flag = false;
  for (int i = 0; i < sizeof(data); i++) data[i] = 0;
}


void loop()
{
  switch(reader_state)
  {
    case R_INIT:
      /* reset all necessary variables */
      
      counter = 0;
      tags_found = 0;
      tx_version = TX_UNIT.Init(); // Based on SPI_UART_CC1101.h lib
      rx_version = RX_UNIT.Init(); // Based on Arduino SPI.h and ZUHF_CC1101.h lib
      
      /* quick check if modules version looks ok, if modules do not work properly the version value will not match */
      //tx_version = TX_UNIT.Init(); // Based on SPI_UART_CC1101.h lib
      //rx_version = RX_UNIT.Init(); // Based on Arduino SPI.h and ZUHF_CC1101.h lib
      delay(10);
      tx_version = TX_UNIT.SpiReadStatus(CC1101_VERSION);
      rx_version = RX_UNIT.SpiReadStatus(CC1101_VERSION);
      
      if ((tx_version == 0x14 or tx_version == 0x04) and (rx_version == 0x14 or rx_version == 0x04))
      {
        Serial.println("[CC1101] Modules Check - OK");
        //Serial.println("[TX] SFTXON");
        TX_UNIT.SpiStrobe(CC1101_SFSTXON);
        //Serial.println("[RX] SFTXON");
        RX_UNIT.SpiStrobe(CC1101_SFSTXON);

        n_tag_responses = 0;
        /* syncronization word - preamble + Framesync */
        sync1 = 0xAD;
        sync0 = 0x23;
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
        leds[0] = CRGB::Magenta;
        leds[1] = CRGB::Magenta;
        FastLED.show();
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
      LED_BLUE_OFF;
      LED_RED_OFF;
      leds[0] = CRGB::Magenta;
      leds[1] = CRGB::Magenta;
      FastLED.show();
      /* Set modules into idle mode */
      /*this section is necessary otherwise the rx-module will not capture any more signals */
      /* begin */
      while ((TX_UNIT.SpiReadStatus(CC1101_TXBYTES) & 0x7f) > 0);
      /* Terminate TX Session - Go Into Standby */
      TX_UNIT.SpiStrobe(CC1101_SIDLE);
      RX_UNIT.SpiStrobe(CC1101_SIDLE);
      delay(5);
      /* end */
      
      for (int i = 0; i < 16; i++) RN16_Bits[i] = 0;
      for (int i = 0; i < 16; i++) HANDLE_Bits[i] = 0;
      LED_GREEN_ON;
      /* ********** START TX AND SEND CW FOR TAG SETTLE ********** */
      TX_UNIT.SpiStrobe(CC1101_SFTX);
      RX_UNIT.SpiStrobe(CC1101_SFRX);
      delay(2);
      TX_UNIT.SpiStrobe(CC1101_STX);
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 20);
      /* ********** *********************************** ********** */
      reader_state = R_QUERY;
      break;

    case R_QUERY:
      if (counter < repetitions)
      {
        reader_state = R_START;
      }
      else
      {
        reader_state = R_POWERDOWN;
      }
      
      send_default_query();
      if (read_RN16(RN16_Bits))
      {
        LED_BLUE_ON;
        send_ack(RN16_Bits);
        if (read_epc(&tag_data))
        {
          LED_RED_ON;
          send_req_rn(RN16_Bits);
          if (read_Handle(HANDLE_Bits))
          {
            TX_UNIT.SendCW(50);
            leds[0] = CRGB::Blue;
            leds[1] = CRGB::Green;
            FastLED.show();
            
            Serial.println("#TAGDATA");
            Serial.write(tag_data.stored_pc,2);
            Serial.write(tag_data.epc,12);
            Serial.write(tag_data.crc16,2);
            //debug(String(write_flag));
            if (read_flag)
            {
              reader_state = R_READ;
              counter = 0;
            }
            else if(write_flag)
            {
              reader_state = R_WRITE;
              counter = 0;
            }
            else if(lock_flag)
            {
              reader_state = R_LOCK;
              counter = 0;
            }
            else
            {
              reader_state = R_POWERDOWN;
              counter = repetitions;
            }
          }
          else
          {
            counter++;
            leds[0] = CRGB::Orange;
            leds[1] = CRGB::Orange;
            FastLED.show();
            LED_BLUE_OFF;
            LED_RED_OFF;
          }
        }
        else
        {
          counter++;
          leds[0] = CRGB::Magenta;
          leds[1] = CRGB::Magenta;
          FastLED.show();
          LED_BLUE_OFF;
          LED_RED_OFF;
        }
      }
      else
      {
        counter++;
        leds[0] = CRGB::Magenta;
        leds[1] = CRGB::Magenta;
        FastLED.show();
        LED_BLUE_OFF;
        LED_RED_OFF;
      }
      break;

    case R_ACCESS:
      if (counter < repetitions){
        reader_state = R_ACCESS;
      }else{
        debug("[ACCESS] FAILED!");
        reader_state = R_POWERDOWN;
      }
      send_req_rn(HANDLE_Bits);
      if(read_Handle(RN16_Bits))
      {
        debug("[ACCESS] *");
        reader_state = R_POWERDOWN;
      }
      counter++;
      break;

    case R_WRITE:
      if (counter < repetitions)
      {
        reader_state = R_WRITE;
      }
      else
      {
        reader_state = R_POWERDOWN;
      }
      
      /* ***** WRITE DATA TO TAG ***** */
      for (int i = 0; i < nWords; i++)
      {
        send_req_rn(HANDLE_Bits);
        if(read_Handle(RN16_Bits))
        {
          dataword = data[(i*2)+1] << 8 | data[(i*2)];
          send_write(memoryblock, blockaddr+i, dataword, HANDLE_Bits, RN16_Bits);
          if (search_write_ack())
          {
            leds[0] = CRGB::Blue;
            leds[1] = CRGB::Green;
            FastLED.show();
            /* READ OUT DATA WRITTEN TO TAG FOR CONFIRMATION */
            reader_state = R_READ;
            // reset counter prior going to R_READ
            counter = 0;
          }
          else
          {
            counter++;
          }
        }
        else
        {
        counter++;
        reader_state = R_QUERY; // start again with full query
        }
      }
      break;

    case R_READ:
      if (counter < repetitions)
      {
        reader_state = R_READ;
      }
      else
      {
        reader_state = R_POWERDOWN;
      }
      for (int i = 0; i < sizeof(data); i++) data[i]=0;
      /* **** READ DATA FROM TAG ***** */
      debug("read nWords: ");debug(String(nWords));
      send_read(memoryblock,blockaddr,nWords,HANDLE_Bits);      
      if (read_data(data, nWords)) //add 4 bytes for crc16 and handle + 1 byte because of the 0 header
      {
        Serial.println("#READDATA");
        Serial.write(data,nWords*2);
        reader_state = R_POWERDOWN;
        counter = repetitions;
      }
      else
      {
        counter++;
      }
      break;


    case R_LOCK:
      /* ******** LOCK CMD ******** */
      //send_lock(mask_bits, HANDLE_Bits);
      break;


    case R_QUERY_BAK:
      send_default_query();
      if (read_RN16(RN16_Bits))
      {
        LED_BLUE_ON;
        /* ***************** SEND ACK **************** */
        send_ack(RN16_Bits);
        /* ******** CAPTURE TAGS EPC RESPONSE ******** */  
        if(read_epc(&tag_data))
        {
          LED_RED_ON;
          /* ******** START ACCESS SEQUENCE ************ */
          send_req_rn(RN16_Bits);
          if (read_Handle(HANDLE_Bits))
          { 
            /* ******** LOCK CMD ******** */
            /*
            byte lock_mask[] = {0,0,0,0,0,0,1,1,0,0};
            byte lock_action[] = {0,0,0,0,0,0,0,0,0,0};
            send_lock(lock_mask, lock_action, HANDLE_Bits);
            */
            /* ************************* */
            TX_UNIT.SendCW(50);
            byte memoryblock = 3;
            /* ***** WRITE DATA TO TAG ***** */
            send_req_rn(HANDLE_Bits);
            if(read_Handle(RN16_Bits)){
              send_write(memoryblock, 0, 0xFECA, HANDLE_Bits, RN16_Bits);
              if (search_write_ack())
              {
                leds[0] = CRGB::Blue;
                leds[1] = CRGB::Green;
                FastLED.show();
              }
            }
            /* **** READ DATA FROM TAG ***** */
            for (byte i = 0; i < 8;i++)
            {
              words = 1;
              send_read(memoryblock,i,words,HANDLE_Bits);
              byte data[64];
              byte data_size = words * 2;
              read_data(data, words); //add 4 bytes for crc16 and handle + 1 byte because of the 0 header
            }
            send_ack(HANDLE_Bits);
          }
        }
        //TX_UNIT.SendCW(50);  
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
      reader_state = R_WAIT;
      LED_BLUE_OFF;
      LED_RED_ON;
      LED_GREEN_OFF;
      leds[0] = CRGB::Magenta;
      leds[1] = CRGB::Magenta;
      FastLED.show();
      Serial.println("**** END RUN ****");
      Serial.println(n_tag_responses);
      Serial.println("#END");  
      break;

    case R_WAIT:
      digitalWrite(LED_RED, HIGH);
      //runMenu();
      cmd_string = Serial.readStringUntil('#');
      debug(cmd_string);
      if (cmd_string.equals("RUN"))
      {
        Serial.println(cmd_string);
        reader_state = R_INIT;
        //delay(500);
      }
      else if (cmd_string.equals("REP"))
      {
        cmd_string = Serial.readStringUntil('#');
        repetitions = cmd_string.toInt();
      }
      else if (cmd_string.equals("TXP"))
      {
        cmd_string = Serial.readStringUntil('#');
        tx_power = (byte) cmd_string.toInt();
      }
      else if (cmd_string.equals("READ"))
      {
        Serial.println("#READ#");
        read_flag = true;
        cmd_string = Serial.readStringUntil('#');
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        blockaddr = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        nWords = (byte) cmd_string.toInt();
      }
      else if (cmd_string.equals("WRITE"))
      {
        Serial.println("#WRITE#");
        write_flag = true;
        block_flag = false;
        debug("#WRITE1");
        
        cmd_string = Serial.readStringUntil('#');
        Serial.println(cmd_string);
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        Serial.println(cmd_string);
        blockaddr = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        Serial.println(cmd_string);
        nWords = (byte) cmd_string.toInt();
        for (int i = 0; i < sizeof(data); i++) data[i] = 0;
        Serial.readBytes(data,nWords * 2);
        //dataword = data[1] << 8 | data[0];
        debug("#WRITE MODE");
      }
      /*
      else if (cmd_string.equals("LOCK"))
      {
        Serial.println("#LOCK#");
        lock_flag = true;
        // Read in the 20 mask/access bits
        for (int i = 0; i < 20; i++){
          mask_bits[i] = (byte) ser.read(1).toInt();
        }
      }
      */
      else
      { 
        reader_state = R_WAIT;
        delay(100);
      }
      cmd_string = "";
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
