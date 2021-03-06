/*  ZUHF-RFID - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
  Version: v1c - supporting CLI accessibility via zuhf-cli.py    
  Author:       Manfred Heinz
    Last Update:  01.06.2021
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
#define TX_POWER    0x16 // 0x11 --> 0x11/0x80 (5.2 dBm) 0x12/0x27 (-9.8 dBm) 0x13/0x67 (-5.0 dBm) 0x14/0x50 (-0.3 dBm) 0x16/0xc0 (10.7 dBm) byte PaTable[8] = {0x00,0x80,0x27,0x67,0x50,0x80,0xc0,0x00}; //
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
boolean write_flag=false, read_flag=false, block_flag=false, lock_flag=false, epc_found=false, monza_flag=true, access_flag;
EPC_DATA tag_data;

/* MEMBLOCK READ/WRITE VARS */
byte memoryblock = 3;
byte blockaddr = 0;
byte nWords = 1;
word dataword = 0;
byte data[512];
byte mask_bits[10], action_bits[10];
uint16_t lpass,hpass;
byte wordcount=0;


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
  access_flag = false;
  lpass = 0;
  hpass = 0;
  for (int i = 0; i < sizeof(data); i++) data[i] = 0;
}


void loop()
{
  switch(reader_state)
  {
    case R_INIT:
      /* reset all necessary variables */
      
      counter = 0;
      epc_found = 0;
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
      wordcount = 0;
      LED_GREEN_ON;
      /* ********** START TX AND SEND CW FOR TAG SETTLE ********** */
      TX_UNIT.SpiStrobe(CC1101_SFTX);
      RX_UNIT.SpiStrobe(CC1101_SFRX);
      delay(5);
      TX_UNIT.SpiStrobe(CC1101_STX);
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 20);
      /* ********** *********************************** ********** */
      {
        reader_state = R_QUERY;
      }
      break;

    /* QUERY REQUEST
     * R_QUERY: 
     * 1) send QUERY request and read tag response (RN16)
     * 2) send ACK and read tags EPC response / if only EPC reading then this is the last step otherwise see 3)
     * 3) send REQ_RN and transition tag into OPEN/SECURED state (depending on access passwd setting)
     * 4) delegate to next command (READ, WRITE, LOCK, ACCESS etc.)
     */
    case R_QUERY:
      if (counter < repetitions)
      {
        reader_state = R_START;
      }
      else
      {
        reader_state = R_POWERDOWN;
      }
      // 1) SEND QUERY REQUEST
      send_default_query();
      //    READ TAG RESPONSE / RN16
      if (read_RN16(RN16_Bits))
      {
        LED_BLUE_ON;
        // 2) SEND ACK
        send_ack(RN16_Bits);
        //    READ TAG RESPONSE / EPC DATA
        if (read_epc(&tag_data))
        {
          debug("[ACKNOWLEDGED]");
          epc_found = true;
          LED_RED_ON;
          //debug("[+] EPC Found!");
            
          if ( !read_flag && !write_flag && !lock_flag ){
            counter = repetitions;
          }
          else
          {
            // 3) SEND REQ_RN
            send_req_rn(RN16_Bits);
            //    READ HANDLE
            if (read_Handle(HANDLE_Bits))
            {
              // TAG SHOULD NOW BE either in OPEN or SECURED STATE
              debug("[OPEN/SECURED]");
              /* add some CW to ensure tag stays powered up */
              TX_UNIT.SendCW(32);
              
              // 4) DELEGATE TO NEXT COMMAND
              if (access_flag)
              {
                debug("SWITCH TO ACCESS");
                reader_state = R_ACCESS;
                counter = 0;
              }
              else if (read_flag)
              {
                reader_state = R_READ;
                counter = 0;
              }
              else if(write_flag)
              {
                //debug("[WRITE] *");
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
      /* access sequence
       * 1) send req_rn / recv new RN16
       * 2) send access_cmd with [password (lower word) xor RN16] / check crc16 of response
       * 3) send req_rn / recv new RN16
       * 4) send access_cmd with upper word / check crc16 response
       * 5) if all good tag goes into secured state / if not goes back to arbitrate state (full query sequence needs to be repeated)
       */
      if (counter < repetitions)
      {
        reader_state = R_QUERY;
        // in case of errors restart full query sequence
      }
      else
      {
        debug("[ACCESS] FAILED!");
        reader_state = R_POWERDOWN;
      }

      /* initiate first access sequence with req_rn(handle) */
      send_req_rn(HANDLE_Bits);
      if(read_Handle(RN16_Bits))
      {
        //debug("[ACCESS 1] *");
        send_access(lpass, HANDLE_Bits, RN16_Bits);
        if (search_access_ack())
        {
          debug("[ACCESS 1] OK");
          /* send second access sequence */
          send_req_rn(HANDLE_Bits);
          if(read_Handle(RN16_Bits))
          {
            //debug("[ACCESS 2] *");
            send_access(hpass, HANDLE_Bits, RN16_Bits);
            if (search_access_ack())
              debug("[SECURED]");
              //TX_UNIT.SendCW(32);
              if (read_flag)
              {
                reader_state = R_READ;
                counter = 0;
              }
              else if (write_flag)
              {
                reader_state = R_WRITE;
                counter = 0;
              }
              else if (lock_flag)
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
              LED_BLUE_OFF;
              LED_RED_OFF;
              break;
            }
          }
          else
          {
            counter++;
            LED_BLUE_OFF;
            LED_RED_OFF;
            break;
          }
        }
        else
        {
          counter++;
          LED_BLUE_OFF;
          LED_RED_OFF;
          break;
        }     
      break;

    case R_WRITE:
      /* ***** WRITE DATA TO TAG ***** */
      if (counter < repetitions)
      {
        reader_state = R_WRITE;
      }
      else
      {
        reader_state = R_POWERDOWN;
        break;
      }
      debug("[WRITE] *");
      debug(String(wordcount));
      debug(String(nWords));
      debug(String(counter));
      while ((wordcount < (nWords)) && (counter < repetitions))
      {
        debug("[REQ_RN] *");
        send_req_rn(HANDLE_Bits);
        if(read_Handle(RN16_Bits))
        {
          debug("[REQ_RN(Handle)]");
          dataword = data[(wordcount*2)+1] << 8 | data[(wordcount*2)];
          send_write(memoryblock, blockaddr+wordcount, dataword, HANDLE_Bits, RN16_Bits);
          if (search_write_ack())
          {
            TX_UNIT.SendCW(16);
            wordcount++;
          }
          
        }
        counter++;
        leds[0] = CRGB::Magenta;
        leds[1] = CRGB::Magenta;
        FastLED.show();
        LED_BLUE_OFF;
        LED_RED_OFF;
      }
      counter = repetitions;
      break;

    case R_READ:
      TX_UNIT.SendCW(16);
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
      //debug("[READ] *");
      //debug(String(memoryblock));
      //debug(String(blockaddr));
      
      //debug(String(nWords));
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
      if (counter < repetitions)
      {
        reader_state = R_LOCK;
      }
      else
      {
        reader_state = R_POWERDOWN;
        break;
      }
      while (counter < repetitions)
      {
        send_lock(mask_bits, action_bits, HANDLE_Bits);
        //debug("[+] R_LOCK");
        if (search_lock_ack())
        {        
          counter = repetitions;
          break;
        }
        counter++;
        LED_BLUE_OFF;
        LED_RED_OFF;
        TX_UNIT.SendCW(16);
        break;
      }
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
      if (epc_found){
        Serial.println("#TAGDATA");
        Serial.write(tag_data.stored_pc,2);
        Serial.write(tag_data.epc,12);
        Serial.write(tag_data.crc16,2);
      }
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
      //debug(cmd_string);
      if (cmd_string.equals("RUN"))
      {
        debug("Running ...");
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
      // Reading data from memory blocks
      else if (cmd_string.equals("READ"))
      {
        //Serial.println("#READ#");
        read_flag = true;
        write_flag = false;
        lock_flag = false;
        block_flag = false;
        access_flag = false;
        cmd_string = Serial.readStringUntil('#');
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        blockaddr = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        nWords = (byte) cmd_string.toInt();
        debug("[+] Read Mode");
      }
      // Writing data to tag
      else if (cmd_string.equals("WRITE"))
      {
        write_flag = true;
        block_flag = false;
        monza_flag =false;
        lock_flag = false;
        access_flag = false;
        
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        blockaddr = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        nWords = (byte) cmd_string.toInt();
        for (int i = 0; i < sizeof(data); i++) data[i] = 0;
        Serial.readBytes(data,nWords * 2);
        //dataword = data[1] << 8 | data[0];
        debug("[+] WRITE MODE");
      }
      /* Section for processing of access sequence */
      else if (cmd_string.equals("READACCESS"))
      {
        access_flag = true;
        read_flag = true;
        write_flag = false;
        lock_flag = false;
        /* Read in passwd as lower and upper word */
        byte tmp[2];
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        lpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        hpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');

        /* read in standart parameters */
        cmd_string = Serial.readStringUntil('#');
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        blockaddr = (byte) cmd_string.toInt();
        //debug(String(blockaddr));
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        nWords = (byte) cmd_string.toInt();
        //debug(String(nWords));
        debug("[READACCESS]");
      }
      else if (cmd_string.equals("WRITEACCESS"))
      {
        access_flag = true;
        read_flag = false;
        write_flag = true;
        lock_flag = false;
        byte tmp[2];
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        lpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        hpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');
        
        cmd_string = Serial.readStringUntil('#');
        memoryblock = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        blockaddr = (byte) cmd_string.toInt();
        cmd_string = Serial.readStringUntil('#');
        //Serial.println(cmd_string);
        nWords = (byte) cmd_string.toInt();
        for (int i = 0; i < sizeof(data); i++) data[i] = 0;
        Serial.readBytes(data,nWords * 2);
      }
      
      // Section if only basic EPC read should be performed
      else if (cmd_string.equals("EPC"))
      {
        Serial.println("#READ_EPC");
        write_flag = false;
        read_flag = false;
        block_flag = false;
        access_flag = false;
        debug("[+] Read EPC");
      }
      else if (cmd_string.equals("LOCK"))
      {
        write_flag = false;
        read_flag = false;
        lock_flag = true;
        access_flag = false;
        
        
        // Read in the 20 mask/action bits
        Serial.readBytes(mask_bits,10);
        Serial.readBytes(action_bits,10);
        
        debug("[+] LOCK MODE");
      }
      else if (cmd_string.equals("LOCKACCESS"))
      {
        access_flag = true;
        write_flag = false;
        read_flag = false;
        lock_flag = true;

        byte tmp[2];
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        lpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');
        Serial.readBytes(tmp,2);
        debug(tmp,2);
        hpass = (tmp[1] << 8) | tmp[0];
        cmd_string = Serial.readStringUntil('#');
        
        // Read in the 20 mask/action bits
        Serial.readBytes(mask_bits,10);
        Serial.readBytes(action_bits,10);
        
        debug("[LOCKACCESS]");
      }
      else
      { 
        reader_state = R_WAIT;
        //flushSerial();
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

void flushSerial()
{
  while (Serial.available())
  {
    byte dummy = Serial.read();
  }
}
