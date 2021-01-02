#include <RFID_CC1101.h> //Download original library here: http://electronoobs.com/eng_arduino_RFID_cc1101.php
#include <zuhf_vars.h>
#include <zuhf_buffer.h>

#define DEBUG 0
#define MAXQUERY 88
#define QUERYSIZE 22 
#define MAXROUNDS 16
#define CW_SIZE 4
#define IDLE_SIZE 10
// additional global vars

byte paTable[8] = {0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00}; 
byte ack[] = {0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0};

DATA_BUFFER databuffer;
byte QUERY[QUERYSIZE];
byte ENCODED_QUERY[MAXQUERY];
bool ACK_FLAG = 0;
byte encoded_size = 0;
byte check = 0;
byte FIFOBUFFER[512];
int  buffer_size = 0;
int  irounds = 0;
byte CW[CW_SIZE]; // 250µs
byte OFF[IDLE_SIZE];
byte CRC[5] = {0,0,0,0,0};


void setup()
{
  Serial.begin(9600);
  /* basic initialization of CC1101 */
  RFID_cc1101.Init();
  InitRegisters();
  /* custom initial register settings for CC1101 
   * Register Setup for
   *  910e6 Hz
   *  datarate 80kBaud
   *  ASK/OOK modulation
   *  no crc
   *  no preamble
   *  no sync
   *  infinite packet length selected
   *  manual calibration (once in setup) needs only be updated if power adjustments are done.
   */

   for (int i = 0; i < CW_SIZE; i++)
   {
    CW[i] = 0xff;
   }
   for (int i = 0; i < IDLE_SIZE; i++)
   {
    OFF[i] = 0x00;
   }
}


void loop()
{
  //reader_state = R_TEST;
      // calibrate cc1101
    RFID_cc1101.SpiStrobe(CC1101_SCAL);
    // turn on tx 
    RFID_cc1101.SpiStrobe(CC1101_STX);
    // send cw for tag activation
    update_fifo(TAGSETTLE, sizeof(TAGSETTLE));
  while (reader_state != R_WAIT)
  {
    switch(reader_state)
    {
      case R_START:
        irounds = 0;
        ACK_FLAG = 0;
        databuffer.data_size = 0;
        reader_state = R_QUERY;
        break;
      case R_QUERY:
        // 1) add preamble
        check = copy_to_buffer(&databuffer, PREAMBLE, sizeof(PREAMBLE));
        // 2) build query
        build_query(0);
        // 3a) update crc5
        crc5_append(QUERY, QUERYSIZE);
        // 3b) encode query
        encoded_size = encode_data(ENCODED_QUERY, QUERY, QUERYSIZE);
        // 4) add ENCODED_QUERY to databuffer
        check = copy_to_buffer(&databuffer, ENCODED_QUERY, encoded_size);
        for (byte i = 0; i < 125; i++)
          {
            byte cw[1] = {1};
            check = copy_to_buffer(&databuffer, cw, 1);
          }
        
        // 6) translate final query into bytes for fifo
        buffer_size = generate_bytes(FIFOBUFFER, databuffer.data, databuffer.data_size); 
        // 7) send data to fifo
/*
        Serial.println("databuffer");
        for (int i = 0; i < databuffer.data_size; i++)
        {
          if (((i+1) % 8) == 0) Serial.print(" ");
          Serial.print(databuffer.data[i],DEC);
        }
        Serial.println();
        Serial.println("fifobuffer");
        buffer_size = generate_bytes (FIFOBUFFER, databuffer.data, databuffer.data_size);
        for (int i = 0; i < buffer_size;i++)
        {
          Serial.print(FIFOBUFFER[i],BIN);
          Serial.print(" ");
        }
        Serial.println();
*/        
        update_fifo(FIFOBUFFER, buffer_size);
        // 8) switch to next state 
        reader_state = R_CW;
        break;

      case R_CW:
        //update_fifo(CW,CW_SIZE);
        //Serial.println("[CW]");
        if (ACK_FLAG)
        {
          //Serial.println("--> ACK");
          //Serial.println("[-> ACK]");
          reader_state = R_ACK;
        }else{
          if (irounds < MAXROUNDS)
          {
            //Serial.println("[-> QUERYREP]");
            reader_state = R_QUERYREP;
          }else{
            //Serial.println("[-> START]");
            reader_state = R_START;
          }
        }
        break;
      case R_QUERYREP:
        if (irounds < 1)
        {
          databuffer.data_size = 0;
          check = copy_to_buffer(&databuffer, FRAMESYNC, sizeof(FRAMESYNC));
          check = copy_to_buffer(&databuffer, DATA0, sizeof(DATA0));
          check = copy_to_buffer(&databuffer, DATA0, sizeof(DATA0));
          check = copy_to_buffer(&databuffer, DATA0, sizeof(DATA0));
          check = copy_to_buffer(&databuffer, DATA0, sizeof(DATA0));
          for (byte i = 0; i < 100; i++)
          {
            byte cw[1] = {1};
            check = copy_to_buffer(&databuffer, cw, 1);
          }
        }
        buffer_size = generate_bytes(FIFOBUFFER, databuffer.data, databuffer.data_size);
        update_fifo(FIFOBUFFER, buffer_size);
        irounds++;
        reader_state = R_CW;
        break;
        
      case R_ACK:
        // send a dummy ack
        buffer_size = generate_bytes(FIFOBUFFER, ack, sizeof(ack));
        update_fifo(FIFOBUFFER, buffer_size);
        reader_state = R_CW;
        ACK_FLAG = 0;
        break;
        
      case R_POWERDOWN:
        //while(digitalRead(!GDO0));
        //update_fifo(FIFOBUFFER, buffer_size);
        //while(digitalRead(!GDO0));
        while(digitalRead(GDO0));
        RFID_cc1101.SpiStrobe(CC1101_SFTX);
        delay(5);
        reader_state = R_START;
        break;
      
      default:
        Serial.println("-> [WAIT]");
        reader_state = R_START;
        break;
    }   
  }
}

