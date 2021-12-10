
/*
	CC1101_DUE
	Basic CC1101 library for Arduino Due. It makes use of the Arduino SPI.h library.
*/
#ifndef CC1101_CPP
#define CC1101_CPP

#include "CC1101.h"
#include "CC1101_REGS.h"


/****************************************************************
*FUNCTION NAME:SpiInit
*FUNCTION     :spi communication initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC1101::SpiInit()
{
#ifdef SAMX
  SPI.end();
  
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(RF_PIN, OUTPUT); // CSN / CSS / SS 
  
  /* ********************* */
  pinMode(4, OUTPUT); 
  pinMode(10, OUTPUT);
  pinMode(52, OUTPUT);
  
  /* Initialize SPI For CC1101 */
  SPI.begin(RF_PIN);
  SPI.setDataMode(RF_PIN, SPI_MODE0);
  SPI.setClockDivider(RF_PIN, 21); 	// 4MHz // default is 4MHz higher rates are possible but playing it safe here
  SPI.setBitOrder(RF_PIN, MSBFIRST);
#else
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(SS_PIN, OUTPUT);
  SpiMode(0);
#endif
}
#ifdef AVR
/****************************************************************
*FUNCTION NAME:SpiMode
*FUNCTION     :set spi mode
*INPUT        :        config               mode
			   (0<<CPOL) | (0 << CPHA)		 0
			   (0<<CPOL) | (1 << CPHA)		 1
			   (1<<CPOL) | (0 << CPHA)		 2
			   (1<<CPOL) | (1 << CPHA)		 3
*OUTPUT       :none
****************************************************************/
void CC1101::SpiMode(byte config)
{
  byte tmp;
  // enable SPI master with configuration byte specified
  SPCR = 0;
  SPCR = (config & 0x7F) | (1<<SPE) | (1<<MSTR);
  tmp = SPSR;
  tmp = SPDR;
}
#endif
/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0_TX,GDO2_TX pin
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void CC1101::GDO_Set (void)
{
#ifdef RF_STREAM
  // when using continous rx/tx mode on cc1101
	pinMode(GDO0_PIN, OUTPUT);
	pinMode(GDO2_PIN, INPUT); 
#else
  // when using packet mode on cc1101
	pinMode(GDO0_PIN, INPUT);
	pinMode(GDO2_PIN, INPUT);
#endif
}

/****************************************************************
*FUNCTION NAME:Reset
*FUNCTION     :CC1101 reset //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC1101::Reset(void)
{
#ifdef SAMX
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(MOSI_PIN, LOW);
    digitalWrite(RF_PIN, LOW);
    delay(1);
    digitalWrite(RF_PIN, HIGH);
    delay(1);
    digitalWrite(RF_PIN, LOW);
    while(digitalRead(MISO_PIN));
	SpiStrobe(CC1101_SRES);
    while(digitalRead(MISO_PIN));
    delay(1);
#else
	digitalWrite(SS_PIN, LOW);
	delay(1);
	digitalWrite(SS_PIN, HIGH);
	delay(1);
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(CC1101_SRES);
	while(digitalRead(MISO_PIN));
	digitalWrite(SS_PIN, HIGH);
#endif
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
byte CC1101::Init()
{
    SpiInit();						
    GDO_Set();										
    #ifdef AVR
        digitalWrite(SS_PIN, HIGH);
        digitalWrite(SCK_PIN, HIGH);
        digitalWrite(MOSI_PIN, LOW);
    #endif	
    Reset();
  
  // print out basic cc1101 info: version & partnum as a simple check
  // partnum should be 0x14 or 0x04; version is usually 0x00
  byte xversion = SpiReadStatus(CC1101_VERSION);
  byte xpart    = SpiReadStatus(CC1101_PARTNUM);
  #ifdef SAMX
    Serial.print("CSS PIN:     ");Serial.println(RF_PIN, DEC);
  #else
    Serial.print("CSS PIN:     ");Serial.println(SS_PIN, DEC);
  #endif  
  Serial.print("MOSI PIN:    ");Serial.println(MOSI_PIN, DEC);
  Serial.print("MISO PIN:    ");Serial.println(MISO_PIN, DEC);
  Serial.print("SCK PIN:     ");Serial.println(SCK_PIN, DEC);
  Serial.print("Version:     0x");Serial.println(xversion, HEX);
  Serial.print("Partnumber:  0x");Serial.println(xpart, HEX);
  
  RegConfigSettings();							
	SpiWriteBurstReg(CC1101_PATABLE,PaTable,8);
  SpiStrobe(CC1101_SCAL);
  return xversion;
}

#ifdef AVR
/****************************************************************
*FUNCTION NAME:SpiTransfer
*FUNCTION     :spi transfer
*INPUT        :value: data to send
*OUTPUT       :data to receive
****************************************************************/
byte CC1101::SpiTransfer(byte value)
{
  SPDR = value;
  while (!(SPSR & (1<<SPIF))) ;
  return SPDR;
}
#endif

/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC1101 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none
****************************************************************/
void CC1101::SpiWriteReg(byte addr, byte value)
{
#ifdef SAMX
  SPI.transfer(RF_PIN, addr, SPI_CONTINUE);
  SPI.transfer(RF_PIN, value);
#else
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(addr);
	SpiTransfer(value);
	digitalWrite(SS_PIN, HIGH);
#endif
}

