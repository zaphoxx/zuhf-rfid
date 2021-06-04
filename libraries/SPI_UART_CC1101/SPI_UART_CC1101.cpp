/* SPI_UART_CC1101.cpp
 * The Code Programs the Arduino Due to use the Serial2 USART as SPI
 * From https://forum.arduino.cc/index.php?topic=283766.0 I used some
 * of the code snippets for that.
 * The Library is build to communicate with a CC1101 and not an all purpose
 * SPI_UART Library. But feel free to copy and use the code as needed.
 * The Library makes the following assumptions
 * CC1101 is used as TX Module (w/ Arduino Due Atmel SAM3X8E)
 *  --> CSS Pin is called TX_PIN
 *  --> Clock Divider 
 *  --> MOSI : TX2 (A.12)
 *  --> MISO : RX2 (A.13)
 *  --> SCK  : SCK1/A0 (A.16)
 *  --> CSS  : PIN 23 (A.14)
 */
 
#ifndef SPI_UART_CC1101_cpp
#define SPI_UART_CC1101_cpp


#include <SPI_UART_CC1101.h>
#include <ZUHF_CC1101_REGS.h>

/****************************************************************/
#define 	WRITE_BURST     	0x40						//write burst
#define 	READ_SINGLE     	0x80						//read single
#define 	READ_BURST      	0xC0						//read burst
#define 	BYTES_IN_RXFIFO   0x7F  				  //byte number in RXfifo
#define		BYTES_IN_TXFIFO		0x7F						//byte number in TXfifo
/****************************************************************/



/****************************************************************
* FUNCTION NAME: Init
* FUNCTION     : configure USASRT1 for usage as SPI and Initialize CC1101
* INPUT        : None
* OUTPUT       : version of the CC1101 for verification
****************************************************************/
byte SPI_UART_CC1101::Init(void)
{
	for (int i = 0; i < 64; i++){
		CW[i] = 0xff;
	}
	
	pmc_enable_periph_clk(ID_USART1);
	USART1->US_WPMR = 0x55534100;   // Unlock the USART Mode register
	USART1->US_CR = US_CR_RSTRX | US_CR_RSTTX;
	PIOA->PIO_WPMR = 0x50494F00;    // Unlock PIOA Write Protect Mode Register
	PIOB->PIO_WPMR = 0x50494F00;    // Unlock PIOB Write Protect Mode Register
	PIOA->PIO_ABSR |= (0u << 14);   // CS: Assign A14 I/O to the Peripheral A function
	PIOA->PIO_PDR |= (1u << 14);    // CS: Disable PIO control, enable peripheral control
	PIOA->PIO_ABSR |= (0u << 16);   // SCK: Assign A16 I/O to the Peripheral A function
	PIOA->PIO_PDR |= (1u << 16);    // SCK: Disable PIO control, enable peripheral control
	PIOA->PIO_ABSR |= (0u << 13);   // MOSI: Assign PA13 I/O to the Peripheral A function
	PIOA->PIO_PDR |= (1u << 13);    // MOSI: Disable PIO control, enable peripheral control
	PIOA->PIO_ABSR |= (0u << 12);   // MISO: Assign A12 I/O to the Peripheral A function
	PIOA->PIO_PDR |= (1u << 12);    // MISO: Disable PIO control, enable peripheral control
	
  USART1->US_MR = 0x409CE; // MODE0, 8_BIT, SPI_MODE
  USART1->US_BRGR = 21; 
  
  USART1->US_CR = US_CR_RXEN;
  USART1->US_CR = US_CR_TXEN;

  // SET GDO PIN MODES
  pinMode(TX_GDO0_PIN, INPUT);
  pinMode(TX_GDO2_PIN, INPUT);
  //pinMode(TX_SS_PIN, OUTPUT);
  // RESET CC1101
  Serial.println("[TX] CC1101 RESET *");
  digitalWrite(TX_SS_PIN, LOW); // CSS LOW
  delay(1);
  digitalWrite(TX_SS_PIN, HIGH); // CSS HIGH
  delay(1);
  digitalWrite(TX_SS_PIN, LOW); // CSS LOW
  while (((USART1->US_CSR) & 0x01)); // WAIT FOR MISO LOW
  SpiStrobe(CC1101_SRES);
  while (((USART1->US_CSR) & 0x01)); // WAIT FOR MISO LOW
  /* !!! DELAY NECESSARY TO COMPLETE RESET !!! */
  delay(1);
  Serial.println("[TX] CC1101 RESET OK");
  /* *** RESET COMPLETE *** */
  
  // SET CONFIGURATION REGISTERS CC1101
  RegConfigSettings();
  SpiWriteBurstReg(CC1101_PATABLE,TXPaTable,8);
  SpiStrobe(CC1101_SCAL);
  
  // print out basic cc1101 info: version & partnum as a simple check
  // partnum should be 0x14 or 0x04; version is usually 0x00
  byte xversion = SpiReadStatus(CC1101_VERSION);
  byte xpart    = SpiReadStatus(CC1101_PARTNUM);
  Serial.print("[TX] Version:     0x");Serial.println(xversion, HEX);
  Serial.print("[TX] Partnumber:  0x");Serial.println(xpart, HEX);
  return xversion;
}