/*  */
byte build_query()
{
  build_query(0);
}

byte build_query(byte s)
{
  byte pos=0;
  // QUERY = QUERY_CODE + DR + M + TREXT + SEL_ALL + SESSION + TARGET + Q + CRC5
  memcpy(QUERY, &QUERY_CODE, sizeof(QUERY_CODE));
  pos += sizeof(QUERY_CODE);
  memcpy(QUERY+pos, &DR, sizeof(DR));
  pos += sizeof(DR);
  memcpy(QUERY+pos, &M, sizeof(M));
  pos += sizeof(M);
  memcpy(QUERY+pos, &TREXT, sizeof(TREXT));
  pos += sizeof(TREXT);
  memcpy(QUERY+pos, &SEL_ALL, sizeof(SEL_ALL));
  pos += sizeof(SEL_ALL);
  memcpy(QUERY+pos, &SESSION[s], sizeof(SESSION[s]));
  pos += sizeof(SESSION[s]);
  memcpy(QUERY+pos, &TARGET, sizeof(TARGET));
  pos += sizeof(TARGET);
  memcpy(QUERY+pos, &Q_VALUE[FIXED_Q], 4);
  pos += 4;
  memcpy(QUERY+pos, &CRC, 5);
  pos += 5;
  return pos; // 
}


