/*
	ZUHF_CC1101_DUE
	Basic CC1101 library for Arduino Due. It makes use of the Arduino SPI.h library.
*/
#ifndef ZUHF_CC1101_CPP
#define ZUHF_CC1101_CPP

#include <ZUHF_CC1101.h>
#include <ZUHF_CC1101_REGS.h>
//#include <arduino.h>

/****************************************************************
*FUNCTION NAME:SpiInit
*FUNCTION     :spi communication initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::SpiInit()
{
  SPI.end();
  
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(RX_PIN, OUTPUT); // CSN / CSS / SS 
  
  /* ********************* */
  pinMode(4, OUTPUT); 
  pinMode(10, OUTPUT);
  pinMode(52, OUTPUT);
  
  /* Initialize SPI For TX CC1101 */
  SPI.begin(RX_PIN);
  SPI.setDataMode(RX_PIN, SPI_MODE0);
  SPI.setClockDivider(RX_PIN, 21); 	// 4MHz // default is 4MHz higher rates are possible but playing it safe here
  SPI.setBitOrder(RX_PIN, MSBFIRST);
}

/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0_TX,GDO2_TX pin
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void ZUHF_CC1101::GDO_Set (void)
{
	pinMode(RX_GDO0_PIN, INPUT);
	pinMode(RX_GDO2_PIN, INPUT);
}

/****************************************************************
*FUNCTION NAME:Reset
*FUNCTION     :CC1101 reset //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::Reset(void)
{
	Serial.println("[RX] ***** BEGIN RESET *****");
	digitalWrite(SCK_PIN, HIGH);
  digitalWrite(MOSI_PIN, LOW);
	digitalWrite(RX_PIN, LOW);
	delay(1);
	digitalWrite(RX_PIN, HIGH);
	delay(1);
	digitalWrite(RX_PIN, LOW);
  while(digitalRead(MISO_PIN));
	SpiStrobe(CC1101_SRES);
  while(digitalRead(MISO_PIN));
  delay(1);
	Serial.println("[RX] ***** RESET COMPLETE *****");
/*
	Note that the above reset procedure is
	only required just after the power supply is
	first turned on. If the user wants to reset
	the CC1101 after this, it is only necessary to
	issue an SRES command strobe
*/
}

/****************************************************************
*FUNCTION NAME:Init
*FUNCTION     :CC1101 initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
byte ZUHF_CC1101::Init()
{
	SpiInit();						
	GDO_Set();										
	//SpiStrobe(CC1101_SRES);
	Reset();
  
  // print out basic cc1101 info: version & partnum as a simple check
  // partnum should be 0x14 or 0x04; version is usually 0x00
  byte xversion = SpiReadStatus(CC1101_VERSION);
  byte xpart    = SpiReadStatus(CC1101_PARTNUM);
  Serial.print("[RX] CSS PIN:     ");Serial.println(RX_PIN, DEC);
  Serial.print("[RX] MOSI PIN:    ");Serial.println(MOSI_PIN, DEC);
  Serial.print("[RX] MISO PIN:    ");Serial.println(MISO_PIN, DEC);
  Serial.print("[RX] SCK PIN:     ");Serial.println(SCK_PIN, DEC);
  Serial.print("[RX] Version:     0x");Serial.println(xversion, HEX);
  Serial.print("[RX] Partnumber:  0x");Serial.println(xpart, HEX);
  
  RegConfigSettings();							
	SpiWriteBurstReg(CC1101_PATABLE,PaTable,8);
  SpiStrobe(CC1101_SCAL);
  return xversion;
}


/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC1101 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::SpiWriteReg(byte addr, byte value)
{
  SPI.transfer(RX_PIN, addr, SPI_CONTINUE);
  SPI.transfer(RX_PIN, value);
}

/****************************************************************
*FUNCTION NAME:SpiWriteBurstReg
*FUNCTION     :CC1101 write burst data to register
*INPUT        :addr: register address; buffer:register value array; num:number to write
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::SpiWriteBurstReg(byte addr, byte *buffer, byte num)
{
	byte i, temp;

	temp = addr | WRITE_BURST;
    SPI.transfer(RX_PIN, temp, SPI_CONTINUE);
    for (i = 0; i < num; i++)
 	{
        if (i == (num - 1)){
			SPI.transfer(RX_PIN, buffer[i]);
		}else{
			SPI.transfer(RX_PIN, buffer[i], SPI_CONTINUE);
		}
	}
}

/****************************************************************
*FUNCTION NAME:SpiStrobe
*FUNCTION     :CC1101 Strobe
*INPUT        :strobe: command; //refer define in CC1101.h//
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::SpiStrobe(byte strobe)
{
	SPI.transfer(RX_PIN, strobe);
}

/****************************************************************
*FUNCTION NAME:SpiReadReg
*FUNCTION     :CC1101 read data from register
*INPUT        :addr: register address
*OUTPUT       :register value
****************************************************************/
byte ZUHF_CC1101::SpiReadReg(byte addr) 
{
	byte temp, value;
    temp = addr|READ_SINGLE;
	value = SPI.transfer(RX_PIN, temp, SPI_CONTINUE);
	value = SPI.transfer(RX_PIN, 0x00);
	return value;
}