/****************************************************************
* FUNCTION NAME: Transfer
* FUNCTION     : Send byte via SPI
* INPUT        : b: byte value to send
* OUTPUT       : byte value: response from SPI
****************************************************************/
byte SPI_UART_CC1101::Transfer(byte b)
{
  USART1->US_CR = US_CR_RXEN;
  USART1->US_CR = US_CR_TXEN;
	// wait for transmitter ready
	while (!(USART1->US_CSR  & US_CSR_TXRDY));
	USART1->US_THR = b;
	// wait for receiver ready
	while (!((USART1->US_CSR) & 0x01));
	byte result = USART1->US_RHR;
	return result;
}


/****************************************************************
* FUNCTION NAME: RegCmd
* FUNCTION     : Send register command to CC1101
* INPUT        : cmd: register address, val: value to send with the command
* OUTPUT       : byte value: response from the CC1101
****************************************************************/
byte SPI_UART_CC1101::RegCmd(byte cmd, byte val)
{
	USART1->US_CR = US_CR_RXEN;
	USART1->US_CR = US_CR_TXEN;
	// MANUALLY KEEP CS LOW UNTIL CMD AND VALUE HAVE BEEN TRANSFERRED
	// DATASHEET (pg. 805, 35.7.7.5 Character Transmission)
	USART1->US_CR = US_CR_RTSEN;
	// WAIT FOR TRANSMITTER TO BE READY
	while (!(USART1->US_CSR  & US_CSR_TXRDY));
	USART1->US_THR = cmd;
	
	while (!(USART1->US_CSR  & US_CSR_TXRDY));
	USART1->US_THR = val;
  
	// WAIT FOR ALL DATA TO BE TRANSMITTED
	while (!(USART1->US_CSR & US_CSR_TXEMPTY));
	// MANUALLY PULL CS HIGH AGAIN
	USART1->US_CR = US_CR_RTSDIS;
  
	// wait for receiver ready
	while (!(USART1->US_CSR  & US_CSR_RXRDY));
	byte result = USART1->US_RHR;
	return result;
}


/****************************************************************
* FUNCTION NAME: ReadStatus
* FUNCTION     : CC1101 read status register
* INPUT        : addr: register address
* OUTPUT       : status value
****************************************************************/
byte SPI_UART_CC1101::SpiReadStatus(byte addr) 
{
	byte value,temp;
	temp = addr | READ_BURST;
	value = RegCmd(temp, 0x00);
	return value;
}


/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC1101 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none (return value from RegCmd is not returned)
****************************************************************/
void SPI_UART_CC1101::SpiWriteReg(byte addr, byte value)
{
	byte dummy = RegCmd(addr, value);
}


