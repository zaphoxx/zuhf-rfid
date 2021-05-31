#ifndef UHF_RFID_H
#define UHF_RFID_H

#include <ZUHF_CRC.h>

#define TXBUFFERSIZE  512
#define BUFFERSIZE    512
#define QUERYSIZE     22 // query 22 bits
#define RN16_LEN      4 // length in bytes FM0 encoded (assuming a BLF 40kHz)
#define dT1  16
#define dT2  32

extern TAG_INFO tags_read[1000];
extern int tags_found;

struct EPC_DATA
{
  bool crc_ok       = false;
  byte epc[12]      = {0,0,0,0,0,0,0,0,0,0,0,0};
  byte stored_pc[2] = {0,0};
  byte crc16[2]     = {0,0};
};

/* ******************************************************************************** */
/* PROTOTYPES */
/* ******************************************************************************** */
/* UHF RFID COMMANDS */
/* QUERY */
void send_default_query(void);
void send_query_cmd(  byte datarate,      // (0,1)
                      byte m_index,       // (0,3)
                      byte trext,         // (0,1)
                      byte sel_index,     // (0,3)  
                      byte session_index, // (0,3)
                      byte target,        // t = (0,1)
                      byte q);            // q = (0,15)
void send_ack(byte *rn_bits);
void send_write(byte membank, uint32_t address, uint16_t data, byte *handle_bits, byte *rn16_bits);
void send_read(byte membank, uint32_t address, byte nwords, byte *handle_bits);
void send_req_rn(byte *rn_bits);
void send_lock(byte *lock_mask, byte *lock_action, byte *handle_bits);

/* FUNCTIONS FOR READING TAG RESPONSES */
bool read_epc(EPC_DATA *tag_epc);
bool read_RN16(byte *rn16);
bool read_data(byte *data, byte nwords);
bool search_write_ack();

/* FUNCTIONS FOR PROCESSING DATA */
int encode_data(byte *encoded, const byte *data, byte data_size);
int generate_bytes(byte *payload, byte *command, byte command_size);
void collect_epc_data(byte *data);
void decodeFM0(byte *bit_array, byte *data, byte data_size);
uint32_t calculate_evb_size(const uint32_t address);
uint32_t convert_to_evb(byte *evb, uint32_t address);
void transformCRC16ToBitarray(byte *bitarray, uint16_t data);
void data_to_bitarray(byte *bitarray, byte *data, byte numbytes);
/* ******************************************************************************** */

/* DEBUG FUNCTIONS */
void debug(String message);
void debug(byte *data, int data_size);
/* ******************************************************************************** */


/********************************************************************************************************************************
* FUNCTION NAME: send_query_cmd()
* FUNCTION     : sends out default query command
* INPUT        : none
* OUTPUT       : none
* https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf
* page 78, Table 6-32
* #     | CMD | DR | M | TREXT | SEL | SESSION | TARGET | Q | CRC5|
* bits  | 4   | 1  | 2 | 1     | 2   | 2       | 1      | 4 | 5   | 
* CMD = {1000};
* see also ZUHF_CC1101_VARS.h
**********************************************************************************************************************************/
void send_default_query(void)
{
  /* Send Basic Query */
  /*
   * datarate       = 0
   * m_index        = 0
   * trext          = 1 ; turn on pilot tone
   * sel_index      = 0
   * session_index  = 0
   * target         = 0
   * q              = 0
   */
   send_query_cmd(0, 0, 1, 0, 0, 0, 0);
}