void InitRegisters()
{
  RFID_cc1101.SpiWriteBurstReg(CC1101_PATABLE, paTable, 8);
  //RFID_cc1101.SpiWriteReg(CC1101_IOCFG2,0x29); // 
  //RFID_cc1101.SpiWriteReg(CC1101_IOCFG1,0x06);  // not connected
  RFID_cc1101.SpiWriteReg(CC1101_IOCFG2,0x03);  /* Associated to the TX FIFO: Asserts when TX FIFO is full. De-asserts when the 
                                                   TX FIFO is drained below the TX FIFO
                                                   threshold. */
  RFID_cc1101.SpiWriteReg(CC1101_IOCFG0,0x06);  /*Asserts when sync word has been sent / received, and de-asserts at the end of the packet. 
                                                  In RX, the pin will also deassert when a packet is discarded due to address or maximum length 
                                                  filtering or when the radio enters RXFIFO_OVERFLOW state. In TX the pin will de-assert if the 
                                                  TX FIFO underflows. */ 
  RFID_cc1101.SpiWriteReg(CC1101_PKTLEN,0x00);  //Packet Length
  RFID_cc1101.SpiWriteReg(CC1101_PKTCTRL0,0x02);//Packet Automation Control // infinit packetlength
  RFID_cc1101.SpiWriteReg(CC1101_PKTCTRL1,0x00);
  RFID_cc1101.SpiWriteReg(CC1101_FSCTRL1,0x06); //Frequency Synthesizer Control
  RFID_cc1101.SpiWriteReg(CC1101_FREQ2,0x21);   //Frequency Control Word, High Byte
  RFID_cc1101.SpiWriteReg(CC1101_FREQ1,0x62);   //Frequency Control Word, Middle Byte
  RFID_cc1101.SpiWriteReg(CC1101_FREQ0,0x76);   //Frequency Control Word, Low Byte
  RFID_cc1101.SpiWriteReg(CC1101_MDMCFG4,0x5b); //Modem Configuration  // DR 80kBaud
  RFID_cc1101.SpiWriteReg(CC1101_MDMCFG3,0xa7); //Modem Configuration // DR 80kBaud
  RFID_cc1101.SpiWriteReg(CC1101_MDMCFG2,0x30); //Modem Configuration // ASK / OOK nopreamble nosync , no crc appended
  RFID_cc1101.SpiWriteReg(CC1101_MDMCFG1,0x00); //channel spacing 
  RFID_cc1101.SpiWriteReg(CC1101_MDMCFG0,0xff); // channel spacing 
  //RFID_cc1101.SpiWriteReg(CC1101_MCSM1,0x32);   //Main Radio Control State Machine Configuration
  //RFID_cc1101.SpiWriteReg(CC1101_MCSM1,0x0F);   //configured to always stay in TX state
  RFID_cc1101.SpiWriteReg(CC1101_MCSM1,0x30);   // default settings 
  RFID_cc1101.SpiWriteReg(CC1101_MCSM0,0x29);   //Main Radio Control State Machine Configuration
  RFID_cc1101.SpiWriteReg(CC1101_FOCCFG,0x1D);  //Frequency Offset Compensation Configuration
  RFID_cc1101.SpiWriteReg(CC1101_BSCFG,0x1C);   //Bit Synchronization Configuration
  RFID_cc1101.SpiWriteReg(CC1101_AGCCTRL2,0x07);//AGC Control // mainly used for RX mode
  RFID_cc1101.SpiWriteReg(CC1101_AGCCTRL1,0x00);//AGC Control // mainly used for RX mode
  RFID_cc1101.SpiWriteReg(CC1101_AGCCTRL0,0x92);//AGC Control // mainly used for RX mode
  RFID_cc1101.SpiWriteReg(CC1101_FREND1,0x56);  //Front End RX Configuration
  RFID_cc1101.SpiWriteReg(CC1101_FREND0,0x11);  // the last two bits define the index from the patable to use
  RFID_cc1101.SpiWriteReg(CC1101_FSCAL3,0xEA);  //Frequency Synthesizer Calibration
  RFID_cc1101.SpiWriteReg(CC1101_FSCAL2,0x2A);  //Frequency Synthesizer Calibration
  RFID_cc1101.SpiWriteReg(CC1101_FSCAL1,0x00);  //Frequency Synthesizer Calibration
  RFID_cc1101.SpiWriteReg(CC1101_FSCAL0,0x1F);  //Frequency Synthesizer Calibration
  RFID_cc1101.SpiWriteReg(CC1101_TEST2,0x81);   //Various Test Settings
  RFID_cc1101.SpiWriteReg(CC1101_TEST0,0x35);   //Various Test Settings
  RFID_cc1101.SpiWriteReg(CC1101_TEST0,0x09);   //Various Test Settings
  RFID_cc1101.SpiWriteReg(CC1101_FIFOTHR,0x07); // tx threshold 33bytes
   // manual calibration trigger
  RFID_cc1101.SpiStrobe(CC1101_SCAL);
}


