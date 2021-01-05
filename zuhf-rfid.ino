#include <ZUHF_CC1101.h> //Download original library here: http://electronoobs.com/eng_arduino_RFID_cc1101.php
#include <ZUHF_VARS.h>
#include <ZUHF_BUFFER.h>

#define DEBUG 0
#define MAXQUERY 88
#define QUERYSIZE 22 
#define MAXROUNDS 16
#define CW_SIZE 3
#define IDLE_SIZE 10
// additional global vars

byte paTable[8] = {0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00}; 
byte ack[] = {0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0};
byte powerValues[] = {0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0x80,0x81,0x82,0xCE,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0xCF,0x8B,0x8C,0x8D,0x8E,0x50,0x60,0x51,0x61,0x40,0x52,0x62,0x3F,0x3E,0x53,0x3D,0x63,0x3C,0x54,0x64,0x3B,0x55,0x2F,0x65,0x3A,0x2E,0x56,0x66,0x39,0x2D,0x57,0x67,0x8F,0x2C,0x38,0x68,0x2B,0x37,0x69,0x2A,0x6A,0x36,0x29,0x6B,0x28,0x35,0x27,0x26,0x34,0x25,0x6C,0x33,0x24,0x1F,0x1E,0x1D,0x1C,0x23,0x32,0x1B,0x1A,0x19,0x18,0x22,0xF,0xE,0x17,0xD,0xC,0x16,0x31,0xB,0xA,0x15,0x9,0x6D,0x8,0x14,0x21,0x7,0x13,0x6,0x5,0x12,0x4,0x3,0x2,0x11,0x1,0x10,0x20,0x30,0x0,0x6E,0x6F};
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
byte p = 0;


void setup()
{
    Serial.begin(9600);
    /* basic initialization of CC1101 */
    ZUHF_cc1101.Init(SS_TX);

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
    
  while (reader_state != R_WAIT)
  {
    switch(reader_state)
    {
      case R_START:
        //p = (p % sizeof(powerValues)) + 1 ;
        //paTable[1] = powerValues[p];
        //ZUHF_cc1101.SpiWriteBurstReg(CC1101_PATABLE, paTable, 8);
        ZUHF_cc1101.SpiStrobe(SS_TX, CC1101_SCAL);
        // turn on tx 
        ZUHF_cc1101.SpiStrobe(SS_TX, CC1101_STX);
        // send cw for tag activation
        ZUHF_cc1101.UpdateFifo(TAGSETTLE, sizeof(TAGSETTLE));
        
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
        ZUHF_cc1101.UpdateFifo(FIFOBUFFER, buffer_size);
        // 8) switch to next state 
        reader_state = R_CW;
        break;

      case R_CW:
        //ZUHF_cc1101.UpdateFifo(CW,CW_SIZE);
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
            reader_state = R_POWERDOWN;
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
        ZUHF_cc1101.UpdateFifo(FIFOBUFFER, buffer_size);
        irounds++;
        reader_state = R_CW;
        break;
        
      case R_ACK:
        // send a dummy ack
        buffer_size = generate_bytes(FIFOBUFFER, ack, sizeof(ack));
        ZUHF_cc1101.UpdateFifo(FIFOBUFFER, buffer_size);
        reader_state = R_CW;
        ACK_FLAG = 0;
        break;
        
      case R_POWERDOWN:
        //while(digitalRead(!GDO0_TX));
        //ZUHF_cc1101.UpdateFifo(FIFOBUFFER, buffer_size);
        //while(digitalRead(!GDO0_TX));
        //while(digitalRead(GDO0_TX));
        //ZUHF_cc1101.SpiStrobe(CC1101_SFTX);
        //delay(5);
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