/****************************************************************
*FUNCTION NAME:SpiReadBurstReg
*FUNCTION     :CC1101 read burst data from register
*INPUT        :addr: register address; buffer:array to store register value; num: number to read
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::SpiReadBurstReg(byte addr, byte *buffer, byte num)
{
	byte i,temp;
	temp = addr | READ_BURST;
	SPI.transfer(RX_PIN, temp, SPI_CONTINUE);
	for(i=0;i<num;i++)
	{
		if (i < (num-1)) {
			buffer[i]=SPI.transfer(RX_PIN, 0x00, SPI_CONTINUE);
		}else{
			buffer[i]=SPI.transfer(RX_PIN, 0x00);
		}
	}
}

/****************************************************************
*FUNCTION NAME:SpiReadStatus
*FUNCTION     :CC1101 read status register
*INPUT        :addr: register address
*OUTPUT       :status value
****************************************************************/
byte ZUHF_CC1101::SpiReadStatus(byte addr) 
{
	byte value,temp;

	temp = addr | READ_BURST;
	value = SPI.transfer(RX_PIN, temp, SPI_CONTINUE);
	value = SPI.transfer(RX_PIN, 0x00);
	return value;
}

/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC1101 register config //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ZUHF_CC1101::RegConfigSettings(){
/* custom initial basic register settings for CC1101 
* Register Setup for
*  868e6 Hz
*  datarate 80kBaud
*  ASK/OOK modulation
*  no crc
*  no preamble
*  no sync
*  infinite packet length selected
*  manual calibration (once in setup) needs only be updated if power adjustments are done.
*/	

  
	SpiWriteReg(CC1101_IOCFG2,      0x03);  /* Associated to the TX FIFO: Asserts when TX FIFO is full. De-asserts when the 
                                          TX FIFO is drained below the TX FIFO
                                          threshold. */
	SpiWriteReg(CC1101_IOCFG0,      0x06);  /*Asserts when sync word has been sent / received, and de-asserts at the end of the packet. 
                                          In RX, the pin will also deassert when a packet is discarded due to address or maximum length 
                                          filtering or when the radio enters RXFIFO_OVERFLOW state. In TX the pin will de-assert if the 
                                          TX FIFO underflows. */ 
	SpiWriteReg(CC1101_PKTLEN,      0xFF);  //Packet Length
	SpiWriteReg(CC1101_PKTCTRL0,    0x00);  //Packet Automation Control // infinit packetlength
	SpiWriteReg(CC1101_PKTCTRL1,    0x00);
	SpiWriteReg(CC1101_CHANNR,      0x00);
	SpiWriteReg(CC1101_FSCTRL1,     0x06);  //Frequency Synthesizer Control
	SpiWriteReg(CC1101_FREQ2,       0x21);  //Frequency Control Word, High Byte
	SpiWriteReg(CC1101_FREQ1,       0x62);  //Frequency Control Word, Middle Byte
	SpiWriteReg(CC1101_FREQ0,       0x76);  //Frequency Control Word, Low Byte
	SpiWriteReg(CC1101_MDMCFG4,     0x8B);  //Modem Configuration  // DR 80kBaud
	SpiWriteReg(CC1101_MDMCFG3,     0x93);  //Modem Configuration // DR 80kBaud
	SpiWriteReg(CC1101_MDMCFG2,     0x30);  //Modem Configuration // ASK / OOK nopreamble nosync , no crc appended
	SpiWriteReg(CC1101_MDMCFG1,     0x00);  //channel spacing 
	SpiWriteReg(CC1101_MDMCFG0,     0xff);  // channel spacing 
	SpiWriteReg(CC1101_MCSM1,       0x30);  // default settings 
	SpiWriteReg(CC1101_MCSM0,       0x29);  //Main Radio Control State Machine Configuration
	SpiWriteReg(CC1101_FOCCFG,      0x1D);  //Frequency Offset Compensation Configuration
	SpiWriteReg(CC1101_BSCFG,       0x1C);  //Bit Synchronization Configuration
	
  SpiWriteReg(CC1101_AGCCTRL2,    0x03);  //AGC Control // mainly used for RX mode
	SpiWriteReg(CC1101_AGCCTRL1,    0x00);  //AGC Control // mainly used for RX mode
	SpiWriteReg(CC1101_AGCCTRL0,    0x90);  //AGC Control // mainly used for RX mode
	
  SpiWriteReg(CC1101_FREND1,      0x56);  //Front End RX Configuration
	SpiWriteReg(CC1101_FREND0,      0x11);  // the last two bits define the index from the patable to use
	SpiWriteReg(CC1101_FSCAL3,      0xEA);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL2,      0x2A);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL1,      0x00);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL0,      0x1F);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_TEST2,       0x81);  //Various Test Settings
	SpiWriteReg(CC1101_TEST0,       0x35);  //Various Test Settings
	SpiWriteReg(CC1101_TEST0,       0x09);  //Various Test Settings
	SpiWriteReg(CC1101_FIFOTHR,     0x07);  // tx threshold 33bytes
	
	SpiStrobe(CC1101_SCAL);
}