void update_fifo(byte *data, int nbytes)
{
  int index = 0;
  while (index < nbytes)
  {
    while(digitalRead(GDO2)); // if/while fifo full pause (de-asserts when below threshold
    RFID_cc1101.SpiWriteBurstReg(CC1101_TXFIFO,&data[index],1);
    index++;
  }
}


void crc5_append(byte *q, byte q_size)
{
  byte crc[] = {1,0,0,1,0};
  for(byte i = 0; i < 17; i++)
  {
    byte tmp[] = {0,0,0,0,0};
    tmp[4] = crc[3];
    if(crc[4] == 1)
    {
      if (q[i] == 1)
      {
        tmp[0] = 0;
        tmp[1] = crc[0];
        tmp[2] = crc[1];
        tmp[3] = crc[2];
      }
      else
      {
        tmp[0] = 1;
        tmp[1] = crc[0];
        tmp[2] = crc[1];
        if(crc[2] == 1)
        {
          tmp[3] = 0;
        }
        else
        {
          tmp[3] = 1;
        }
      }
    }
    else
    {
      if (q[i] == 1)
      {
        tmp[0] = 1;
        tmp[1] = crc[0];
        tmp[2] = crc[1];
        if(crc[2] == 1)
        {
          tmp[3] = 0;
        }
        else
        {
          tmp[3] = 1;
        }
      }
      else
      {
        tmp[0] = 0;
        tmp[1] = crc[0];
        tmp[2] = crc[1];
        tmp[3] = crc[2];
      }
    }
    memcpy(&crc,&tmp,5);
  }
  for (int i = 0; i < 5; i++){
    memcpy(q+17+i, &crc[4-i], 1);
  }
}


/*  
 *  translate the array of data bits into an actual payload that can be send to Fifo
 *  parameters: 
 *    payload - pointer to an array which should hold the final payload
 *    command - pointer to the command that needs translation
 *    command_size - size of the command to be translated
 *    !!! sizeof(command) != command_size, sizeof(command) >= command_size !!!
 *    use command_size to translate the actual number of bits
 */
int generate_bytes(byte *payload, byte *command, byte command_size)
{
  int R = command_size % 8;
  int N = command_size / 8;

  byte block = 0;

  for (int i = 0; i < N ; i++)
  {
    for (byte k = 0; k < 8; k++)
    {  
      block +=  (command[(i*8)+k] * power((7-k)));  
    }
    payload[i] = block;
    block = 0x00;
  }
  if (R > 0)
  {
    //Serial.println(R,DEC);
    block = 0x00;
    for (int j = 0; j < R; j++)
    {
      block = block + (command[N*8+j] * power(7-j));
    }
    for (int k = 0; k <= (7-R); k++)
    {
      block += power(k);
    }
  }
  payload[N] = block;
  return N;
}


/* power(int exponent) 
 * calculates y=2^exponent
 * parameters: exponent of type int 
 * returns result of type int
 * 
 */
int power(int exponent)
{
  int result=0;
  switch(exponent){
    case 0:
      return 1;
      break;
    default:
      result = 1;
      for (int i = 0; i < exponent; i++)
      {
        result *= 2;
      }
      break;
  }
  return result;
}


// generate query sequence pie encoded based on bit sequence provided
int encode_data(byte *encoded, const byte *data, byte data_size)
{
  byte index = 0;
  for (byte b = 0 ; b < data_size; b++)
  {
    if (data[b] == 0)
    {
      memcpy(encoded + index, DATA0, sizeof DATA0);
      index += sizeof DATA0;
    }
    if (data[b] == 1)
    {
      memcpy(encoded + index, DATA1, sizeof DATA1);
      index += sizeof DATA1;
    }
  } 
  return index;
}