/****************************************************************
* FUNCTION NAME: SpiWriteBurstReg
* FUNCTION     : CC1101 write burst data to register
* INPUT        : addr: register address; buffer:register value array; num:number to write
* OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SpiWriteBurstReg(byte addr, byte *buffer, byte num)
{
  USART1->US_CR = US_CR_RXEN;
  USART1->US_CR = US_CR_TXEN;
	byte i, temp;
	temp = addr | WRITE_BURST;
  // MANUALLY KEEP CS LOW UNTIL CMD AND VALUE HAVE BEEN TRANSFERRED
  // DATASHEET (pg. 805, 35.7.7.5 Character Transmission)
  USART1->US_CR = US_CR_RTSEN;
  // SEND COMMAND
  while (!(USART1->US_CSR  & US_CSR_TXRDY));
  USART1->US_THR = temp;
  // WAIT FOR TRANSMITTER TO BE READY
	for (i = 0; i < num; i++){
    while (!(USART1->US_CSR  & US_CSR_TXRDY));
    USART1->US_THR = buffer[i];
  }
  // WAIT FOR ALL DATA TO BE TRANSMITTED
	while (!(USART1->US_CSR & US_CSR_TXEMPTY));
  // MANUALLY PULL CS HIGH AGAIN
  USART1->US_CR = US_CR_RTSDIS;
}


/****************************************************************
* FUNCTION NAME: SpiStrobe
* FUNCTION     : CC1101 Strobe
* INPUT        : strobe: command; //refer define in CC1101.h//
* OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SpiStrobe(byte strobe)
{
	byte dummy = Transfer(strobe);
}


/****************************************************************
* FUNCTION NAME: SpiReadReg
* FUNCTION     : CC1101 read data from register
* INPUT        : addr: register address
* OUTPUT       : register value
****************************************************************/
byte SPI_UART_CC1101::SpiReadReg(byte addr) 
{
	byte temp, value;
  temp = addr | READ_SINGLE;
	value = Transfer(temp);
	return value;	
}


/****************************************************************
* FUNCTION NAME: SpiReadBurstReg
* FUNCTION     : CC1101 read burst data from register
* INPUT        : addr: register address; buffer:array to store register value; num: number to read
* OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SpiReadBurstReg(byte addr, byte *buffer, byte num)
{
  USART1->US_CR = US_CR_RXEN;
  USART1->US_CR = US_CR_TXEN;
	byte temp;
	temp = addr | READ_BURST;
  // MANUALLY KEEP CS LOW UNTIL CMD AND VALUE HAVE BEEN TRANSFERRED
  // DATASHEET (pg. 805, 35.7.7.5 Character Transmission)
  USART1->US_CR = US_CR_RTSEN;
  // SEND COMMAND
  while (!(USART1->US_CSR  & US_CSR_TXRDY));
  USART1->US_THR = temp;
  
	for (byte i = 0; i < num; i++){
    // WAIT FOR ALL DATA TO BE TRANSMITTED
    while (!(USART1->US_CSR & US_CSR_TXEMPTY));
    USART1->US_THR = 0x00;
    // wait for receiver ready
    while (!(USART1->US_CSR  & US_CSR_RXRDY));
    buffer[i] = USART1->US_RHR;
  }
  USART1->US_CR = US_CR_RTSDIS;
}


/****************************************************************
* FUNCTION NAME: RegConfigSettings
* FUNCTION     : CC1101 register config //details refer datasheet of CC1101/CC1100//
* INPUT        : none
 *OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::RegConfigSettings(void){
	/* ******************************************************************************
   * custom initial basic register settings for CC1101	
	 * for ASK/OOK Settings also see https://www.ti.com/lit/an/swra215e/swra215e.pdf
   * ****************************************************************************** */
	SpiWriteReg(CC1101_MDMCFG4,   0x8B); // 203kHz Filter Bandwidth
    //SpiWriteReg(CC1101_MDMCFG4,   0x5B); // 325kHz Filter Bandwidth
	SpiWriteReg(CC1101_FSCTRL1,   0x0F);
	SpiWriteReg(CC1101_FREND1,    0xB6);
	SpiWriteReg(CC1101_TEST2,     0x81);
	SpiWriteReg(CC1101_TEST1,     0x35);
	SpiWriteReg(CC1101_FIFOTHR,   0x0f);
	SpiWriteReg(CC1101_AGCCTRL2,  0x03);  
	SpiWriteReg(CC1101_AGCCTRL1,  0x00);
	SpiWriteReg(CC1101_AGCCTRL0,  0x91);

	SpiWriteReg(CC1101_FREND0,    0x11); 
	SpiWriteReg(CC1101_PKTCTRL0,  0x02);
	SpiWriteReg(CC1101_IOCFG0,    0x02);  
	SpiWriteReg(CC1101_MDMCFG2,   0x30); // ASK/OOK w/o sync+preamble
	
	SpiWriteReg(CC1101_SYNC1,     0xAD); 
	SpiWriteReg(CC1101_SYNC0,     0x23); 
	SpiWriteReg(CC1101_IOCFG2,    0x03);
	SpiWriteReg(CC1101_PKTLEN,    0x0C);  
	SpiWriteReg(CC1101_PKTCTRL1,  0x00);
	SpiWriteReg(CC1101_CHANNR,    0x00);
	/************* 868 MHz ************/
	SpiWriteReg(CC1101_FREQ2,     0x21);   //Frequency Control Word, High Byte
	SpiWriteReg(CC1101_FREQ1,     0x62);   //Frequency Control Word, Middle Byte
	SpiWriteReg(CC1101_FREQ0,     0x76);   //Frequency Control Word, Low Byte
	/************* ******* ************/
	SpiWriteReg(CC1101_MDMCFG3,   0x93); //Modem Configuration // DR 80kBaud
	SpiWriteReg(CC1101_MDMCFG1,   0x22); //channel spacing 
	SpiWriteReg(CC1101_MDMCFG0,   0xff); // channel spacing 
	SpiWriteReg(CC1101_MCSM1,     0x30);   // default settings 
	SpiWriteReg(CC1101_MCSM0,     0x29);   //Main Radio Control State Machine Configuration
	SpiWriteReg(CC1101_FOCCFG,    0x1D);  //Frequency Offset Compensation Configuration
	SpiWriteReg(CC1101_BSCFG,     0x1C);   //Bit Synchronization Configuration
	SpiWriteReg(CC1101_FSCAL3,    0xEA);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL2,    0x2A);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL1,    0x00);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_FSCAL0,    0x1F);  //Frequency Synthesizer Calibration
	SpiWriteReg(CC1101_TEST0,     0x09);   //Various Test Settings
	
  SpiStrobe(CC1101_SCAL);
}