/****************************************************************
*FUNCTION NAME: UpdateFifo
*FUNCTION     : Add Data To Fifo When TX is in infiniteMode
*INPUT        : data: pointer to data; nbytes: number of bytes to write to fifobuffer
*OUTPUT       : none
****************************************************************/
/* updated 12.02.2021 */
void ZUHF_CC1101::UpdateFifo(byte *data, int nbytes)
{
  int index = 0;
  while (index < nbytes)
  {
    while(RX_GDO2_STATE);
    SpiWriteReg(CC1101_TXFIFO, data[index]);
    index++;
  }
}

/****************************************************************
* FUNCTION NAME: SendCW
* FUNCTION     : send continous high signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
* OUTPUT       : none
*****************************************************************/
void ZUHF_CC1101::SendCW(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
		UpdateFifo(CW, 1);
	}
}

/****************************************************************
* FUNCTION NAME: SendCW
* FUNCTION     : send continous high signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
*              : lastbyteduration: number of bits of last byte where 1 bit = 12.5µs (bigEndian notation)
* OUTPUT       : none
*****************************************************************/
/*
void ZUHF_CC1101::SendCW(byte duration, byte lastbyteduration)
{
	byte lastbyte = 0x00;
    byte mask = 0x80;    
    if (lastbyteduration < 8)
    {
        for (int j = 0; j < lastbyteduration; j++)
        {
            lastbyte = lastbyte | mask;
            mask = mask >> 1;
        }
        
        for (int i = 0; i < duration-1; i++)
	    {
            UpdateFifo(CW, 1);
        }
        UpdateFifo(&lastbyte, 1);
    }
    else
    {
        SendCW(duration);
    }
}
*/
/****************************************************************
* FUNCTION NAME: SendCWBlock
* FUNCTION     : send continous high signal of duration n
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
* OUTPUT       : none
*****************************************************************/
void ZUHF_CC1101::SendCWBurst(byte duration)
{
	SpiWriteBurstReg(CC1101_TXFIFO, CW, duration);
}

/****************************************************************
* FUNCTION NAME: SendIdle
* FUNCTION     : send continous low signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
 *OUTPUT       : none
****************************************************************/
void ZUHF_CC1101::SendIdle(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
		UpdateFifo(OFF, 1);
	}
}

/****************************************************************
* FUNCTION NAME: SendByte
* FUNCTION     : send single signal
* INPUT        : b: byte signal to send
 *OUTPUT       : none
****************************************************************/
/* use only if in TX mode 
 * definition for TX_GDO0/2_PIO and TX_GDO0/2_STATE needs to be added 
 * if used.
 */

void ZUHF_CC1101::SendByte(byte value)
{
	while(RX_GDO2_STATE);
  SpiWriteReg(CC1101_TXFIFO, value);
}


/****************************************************************
* FUNCTION NAME: DecodeFM0
* FUNCTION     : FM0 decodes a sequence of bits
* INPUT        : data: pointer to a byte array of data; data_size: number of bytes that should be decoded from data
* OUTPUT       : bitArray: pointer to an array where the result is saved as sequence of bits
****************************************************************/
/*  ATTENTION: It does not check if state is properly changed according to FM0 for each data unit yet. It assumes the
               provided data is a legitimate format. The lack of this check is basically compensated by the usually 
               obligatory CRC check when communicating with a tag.
 */
void ZUHF_CC1101::DecodeFM0(byte *bitArray, byte *data, byte data_size)
{
  byte bit_index = 0;
  byte mask = 0xC0; // 1100 0000
  byte FM0 = 0;
  byte prev = 0;
  byte curr = 0;
  for (byte i = 0; i < data_size; i++)
  {
    Serial.println(data[i],HEX);
    for (byte shift = 0; shift < 4; shift++)
    {
      FM0 = (data[i] & (mask >> (shift*2))) >> (6-(shift*2));
      switch(FM0){
        case 3:
          bitArray[(i*4)+shift] = 1;
          break;
        case 0:
          bitArray[(i*4)+shift] = 1;
          break;
        case 2:
          bitArray[(i*4)+shift] = 0;
          break;
        case 1:
          bitArray[(i*4)+shift] = 0;
          break;
        default:
          Serial.println("[ERROR] Error when decoding FM0 data!");
          break;
      }
    }
  }
}

ZUHF_CC1101 RX_UNIT;

#endif



