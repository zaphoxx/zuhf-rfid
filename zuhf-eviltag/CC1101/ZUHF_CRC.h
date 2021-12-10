#ifndef ZUHF_CRC_H
#define ZUHF_CRC_H


/* Function adapted from https://www.cgran.org/wiki/Gen2 */
/* crc check on received data */
bool check_crc16(byte *data, uint32_t num_bytes)
{
  uint16_t crc_16 = 0x0000, rcvd_crc = 0x0000;
  /* save crc from data into variable for later comparison */
  rcvd_crc = (data[num_bytes - 2] << 8) + data[num_bytes -1];
  
  crc_16 = 0xFFFF; 
  for (uint32_t i=0; i < num_bytes-2; i++)
  {
    crc_16^=data[i] << 8;
    for (uint32_t j=0;j<8;j++)
    {
      if (crc_16 & 0x8000)
      {
        crc_16 <<= 1;
        crc_16 ^= 0x1021;
      }
      else
        crc_16 <<= 1;
    }
  }
  crc_16 = ~crc_16;
  
  if(Serial){
    //Serial.print("[CRC16] CALC CRC16: ");Serial.println(crc_16,HEX);
    //Serial.print("[CRC16] RCVD CRC16: ");Serial.println(rcvd_crc,HEX);
  }
  if(rcvd_crc != crc_16)
    return false;
  else
    return true;
}

// ==========================================================================
// CRC Generation Unit - Linear Feedback Shift Register implementation
// (c) Kay Gorontzi, GHSi.de, distributed under the terms of LGPL
// ==========================================================================
uint16_t crc16B(byte *crc16bits, byte *bits, byte num_bits)
   {
   bool CRC[16];
   byte DoInvert;
   uint16_t crc16;
   for (int i=0; i<16; ++i)  CRC[i] = 1;                    // Init before calculation
   
   for (int i=0; i < num_bits; ++i)
   {
      DoInvert = (bits[i]==1) ^ CRC[15];     
      CRC[15] = CRC[14];
      CRC[14] = CRC[13];
      CRC[13] = CRC[12];
      CRC[12] = CRC[11] ^ DoInvert;
      CRC[11] = CRC[10];
      CRC[10] = CRC[9];
      CRC[9] = CRC[8];
      CRC[8] = CRC[7];
      CRC[7] = CRC[6];
      CRC[6] = CRC[5];
      CRC[5] = CRC[4] ^ DoInvert; 
      CRC[4] = CRC[3];
      CRC[3] = CRC[2];
      CRC[2] = CRC[1];
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
      }
   
   //memcpy(crc16bits, CRC, sizeof(CRC));
   uint16_t mask = 0x0001;
   crc16 = 0x0000;
   for (int i = 0; i < 16; i++)
   {
	   CRC[i] = CRC[i] ^ 1;
	   crc16bits[15-i] = CRC[i];
	   if (CRC[i])
	   {
		   crc16 = crc16 | mask;
	   }
	   mask = mask << 1;
   }
   return(crc16);
}

/* Function adapted from https://www.cgran.org/wiki/Gen2 */
/* check on bitarray after FM0 decoding */
/****************************************************************
*FUNCTION NAME: crc16
*FUNCTION     : calculates CRC16 from a sequence of bits
*INPUT        : bits: bit array of data over which crc16 should be calculated; num_bits: number of bits
*OUTPUT       : crc16 word
****************************************************************/
uint16_t crc16special(byte *bits, uint32_t num_bits)
{
  uint16_t crc_16, rcvd_crc;
  byte *data,*newbits;
  uint32_t num_bytes = num_bits / 8;
  uint32_t R = num_bits % 8;
  data = (uint8_t*) malloc(num_bytes);
  newbits = (uint8_t*) malloc(num_bits + (8 - R));
  uint8_t mask;
  num_bytes++;

  //memcpy(newbits, 0, 8-R);
  memcpy(newbits, bits, num_bits);
  memcpy(newbits + num_bits, 0, 8 - R);
  
  for(uint32_t i = 0 ; i < num_bytes; i++)
  {
	mask = 0x80;
	data[i] = 0;
	for(uint32_t j = 0; j < 8; j++)
	{
	  if (newbits[(i * 8) + j] == 1){
		data[i] = data[i] | mask;
	  }
	  mask = mask >> 1;
	}
  }

  rcvd_crc = (data[num_bytes - 2] << 8) + data[num_bytes -1];
  
  crc_16 = 0xFFFF; 
  for (uint32_t i=0; i < num_bytes; i++)
  {
    crc_16^=data[i] << 8;
    for (uint32_t j=0;j<8;j++)
    {
      if (crc_16 & 0x8000)
      {
        crc_16 <<= 1;
        crc_16 ^= 0x1021;
      }
      else
        crc_16 <<= 1;
    }
  }
  crc_16 = ~crc_16;
  free(data);
  free(newbits);
  return crc_16;
}


/* Function adapted from https://www.cgran.org/wiki/Gen2 */
/* check on bitarray after FM0 decoding */
/****************************************************************
*FUNCTION NAME: crc16
*FUNCTION     : calculates CRC16 from a sequence of bits
*INPUT        : bits: bit array of data over which crc16 should be calculated; num_bits: number of bits
*OUTPUT       : crc16 word
****************************************************************/
uint16_t crc16(byte *bits, uint32_t num_bits)
{
  uint16_t crc_16, rcvd_crc;
  byte *data;
  uint32_t num_bytes = num_bits / 8;
  
  data = (uint8_t*) malloc(num_bytes);
  
  uint8_t mask;
  
  
  for(uint32_t i = 0 ; i < num_bytes; i++)
  {
	mask = 0x80;
	data[i] = 0;
	for(uint32_t j = 0; j < 8; j++)
	{
	  if (bits[(i * 8) + j] == 1){
		data[i] = data[i] | mask;
	  }
	  mask = mask >> 1;
	}
  }

  rcvd_crc = (data[num_bytes - 2] << 8) + data[num_bytes -1];
  
  crc_16 = 0xFFFF; 
  for (uint32_t i=0; i < num_bytes; i++)
  {
    crc_16^=data[i] << 8;
    for (uint32_t j=0;j<8;j++)
    {
      if (crc_16 & 0x8000)
      {
        crc_16 <<= 1;
        crc_16 ^= 0x1021;
      }
      else
        crc_16 <<= 1;
    }
  }
  crc_16 = ~crc_16;
  free(data);
  return crc_16;
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

#endif