/********************************************************************************************************************************
* FUNCTION NAME: send_query_cmd()
* FUNCTION     : sends out default query command
* INPUT        : see table below
* OUTPUT       : none
* https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf
* page 78, Table 6-32
* #     | CMD | DR | M | TREXT | SEL | SESSION | TARGET | Q | CRC5|
* bits  | 4   | 1  | 2 | 1     | 2   | 2       | 1      | 4 | 5   | 
* CMD = {1000};
* see also ZUHF_CC1101_VARS.h
**********************************************************************************************************************************/
void send_query_cmd(  byte datarate,      // (0,1)
                      byte m_index,       // (0,3)
                      byte trext,         // (0,1)
                      byte sel_index,     // (0,3)  
                      byte session_index, // (0,3)
                      byte target,        // t = (0,1)
                      byte q)             // q = (0,15)
{
  //Serial.println("[SEND QUERY]");
  byte buffer[BUFFERSIZE];
  byte query[QUERYSIZE];
  byte encoded_query[BUFFERSIZE];
  byte buffersize = 0;
  byte txbuffer[TXBUFFERSIZE];
  int nBytesToSend = 0;
  byte pos = 0;
  
  memcpy(buffer, PREAMBLE, sizeof(PREAMBLE));
  buffersize += sizeof(PREAMBLE);
  
  // QUERY = QUERY_CODE + DR + M + TREXT + SEL_ALL + SESSION + TARGET + Q + CRC5
  memcpy(query, QUERY_CODE, sizeof(QUERY_CODE));
  pos += sizeof(QUERY_CODE);
  memcpy(query+pos, &datarate, sizeof(datarate));
  pos += sizeof(datarate);
  memcpy(query+pos, M[m_index], sizeof(M[m_index]));
  pos += sizeof(M[m_index]);
  memcpy(query+pos, &trext, sizeof(trext));
  pos += sizeof(trext);
  memcpy(query+pos, SEL_SL[sel_index], sizeof(SEL_SL[sel_index]));
  pos += sizeof(SEL_SL[sel_index]);
  memcpy(query+pos, SESSION[session_index], sizeof(SESSION[session_index]));
  pos += sizeof(SESSION[session_index]);
  memcpy(query+pos, &target, sizeof(target));
  pos += sizeof(target);
  memcpy(query+pos, Q_VALUE[q], sizeof(Q_VALUE[q]));
  pos += sizeof(Q_VALUE[q]);
  pos += 5; 
  crc5_append(query, QUERYSIZE);
  byte encoded_size = encode_data(encoded_query, query, QUERYSIZE);
  memcpy(buffer + buffersize, encoded_query, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer,nBytesToSend);
}