/****************************************************************
* FUNCTION NAME: UpdateFifo
* FUNCTION     : Write n Bytes to TX Fifo
* INPUT        : *data: pointer to a byte buffer; nbytes: total number of bytes to write to TX Fifo 
 *OUTPUT       : none
****************************************************************/
/* updated 12.02.2021 8:44 - changed SpiWriteBurstReg to SpiWriteReg for performance */
void SPI_UART_CC1101::UpdateFifo(byte *data, int nbytes)
{
  int index = 0;
  while (index < nbytes)
  {
	/* wait if tx fifo is full */  
    while(TX_GDO2_STATE);
	SpiWriteReg(CC1101_TXFIFO,data[index]);
    index++;
  }
}

/****************************************************************
* FUNCTION NAME: SendCW
* FUNCTION     : send continous high signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
 *OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SendCW(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
		UpdateFifo(CW, 1);
	}
}

/* duration < 64 */
void SPI_UART_CC1101::SendCWBurst(byte duration)
{
	/* limit duration < 64 */
	duration = duration & 0x3f;
	SpiWriteBurstReg(CC1101_TXFIFO, CW, duration);
}

/****************************************************************
* FUNCTION NAME: SendIdle
* FUNCTION     : send continous low signal
* INPUT        : duration: number of bytes to send where 1 byte corresponds 8*12.5µs = 100µs
 *OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SendIdle(byte duration)
{
	for (int i = 0; i < duration; i++)
	{
    while(digitalRead(TX_GDO2_PIN));
		UpdateFifo(IDLE, 1);
	}
}

/****************************************************************
* FUNCTION NAME: SendByte
* FUNCTION     : send single signal
* INPUT        : b: byte signal to send
 *OUTPUT       : none
****************************************************************/
void SPI_UART_CC1101::SendByte(byte b)
{
	SpiWriteReg(CC1101_FIFOTHR, b);
}
SPI_UART_CC1101 TX_UNIT;
#endif