/****************************************************************
*FUNCTION NAME:SpiWriteBurstReg
*FUNCTION     :CC1101 write burst data to register
*INPUT        :addr: register address; buffer:register value array; num:number to write
*OUTPUT       :none
****************************************************************/
void CC1101::SpiWriteBurstReg(byte addr, byte *buffer, byte num)
{
#ifdef SAMX
	byte i, temp;
	temp = addr | WRITE_BURST;
  SPI.transfer(RF_PIN, temp, SPI_CONTINUE);
  for (i = 0; i < num; i++)
 	{
      if (i == (num - 1)){
		  SPI.transfer(RF_PIN, buffer[i]);
		}else{
			SPI.transfer(RF_PIN, buffer[i], SPI_CONTINUE);
		}
	}
#else
	byte i, temp;
	temp = addr | WRITE_BURST;
  digitalWrite(SS_PIN, LOW);
  while(digitalRead(MISO_PIN));
  SpiTransfer(temp);
  for (i = 0; i < num; i++)
 	{
    SpiTransfer(buffer[i]);
  }
  digitalWrite(SS_PIN, HIGH);
#endif
}

/****************************************************************
*FUNCTION NAME:SpiStrobe
*FUNCTION     :CC1101 Strobe
*INPUT        :strobe: command; //refer define in CC1101.h//
*OUTPUT       :none
****************************************************************/
void CC1101::SpiStrobe(byte strobe)
{
#ifdef SAMX
	SPI.transfer(RF_PIN, strobe);
#else
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(strobe);
	digitalWrite(SS_PIN, HIGH);
#endif
}

/****************************************************************
*FUNCTION NAME:SpiReadReg
*FUNCTION     :CC1101 read data from register
*INPUT        :addr: register address
*OUTPUT       :register value
****************************************************************/
byte CC1101::SpiReadReg(byte addr) 
{
#ifdef SAMX
	byte temp, value;
  temp = addr|READ_SINGLE;
	value = SPI.transfer(RF_PIN, temp, SPI_CONTINUE);
	value = SPI.transfer(RF_PIN, 0x00);
	return value;
#else
	byte temp, value;
    temp = addr|READ_SINGLE;
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(temp);
	value=SpiTransfer(0);
	digitalWrite(SS_PIN, HIGH);
	return value;
#endif
}

/****************************************************************
*FUNCTION NAME:SpiReadBurstReg
*FUNCTION     :CC1101 read burst data from register
*INPUT        :addr: register address; buffer:array to store register value; num: number to read
*OUTPUT       :none
****************************************************************/
void CC1101::SpiReadBurstReg(byte addr, byte *buffer, byte num)
{
#ifdef SAMX
	byte i,temp;
	temp = addr | READ_BURST;
	SPI.transfer(RF_PIN, temp, SPI_CONTINUE);
	for(i=0;i<num;i++)
	{
		if (i < (num-1)) {
			buffer[i]=SPI.transfer(RF_PIN, 0x00, SPI_CONTINUE);
		}else{
			buffer[i]=SPI.transfer(RF_PIN, 0x00);
		}
	}
#else
	byte i,temp;

	temp = addr | READ_BURST;
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(temp);
	for(i=0;i<num;i++)
	{
		buffer[i]=SpiTransfer(0);
	}
	digitalWrite(SS_PIN, HIGH);
#endif
}

/****************************************************************
*FUNCTION NAME:SpiReadStatus
*FUNCTION     :CC1101 read status register
*INPUT        :addr: register address
*OUTPUT       :status value
****************************************************************/
byte CC1101::SpiReadStatus(byte addr) 
{
#ifdef SAMX
	byte value,temp;

	temp = addr | READ_BURST;
	value = SPI.transfer(RF_PIN, temp, SPI_CONTINUE);
	value = SPI.transfer(RF_PIN, 0x00);
	return value;
#else
	byte value,temp;

	temp = addr | READ_BURST;
	digitalWrite(SS_PIN, LOW);
	while(digitalRead(MISO_PIN));
	SpiTransfer(temp);
	value=SpiTransfer(0);
	digitalWrite(SS_PIN, HIGH);
	return value;
#endif
}

/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC1101 register config //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void CC1101::RegConfigSettings(){
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
	SpiWriteReg(CC1101_BSCFG,       0x1F);  //Bit Synchronization Configuration
	
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
void CC1101::UpdateFifo(byte *data, int nbytes)
{
  int index = 0;
  while (index < nbytes)
  {
    while(GDO2_STATE);
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
void CC1101::SendCW(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
		UpdateFifo(CW, 1);
	}
}

/****************************************************************
* FUNCTION NAME: SendCWBlock
* FUNCTION     : send continous high signal of duration n
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
* OUTPUT       : none
*****************************************************************/
void CC1101::SendCWBurst(byte duration)
{
	SpiWriteBurstReg(CC1101_TXFIFO, CW, duration);
}

/****************************************************************
* FUNCTION NAME: SendIdle
* FUNCTION     : send continous low signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
 *OUTPUT       : none
****************************************************************/
void CC1101::SendIdle(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
		UpdateFifo(OFF, 1);
	}
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
void CC1101::DecodeFM0(byte *bitArray, byte *data, byte data_size)
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



CC1101 RFMOD;

#endif