/********************************************************************************************************************************
* FUNCTION NAME: send_ack(bool is_open)
* FUNCTION     : sends ack command to tag
* INPUT        : is_open: this refers to the current tag state, if the tag is in open/secured state then send HANDLE_Bits, if ack is send after a query cmd then
                 ack needs to reply with the RN16_Bits provided;
* OUTPUT       : none
**********************************************************************************************************************************/
void send_ack(byte *rn_bits)
{
  byte buffer[512];
  byte encoded_cmd[512];
  byte ack_bits[128];
  byte txbuffer[64];
  int buffersize = 0;
  int nBytesToSend = 0;
  int offset = 0;
  memcpy(buffer, FRAMESYNC, sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  memcpy(ack_bits, ACK, sizeof(ACK));
  offset += sizeof(ACK);
  //Serial.println();
  /* if not in open state then send RN16_Bits */
  /* else send the HANDLE_Bits aquired from the req_rn command */
  /* this differentiation is important, so make sure you provided
   *  the correct input parameter; However typically you will want
   * to send the RN16_Bits 
   */
  memcpy(ack_bits+offset, rn_bits, 16);
  offset += 16;
  byte encoded_size = encode_data(encoded_cmd, ack_bits, offset);
  memcpy(buffer + buffersize, encoded_cmd, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer, nBytesToSend);
  Serial.println("[SEND ACK]");
}


/********************************************************************************************************************************
* FUNCTION NAME: send_write()
* FUNCTION     : send write command to tag
* INPUT        : membank: memorybank to write to (0-3)
                 address: address to write to (e.g. 0 - first word of memorybank, 1 - second word of memorybank)
                 data: word data to write to memorybank (e.g. 0xCAFE)
* OUTPUT       : none
**********************************************************************************************************************************/
void send_write(byte membank, uint32_t address, uint16_t data, byte *handle_bits, byte *rn16_bits)
{ 
  //Serial.println("[SEND WRITE]");
  uint32_t tmp = address;
  uint32_t evb_size = 0;
  
  /* convert address to ebv format */
  byte evb[evb_size];
  evb_size = convert_to_evb(evb, address);
  
  byte evb_bits[evb_size * 8];
  data_to_bitarray(evb_bits, evb, evb_size);

  /* ******************************* */
  /* ***** XOR data and rn16  ***** */
  /* ******************************* */
  byte rn16[2];
  byte xored[2];
  generate_bytes(rn16, rn16_bits, 16);
  xored[1] = rn16[1]^(byte)(data >> 8);
  xored[0] = rn16[0]^(byte)(data & 0x00ff);
  byte data_bits[16];
  data_to_bitarray(data_bits,xored,sizeof(xored));
  /* ******************************* */
   
  byte write_bits[512];
  byte encoded_cmd[512];
  int nBytesToSend = 0;
  byte buffer[512];
  byte txbuffer[512];
  int buffersize = 0;
  memcpy(buffer, FRAMESYNC, sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  
  byte offset = 0;
  memcpy(write_bits, WRITE_CMD, sizeof(WRITE_CMD));
  offset += sizeof(WRITE_CMD);
  memcpy(write_bits + offset, MEM_BANK[membank], sizeof(MEM_BANK[membank]));
  offset += sizeof(MEM_BANK[membank]);
  memcpy(write_bits + offset, evb_bits, sizeof(evb_bits));
  offset += sizeof(evb_bits);
  memcpy(write_bits + offset, data_bits, sizeof(data_bits));
  offset += sizeof(data_bits);
  memcpy(write_bits + offset, handle_bits, 16);
  offset += 16;
  
  byte crc16_bits[16];
  crc16B(crc16_bits, write_bits, offset);
 
  memcpy(write_bits + offset, crc16_bits, sizeof(crc16_bits));
  offset += sizeof(crc16_bits);
  
  byte encoded_size = encode_data(encoded_cmd, write_bits, offset);

  memcpy(buffer + buffersize, encoded_cmd, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer,nBytesToSend);
}


/********************************************************************************************************************************
* FUNCTION NAME: send_blockwrite()
* FUNCTION     : send write command to tag
* INPUT        : membank: memorybank to write to (0-3)
                 address: address to write to (e.g. 0 - first word of memorybank, 1 - second word of memorybank)
                 data: data to write to memorybank as bytearray(e.g. 0xCAFE)
                 n_bytes: number of data bytes
* OUTPUT       : none
**********************************************************************************************************************************/
void send_blockwrite(byte membank, uint32_t address, byte n_bytes, uint8_t *data,  byte *handle_bits, byte *rn16_bits)
{ 
  //Serial.println("[SEND WRITE]");
  uint32_t tmp = address;
  uint32_t evb_size = 0;
  
  /* convert address to ebv format */
  byte evb[evb_size];
  evb_size = convert_to_evb(evb, address);
  
  byte evb_bits[evb_size * 8];
  data_to_bitarray(evb_bits, evb, evb_size);

  /* ******************************* */
  /* ***** XOR data and rn16  ***** */
  /* ******************************* */
  byte rn16[2];
  generate_bytes(rn16, rn16_bits, 16);
  /* ******************************* */
   
  byte write_bits[512];
  byte data_bits[512];
  byte encoded_cmd[512];
  byte size_bits[8];
  int nBytesToSend = 0;
  byte buffer[512];
  byte txbuffer[512];
  int buffersize = 0;
  byte n_words = n_bytes / 2;
  data_to_bitarray(data_bits, data, n_bytes); // bit array size 8*n_bytes
  data_to_bitarray(size_bits, &n_words, 1);
  memcpy(buffer, FRAMESYNC, sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  
  byte offset = 0;
  memcpy(write_bits, BLOCKWRITE_CMD, sizeof(BLOCKWRITE_CMD));
  offset += sizeof(WRITE_CMD);
  memcpy(write_bits + offset, MEM_BANK[membank], sizeof(MEM_BANK[membank]));
  offset += sizeof(MEM_BANK[membank]);
  memcpy(write_bits + offset, evb_bits, sizeof(evb_bits));
  offset += sizeof(evb_bits);
  memcpy(write_bits + offset, size_bits, sizeof(size_bits)); 
  offset += sizeof(size_bits);
  memcpy(write_bits + offset, data_bits, sizeof(data_bits));
  offset += sizeof(data_bits);
  memcpy(write_bits + offset, handle_bits, 16);
  offset += 16;
  byte crc16_bits[16];
  crc16B(crc16_bits, write_bits, offset);
  memcpy(write_bits + offset, crc16_bits, sizeof(crc16_bits));
  offset += sizeof(crc16_bits);
  
  byte encoded_size = encode_data(encoded_cmd, write_bits, offset);

  memcpy(buffer + buffersize, encoded_cmd, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer,nBytesToSend);
}


/********************************************************************************************************************************
* FUNCTION NAME: send_read()
* FUNCTION     : send read command to tag
* INPUT        : membank: memorybank to read from (0-3)
                 address: address to read from (e.g. 0 - first word of memorybank, 1 - second word of memorybank)
                 nwords: number of words to read
* OUTPUT       : none
**********************************************************************************************************************************/
void send_read(byte membank, uint32_t address, byte nwords, byte *handle_bits)
{
  //Serial.println("[SEND READ]");
  byte wordcount[8];
  uint32_t evb_size = 0;
  
  byte evb[evb_size];
  evb_size = convert_to_evb(evb, address);
  byte evb_bits[evb_size * 8];
  data_to_bitarray(evb_bits, evb, evb_size);
  byte READ_BITS[512];

  //MEM_BANK Selection:
  //0 --> {0,0}, // Reserved
  //1 --> {0,1}, // EPC
  //2 --> {1,0}, // TID
  //3 --> {1,1} // USER
  
  data_to_bitarray(wordcount, &nwords, 1);
  int nBytesToSend = 0;
  byte buffer[512];
  byte txbuffer[512];
  byte encoded_query[512];
  int buffersize = 0;
  memcpy(buffer, FRAMESYNC, sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  
  byte offset = 0;
  memcpy(READ_BITS, READ_CMD, sizeof(READ_CMD));
  offset += sizeof(READ_CMD);
  memcpy(READ_BITS + offset, MEM_BANK[membank], sizeof(MEM_BANK[membank]));
  offset += 2;
  memcpy(READ_BITS + offset, evb_bits, sizeof(evb_bits));
  offset += sizeof(evb_bits);
  memcpy(READ_BITS + offset, wordcount, sizeof(wordcount));
  offset += sizeof(wordcount);
  memcpy(READ_BITS + offset, handle_bits, 16);
  offset += 16;
  byte CRC16_BITS[16];
  crc16B(CRC16_BITS, READ_BITS, offset);
  memcpy(READ_BITS + offset, CRC16_BITS, sizeof(CRC16_BITS));
  offset += sizeof(CRC16_BITS);
  
  byte encoded_size = encode_data(encoded_query, READ_BITS, offset);

  memcpy(buffer + buffersize, encoded_query, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer,nBytesToSend);
}


/********************************************************************************************************************************
* FUNCTION NAME: send_req_rn()
* FUNCTION     : send req_rn command to tag (request handle/rn16)
* INPUT        : rn_bits: current rn16 / handle
* OUTPUT       : none
**********************************************************************************************************************************/
void send_req_rn(byte *rn_bits) // handle size is 16 bits
{
  //Serial.println("[SEND REQ_RN]");
  byte req_rn_bits[40];
  
  memcpy(req_rn_bits, REQ_RN, sizeof(REQ_RN));
  memcpy(req_rn_bits+sizeof(REQ_RN), rn_bits, 16);
  uint16_t CRC16 = crc16(req_rn_bits, 24);
  byte CRC16_BITS[16];
  CRC16 = crc16B(CRC16_BITS,req_rn_bits,24);
  memcpy(req_rn_bits+sizeof(REQ_RN)+16, CRC16_BITS, sizeof(CRC16_BITS));

  
  byte txbuffer[64];
  byte buffer[512];
  int buffersize = 0;
  int offset = 0;
  int nBytesToSend = 0;
  byte encoded_cmd[512];
  
  memcpy(buffer, FRAMESYNC,sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  
  byte encoded_size = encode_data(encoded_cmd, req_rn_bits, 40);
  memcpy(buffer + buffersize, encoded_cmd, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer, nBytesToSend);
}

/********************************************************************************************************************************
* FUNCTION NAME: send_lock()
* FUNCTION     : send lock cmd to tag 
* INPUT        : lock_mask and lock_action as bitarrays (for more details please refer to pg.89 in 
*                https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf)
* OUTPUT       : none
**********************************************************************************************************************************/
void send_lock(byte *lock_mask, byte *lock_action, byte *handle_bits)
{ 
  //Serial.println("[SEND LOCK]");
  #define LOCK_MASK_SIZE 10
  byte lock_bits[512];
  byte encoded_query[512];
  int nBytesToSend = 0;
  byte buffer[512];
  byte txbuffer[512];
  int buffersize = 0;
  
  // byte lock_mask[] = {0,0,0,0,0,0,1,1,0,0};
  // byte lock_action[] = {0,0,0,0,0,0,0,0,0,0};
  
  memcpy(buffer, FRAMESYNC, sizeof(FRAMESYNC));
  buffersize += sizeof(FRAMESYNC);
  
  byte offset = 0;
  memcpy(lock_bits, LOCK_CMD, sizeof(LOCK_CMD));
  offset += sizeof(LOCK_CMD);
  memcpy(lock_bits + offset, lock_mask, LOCK_MASK_SIZE);
  offset += LOCK_MASK_SIZE;
  memcpy(lock_bits + offset, lock_action, LOCK_MASK_SIZE);
  offset += LOCK_MASK_SIZE;
  memcpy(lock_bits + offset, handle_bits, 16);
  offset += 16;
  
  byte CRC16_BITS[16];
  crc16B(CRC16_BITS, lock_bits, offset);
 
  memcpy(lock_bits + offset, CRC16_BITS, sizeof(CRC16_BITS));
  offset += sizeof(CRC16_BITS);
  
  byte encoded_size = encode_data(encoded_query, lock_bits, offset);
  memcpy(buffer + buffersize, encoded_query, encoded_size);
  buffersize += encoded_size;
  nBytesToSend = generate_bytes(txbuffer, buffer, buffersize);
  TX_UNIT.UpdateFifo(txbuffer,nBytesToSend);
}

/********************************************************************************************************************************
* FUNCTION NAME: generate_bytes()
* FUNCTION     : generates byte array from a bit array / if R = nbits % 8 > 0 the function will fillup the
*                remaining bits with ones
* INPUT        : payload: output byte array (user needs to make sure the provided array is large enough); 
*                command: input bit array; 
*                command_size: number of bits to transform
* OUTPUT       : returns the number of bytes generated
**********************************************************************************************************************************/
int generate_bytes(byte *byte_array, byte *bit_array, byte bit_array_size)
{
  int R = bit_array_size % 8;
  int N = bit_array_size / 8;
  byte block = 0;
  byte MASK;
  for (int i = 0; i < N; i++)
  {
    MASK = 0x80;
    byte_array[i] = 0;
    for (int j = 0; j < 8; j++){
      if (bit_array[(i*8)+j] == 1){
        byte_array[i] = byte_array[i] | MASK;
      }
      MASK = MASK >> 1;
    }
  }
  if (R > 0){
    MASK = 0x80;
    byte_array[N] = 0xFF;
    for (int j = 0; j < R; j++){
      if (bit_array[(N*8)+j] == 0){
        byte_array[N] = byte_array[N] & (~MASK);
      }
      MASK = MASK >> 1;
    }
  }
  if (R > 0){
    return N+1;
  }else{
    return N;
  }
}



/********************************************************************************************************************************
* FUNCTION NAME: encode_data()
* FUNCTION     : PIE encoding of a provided bit array
* INPUT        : encoded:   output bit array (user needs to make sure the provided array is large enough); 
*                data:      input bit array; 
*                data_size: number of bits to encode
* OUTPUT       : returns the number of bits representing the encoded data
**********************************************************************************************************************************/
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

/********************************************************************************************************************************
* FUNCTION NAME: decodeFM0()
* FUNCTION     : decodes and FM0 encoded bit array - assuming the tag responses BLF is 40kHz
* INPUT        : bit_array: array to put result into
*              : data: byte array with FM0 encoded data (as received from the RX Module)
 *             : data_size: number of bytes to decode
* OUTPUT       : none
**********************************************************************************************************************************/
// decode FM0 data
// input:  data array with FM0 encoded data, (this is not a general decoder as it assumes a certain structure based on configured datarate)
//         It also DOES !!NOT!! check if State is properly changed for each data unit.
// output: bits as a bitarray, the user needs to make sure the bit array size can hold the necessary data.
// bitarray size should be 4 * sizeof(data) (assuming 1 FM0 symbol is encoded by two bits. 
void decodeFM0(byte *bit_array, byte *data, byte data_size)
{
  byte bit_index = 0;
  byte mask = 0xC0; // 1100 0000
  byte FM0 = 0;
  byte prev = 0;
  byte curr = 0;
  for (byte i = 0; i < data_size; i++)
  {
    for (byte shift = 0; shift < 4; shift++)
    {
      FM0 = (data[i] & (mask >> (shift*2))) >> (6-(shift*2));
      switch(FM0){
        case 3:
          bit_array[(i*4)+shift] = 1;
          break;
        case 0:
          bit_array[(i*4)+shift] = 1;
          break;
        case 2:
          bit_array[(i*4)+shift] = 0;
          break;
        case 1:
          bit_array[(i*4)+shift] = 0;
          break;
        default:
          Serial.println("[ERROR] Error decoding FM0 data!");
          break;
      }
    }
  }
 }


/********************************************************************************************************************************
* FUNCTION NAME: calculate_evb_size()
* FUNCTION     : calculates number of evb blocks necessary to represent an int
* INPUT        : address: uint32_t number to convert to evb and size needs to be calculated from
* OUTPUT       : number of evb blocks
**********************************************************************************************************************************/
uint32_t calculate_evb_size(const uint32_t address)
{
  uint32_t evb_size = 0;
  uint32_t tmp = address;
  if (tmp == 0) evb_size = 1;
  while (tmp > 0) {
    tmp = tmp >> 7;
    evb_size++;
  }
  return evb_size;
}
/********************************************************************************************************************************
* FUNCTION NAME: convert_to_evb()
* FUNCTION     : Converts an int (uint32_t) to EVB Nomenclature
* INPUT        : evb: byte array to write EVB to
                 evb_size: number of bytes EVB is build off; this number needs to be precalculated;
* OUTPUT       : returns size of evb (uint32_t)
**********************************************************************************************************************************/
uint32_t convert_to_evb(byte *evb, uint32_t address)
{
  uint32_t tmp = address;
  uint32_t mask = 0x7F;
  uint32_t evb_size = calculate_evb_size(address);
  
  for (int i = 0; i < evb_size; i++)
  {
    evb[(evb_size-1) - i] = 0;
    evb[(evb_size-1) - i] = (byte) (address & mask);
    if ( i < (evb_size-1) ) {
      evb[(evb_size-1) - i] = evb[(evb_size-1) - i] | 0x80;
    }
    address = address >> 7;
  }
  return evb_size;
}


/********************************************************************************************************************************
* FUNCTION NAME: data_to_bitarray()
* FUNCTION     : Converts byte array to a bit array
* INPUT        : bitarray: array to write data to
                 data: array which is supposed to be transformed
                 numbytes: number of bytes to transform
* OUTPUT       : none
**********************************************************************************************************************************/
void data_to_bitarray(byte *bitarray, byte *data, byte numbytes)
{
  uint32_t numbits = numbytes * 8;
  byte MASK;
  
  for (int i = 0; i < numbytes; i++)
  {
    MASK = 0x80;
    for (int j = 0; j < 8; j++){
      bitarray[i*8+j] = (MASK & data[i]) >> (7 - j);
      MASK = MASK >> 1;
    } 
  }
}


/********************************************************************************************************************************
* FUNCTION NAME: transformCRC16ToBitarray()
* FUNCTION     : Converts uint16_t data into a bit array
* INPUT        : bitarray: array to write data to
                 data: word to transform
               * typically used to transform HANDLE or RN16 into a bitarray *
* OUTPUT       : none
**********************************************************************************************************************************/
void transformCRC16ToBitarray(byte *bitarray, uint16_t data){
  uint16_t MASK = 0x8000;
  for (int i = 0; i < 16; i++){      
      bitarray[i] = (MASK & data) >> (15 - i);
      MASK = MASK >> 1;
  }
}


/********************************************************************************************************************************
* FUNCTION NAME: read_RN16()
* FUNCTION     : searches and reads RN16 data from response - should be used after query cmd only (see also read_handle)
* INPUT        : byte array to be filled with RN16 data
               : (t1: time durations in 100us units; good values are t1 = 16)
* OUTPUT       : returns true if crc16 check is ok; if not it will still fill the
*                epc_data structure but the crc16 flag will set to false;
**********************************************************************************************************************************/
bool read_RN16(byte *rn16)
{
  const byte t1 = dT1; // adjust if necessary
  bool found = false;
  byte rxbuffer[512];
  //Serial.println("[READ RN16] *");
  RX_UNIT.SpiWriteReg(CC1101_PKTLEN, RN16_LEN);
  RX_UNIT.SpiStrobe(CC1101_SRX);
  TX_UNIT.SendCW(16);
  while(TX_GDO0_STATE){
    if(RX_GDO0_STATE){
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, rxbuffer, RN16_LEN);
      decodeFM0(rn16, rxbuffer, RN16_LEN);
      found = true;
      break;
    }
  }
  if (found)
  {
    Serial.println("[READ RN16] OK");
  }
  return found;
}


/********************************************************************************************************************************
* FUNCTION NAME: read_RN16()
* FUNCTION     : searches and reads RN16 data from response - should be used after query cmd only (see also read_handle)
* INPUT        : byte array to be filled with RN16 data
               : (t1: time durations in 100us units; good values are t1 = 16)
* OUTPUT       : returns true if crc16 check is ok; if not it will still fill the
*                epc_data structure but the crc16 flag will set to false;
**********************************************************************************************************************************/
/* This reads either the Handle or an RN16 from an REQ_RN response ; main differenct to read_rn16 function is that in this case a
 *  crc16 check takes place.
 */
bool read_Handle(byte *rn_bits){
  //Serial.println("[READ RN16/HANDLE] *");
  bool found = false;
  bool crc16_ok = false;
  byte rxbuffer[64];
  RX_UNIT.SpiWriteReg(CC1101_PKTLEN, 8);
  RX_UNIT.SpiStrobe(CC1101_SRX);
  TX_UNIT.SendCW(16);
  while(TX_GDO0_STATE){
    if(RX_GDO0_STATE){
      TX_UNIT.SendCW(10);
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, rxbuffer, 8);
      found = true;
      break;
    }
  }
  if (found)
  {
    Serial.println("[READ RN16/HANDLE] FOUND");
    byte HANDLE_DATA_Bits[32];
    decodeFM0(HANDLE_DATA_Bits, rxbuffer, 8);
    byte HANDLE_DATA[4];
    generate_bytes(HANDLE_DATA, HANDLE_DATA_Bits, sizeof(HANDLE_DATA_Bits));
    if(check_crc16(HANDLE_DATA, sizeof(HANDLE_DATA)))
    {
      memcpy(rn_bits, HANDLE_DATA_Bits, 16);
      Serial.println("[READ RN16/HANDLE] OK");
      crc16_ok = true;
    }else{
      crc16_ok = false;
    }
  }
  return crc16_ok;
}


/********************************************************************************************************************************
* FUNCTION NAME: read_epc()
* FUNCTION     : searches and reads epc data from response
* INPUT        : epc_data structure to be filled in
* OUTPUT       : returns true if crc16 check is ok; if not it will still fill the
*                epc_data structure but the crc16 flag will set to false;
**********************************************************************************************************************************/
bool read_epc(EPC_DATA *tag_epc){
  //Serial.println("[READ EPC] *");
  #define PLEN 32
  bool found = false;
  bool crc_ok = false;
  byte epc_data[PLEN];
  byte epc_bits[4*PLEN];
  byte epc_bytes[PLEN/2];
  
  RX_UNIT.SpiWriteReg(CC1101_PKTLEN,PLEN);
  RX_UNIT.SpiStrobe(CC1101_SRX);

  /* If the Reader has difficulties finding the epc or performing other actions after reading the epc
   *  then you might try to tune the value for TX_UNIT.SendCW(xx) a bit. Try to make it as low as
   *  possible. Values > 14 are typically to long and will violate the max repsonse time reader -> tag.
   *  If the value is to low then the reader might not have enough time to capture the radiosignal.
   */
  TX_UNIT.SendCW(14); // critical value, change only if really necessary and you know what you are doing!
  while(TX_GDO0_STATE)
  {
    if (RX_GDO0_STATE)
    {
      TX_UNIT.SendCW(32); // similar to above but not as sensitive as above.
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, epc_data, PLEN);
      found = true;
      break;
    }
  }
  if (found)
  {
    //
    decodeFM0(epc_bits, epc_data, PLEN);
    generate_bytes(epc_bytes,epc_bits,(4 * PLEN));
    
    /* If CRC16 Check is OK print received Tag data and collect data */
    if (check_crc16(epc_bytes,PLEN/2))
    {
      //LED_RED_ON;
      memcpy(tag_epc->stored_pc, epc_bytes, 2);
      /* At this point we assume that the EPC is 96 bits long */
      /* This might need adjustment in the future */
      /* The actual epc size can also be found in the stored_pc checking
       * the first 5bits see section 6.3.2.1.2.2 in 
       * https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf */
      memcpy(tag_epc->epc, epc_bytes + 2, 12);
      memcpy(tag_epc->crc16, epc_bytes + 14, 2); 
      crc_ok = true;
      /* ---------------------------------------------------------*/
      /* TODO: temporarily in - needs to be removed in the future */
      //collect_epc_data(epc_bytes);
      /* ---------------------------------------------------------*/
      Serial.println("[READ EPC] OK");
    }else{
      //Serial.println("[READ EPC] ERROR - CRC ERROR");
    }
  }else{
    //Serial.println("[READ EPC] ERROR - EPC NOT FOUND");
  }
  tag_epc->crc_ok = crc_ok;
  return crc_ok;  
}



/********************************************************************************************************************************
* FUNCTION NAME: read_data()
* FUNCTION     : reads data response from a previous read cmd
* INPUT        : nwords: number of words (words in the sense 2 bytes of data) to read
* OUTPUT       : returns true if crc16 check is ok;
* !!! Dont try to read more then 27 words at once !!! RX FIFO BUFFER is 64 BYTES !!!
**********************************************************************************************************************************/
bool read_data(byte *data, byte nwords)
{
  if (nwords > 27)
  {
    Serial.println("[READ DATA] ERROR! Number of chosen words is too large! Please choose a value < 28 words!");
    return false;
  }
  byte rxbuffer[512];
  byte data_bits[512];
  byte data_bytes[512];
  bool crc_ok = false;
  bool packet_received = false;
  int bytes_received = 0;
  int packetlength = ((nwords * 2) + 5) * 2; 
  
  RX_UNIT.SpiWriteReg(CC1101_PKTLEN, packetlength);
  RX_UNIT.SpiStrobe(CC1101_SRX);

  TX_UNIT.SendCW(14);
  while(TX_GDO0_STATE){
    if(RX_GDO0_STATE){
      TX_UNIT.SendCW(packetlength * 2);
      bytes_received = 0;
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, rxbuffer, packetlength);
      packet_received = true;
      break;
    }
  }
  if (packet_received)
  {
    decodeFM0(data_bits, rxbuffer, packetlength);
    int data_size = generate_bytes(data_bytes, data_bits + 1, nwords * 16);
    memcpy(data, data_bytes, data_size);
    //debug(data, data_size);
  }else{
    //Serial.print("[READ DATA] ERROR");
  }
  return packet_received;
}
/********************************************************************************************************************************
* FUNCTION NAME: search_write_ack()
* FUNCTION     : searches for a response after write_cmd
* INPUT        : none
* OUTPUT       : none
**********************************************************************************************************************************/
bool search_write_ack()
{
  /*
   * The Tag successfully executes the command: After executing the command the Tag
   * shall backscatter the reply shown in Table 6-13 and Figure 6-16, comprising a header (a 0-
   * bit), the Tag’s handle, and a CRC-16 calculated over the 0-bit and handle. The reply shall
   * meet the T5 limits in Table 6-16. If the Interrogator observes this reply within T5(max) then
   * the command completed successfully. 
   */
  bool found = false;
  bool crc_ok = false;
  byte rxbuffer[10]; // ( 16 RN16 + 16 CRC + 1 ) * 2 Bits
  byte databuffer[128];
  //Serial.println("[SEARCH WRITE ACK] *");
  RX_UNIT.SpiWriteReg(CC1101_PKTLEN, 10);
  RX_UNIT.SpiStrobe(CC1101_SRX);
  TX_UNIT.SendCW(64);
  while(TX_GDO0_STATE)
  {
    if(RX_GDO0_STATE)
    {
      TX_UNIT.SendCW(64);
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, rxbuffer, 10);
      found = true;
      break;
    }
  }
  if (found)
  {
    //Serial.println("[WRITE RESPONSE] *");
    decodeFM0(databuffer, rxbuffer, 10);
  }
  
  if (databuffer[0] == 0)
  {
    uint8_t rcrc16[2];
    generate_bytes(rcrc16, databuffer+17,16);
    byte crc16_bits[16];
    uint16_t ccrc16 = crc16B(crc16_bits, databuffer, 17); // rn16 + header bit;
    if (rcrc16[0] == (byte)(ccrc16 >> 8) and rcrc16[1] == (byte)(ccrc16 & 0x00ff))
    {
      crc_ok = true;
      Serial.println("WRITE#OK#");
    }
  }else{
    //Serial.println("[WRITE RESPONSE] ERROR RESPONSE!");
  }
  return crc_ok;
}


/* TODO SECTION */

/* TODO: Outdated Function But Still Necessary Until Fully Updated */
void collect_epc_data(byte *data)
{
  /* collect data */
  tags_read[tags_found].StoredPC = data[0] << 8 + data[1];
  memcpy(tags_read[tags_found].EPC_Data, data+2, 12);
  memcpy(tags_read[tags_found].CRC16, data+14, 2);
  tags_read[tags_found].CRC_OK = true;
  tags_found++;
}
#endif


/* debug(msg)/debug(data) - simple functions to send debug messages via serial communication */
void debug(String message)
{
  // Send Identifier '#DEBUG'
  Serial.println("#DEBUG");
  Serial.print(message);
  Serial.println("#");
}

void debug(byte *data, int data_size)
{
  Serial.println("#DEBUG");
  for (int i = 0; i < data_size; i++){
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println("#");
}
