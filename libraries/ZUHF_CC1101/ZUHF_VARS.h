/*
* global vars
*/
/*
	ZUHF_VARS.h
	Basic CC1101 library for Arduino Due. It makes use of the Arduino SPI.h library.
*/
/*  ZUHF_VARS.h - Arduino Sketch to run a self build UHF RFID Reader (Read/Write)
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
#ifndef ZUHF_VARS_H
#define ZUHF_VARS_H

#define BUFFER_SIZE 255

/* ********** TAG SPECIFIC - PC MASKS *********** */
#define L_MASK    0xF800
#define T_MASK    0x0100
#define XI_MASK   0x0200
#define UMI_MASK  0x0400
/* ********************************************** */

struct TAG_INFO{
  uint16_t 	StoredPC 		= 0;
  uint8_t	RN16[2] 		= {};
  uint8_t	EPC_Data[12] 	= {}; // EPC Length actually depends on StoredPC/PacketPC entry
  uint8_t	CRC16[2] 		= {};
  bool 		CRC_OK 			= false;
};



enum READER_STATES
{
  R_INIT,
	R_WAIT,
	R_START,
	R_SELECT,
	R_QUERY,
	R_QUERYREP,
	R_QUERYADJ,
	R_SEARCH_RN16,
	R_ACK,
	R_NAK,
	R_CW,
	R_POWERUP,
	R_POWERDOWN,
  R_READDATA,
	R_TEST
};



// CONSTANTS (READER CONFIGURATION)

// Fixed number of slots (2^(FIXED_Q))  
const byte FIXED_Q              = 0;

// Query command (Q is set in code)
const byte QUERY_CODE[4] = {1,0,0,0};                       // QUERY command
const byte DR            = 0;                               // 0: TRcal divide ratio (8); 1: TRcal divide ratio (64/3)
const byte M[4][2]       = {{0,0},{0,1},{1,0},{1,1}};       // cycles per symbol (FM0 encoding = {0,0})
const byte TREXT         = 1;                               // 1: pilot tone; 0: no pilot tone
const byte SEL_ALL[2]    = {0,0};                            // which Tags respond to the Query: ALL TAGS {0,0} and {0,1}
const byte SEL_SL[4][2]  = {{0,0},{0,1},{1,0},{1,1}};       // which Tags respond to the Query: SELECTED TAGS
const byte SESSION[4][2] = {{0,0},{0,1},{1,0},{1,1}};       // session for the inventory round SL0,SL1,SL2,SL3 respectively
const byte TARGET        = 0;                               // inventoried flag is A or B

// valid values for Q
const byte Q_VALUE [16][4] =  
{
	{0,0,0,0}, {0,0,0,1}, {0,0,1,0}, {0,0,1,1}, 
	{0,1,0,0}, {0,1,0,1}, {0,1,1,0}, {0,1,1,1}, 
	{1,0,0,0}, {1,0,0,1}, {1,0,1,0}, {1,0,1,1},
	{1,1,0,0}, {1,1,0,1}, {1,1,1,0}, {1,1,1,1}
};  

const byte MEM_BANK [4][2] =
{
	{0,0}, // Reserved
	{0,1}, // EPC
	{1,0}, // TID
	{1,1} // USER
};


/*
* The settings below assume a pulsewidth of 12.5µs (cc1101 datarate at 80 kBaud)
* 1 TARI = 25µs <--> data0 duration = 25µs
*/
const byte DATA0[] = {1,0};
const byte DATA1[] = {1,1,1,0};
const byte DELIM[] = {0};
const byte RTCAL[] = {1,1,1,1,1,0}; // length of (data0 + data1)
const byte TRCAL[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}; // max 3 * length(RTCAL) 16*12.5µs = 200µs --> BLF = 8/200µs = 40kHz (BLF = DR/RTCAL)

/*
* FRAMESYNC: delim + data0 + rtcal 
*/
const byte FRAMESYNC[] = {0,1,0,1,1,1,1,1,0}; // delim + data0 + rtcal
/*
/*
* ACK 
*/
const byte ACK[] = {0,1}; 

/*
 * REQ_RN
 */
const byte REQ_RN[] = {1,1,0,0,0,0,0,1};

/*
 * READ
 */
const byte READ_CMD[] = {1,1,0,0,0,0,1,0};

/*
 * WRITE
 */
const byte WRITE_CMD[] = {1,1,0,0,0,0,1,1};

/*
 * LOCK
 */
const byte LOCK_CMD[] = {1,1,0,0,0,1,0,1};

/*
* QUERYREP: framesync + 4 x data0
*/
const byte QUERYREP[] = {0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0};

/*
* PREAMBLE: delim + data0 + rtcal + trcal
*/
const byte PREAMBLE[] = {0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};

READER_STATES reader_state = R_START;

#endif