
#define RF_STREAM
#define AVR
//#define SAMX

#include "CC1101/CC1101.cpp"

#define FIFO_BUFFER 340// 192
#define BUFFER 16
#define PSIZE 16

//byte CWA[FIFO_BUFFER];
//byte CWB[FIFO_BUFFER];

const int buffersize=340;
const double PW = 3.125; // PW in µs as per CC1101 RX DATARATE

byte AA[PSIZE]={1,0,1,0,1,0,1,1,0,1,1,0,1,0,0,1};
//byte *RX_BUFFER01 = new byte[buffersize];
byte RX_BUFFER[FIFO_BUFFER];
byte RX_BUFFER01[FIFO_BUFFER];
byte RX_BUFFER02[FIFO_BUFFER];
byte RX_BUFFER03[FIFO_BUFFER];
byte DATA[16];
byte CHUNK[BUFFER];
int nChunks=0;



void setup() {
  delay(100);
  Serial.begin(115200);
  
  //for (int i = 0; i < FIFO_BUFFER; i++) RX_BUFFER[i] = 0x00;
  RFMOD.Init();
  Serial.println("EVIL-TAG");
// asynchronous mode
  RFMOD.SpiWriteReg(CC1101_PKTCTRL0,0x12); // synchronous mode / infinite packet length
  RFMOD.SpiWriteReg(CC1101_PKTCTRL1,0x00); // -
  RFMOD.SpiWriteReg(CC1101_IOCFG0,0x0C); // synchronous mode GDO0 TX input / GDO0 RX output
  RFMOD.SpiWriteReg(CC1101_IOCFG2,0x0B); // Serial Clock on GDO2 (output) configuration
  RFMOD.SpiWriteReg(CC1101_MCSM1,0x0b); // 0x0e
  RFMOD.SpiWriteReg(CC1101_MCSM0,0x04);
  RFMOD.SpiWriteReg(CC1101_PKTLEN,0xFF);
  RFMOD.SpiWriteReg(CC1101_SYNC0,0xaa);
  RFMOD.SpiWriteReg(CC1101_SYNC1,0x55);

  /* ******** 870MHZ ******** */
  /*
  RFMOD.SpiWriteReg(CC1101_FREQ2,0x21); // 870MHz - somehow test transmitter frequnenxy is shifted by 2MHz
  RFMOD.SpiWriteReg(CC1101_FREQ1,0x76); //
  RFMOD.SpiWriteReg(CC1101_FREQ0,0x27); //
  /* *********************** */

  /* ******** 868MHZ ******** */
  RFMOD.SpiWriteReg(CC1101_FREQ2,0x21); // 868MHz - somehow test transmitter frequnenxy is shifted by 2MHz
  RFMOD.SpiWriteReg(CC1101_FREQ1,0x62); //
  RFMOD.SpiWriteReg(CC1101_FREQ0,0x76); //
  /* *********************** */
  
  RFMOD.SpiWriteReg(CC1101_FSCTRL1,0x0F);
  RFMOD.SpiWriteReg(CC1101_FSCTRL0,0x00);
  RFMOD.SpiWriteReg(CC1101_CHANNR, 0x00);
 
  RFMOD.SpiWriteReg(CC1101_MDMCFG4,0x0d); //0x0d);  //0x4C -> 160kBaud, BW 403 kHz / 0x0d -> 320kBaud, BW 815kHz
  RFMOD.SpiWriteReg(CC1101_MDMCFG3,0x93); //0x93
  RFMOD.SpiWriteReg(CC1101_MDMCFG2,0x30);
  RFMOD.SpiWriteReg(CC1101_MDMCFG1,0x00);
  RFMOD.SpiWriteReg(CC1101_MDMCFG0,0x93);
  
  RFMOD.SpiWriteReg(CC1101_AGCCTRL2,  0x07);  
  RFMOD.SpiWriteReg(CC1101_AGCCTRL1,  0x00);
  RFMOD.SpiWriteReg(CC1101_AGCCTRL0,  0x90);

  RFMOD.SpiWriteReg(CC1101_BSCFG,0x03);
  
  RFMOD.SpiWriteReg(CC1101_FREND1,0xB6);
  //RFMOD.SpiWriteReg(CC1101_FREND0,0x12);
  RFMOD.SpiWriteReg(CC1101_TEST2,0x88);
  RFMOD.SpiWriteReg(CC1101_TEST1,0x31);
  RFMOD.SpiWriteReg(CC1101_FIFOTHR,0x07);
  delay(100);
 
  
  //pinMode(GDO0_PIN, OUTPUT);
  pinMode(GDO0_PIN, INPUT);
  RFMOD.SpiStrobe(CC1101_SRX);
  //GDO0_LOW;
}

void loop() {
  /*
  // #####################################
  pinMode(GDO0_PIN, OUTPUT);
  //Serial.println("##2");
  RFMOD.SpiStrobe(CC1101_STX);  
  while ((RFMOD.SpiReadStatus(CC1101_MARCSTATE) & 0x1F) != M_STX);
  //Serial.println((RFMOD.SpiReadStatus(CC1101_MARCSTATE) & 0x1F), HEX);
  //delayNbit(9);
  for (int i = 0; i < 8; i++)
  {
    sendSynBit(AA[i]);
  }
  while(!GDO2_STATE);
  GDO0_LOW;
  while(GDO2_STATE);
  delayMicroseconds(112.5); // ~18x PW 
  // ######################################
  */
  // GDO0_IN;
  pinMode(GDO0_PIN, INPUT);
  RFMOD.SpiStrobe(CC1101_SRX);
  while ((RFMOD.SpiReadStatus(CC1101_MARCSTATE) & 0x1F) != M_SRX);
  // while intercepting in the middle of a communication, do nothing
  while (searchPowerUp(16));
  // once communication is down start searching for new communication start e.g. rampup signal from reader
  while (!searchPowerUp(8));
  //while (searchPowerUp(8));
  // search for query or select message from reader
  //nChunks = 0;
  //while (recvSynBits(RX_BUFFER, (int)BUFFER) > ((int)BUFFER-1));
  //Serial.println("[+] Delimiter");
  /*
  while (recvSynBits(RX_BUFFER+((int)BUFFER*nChunks),(int)BUFFER) < (int)BUFFER)
  {
    nChunks++;
  }
  
  Serial.println(nChunks);
  for (int i = 0; i < ((nChunks) * (int)BUFFER); i++)
  {
    Serial.print(RX_BUFFER[i]);
  }
  Serial.println();
  */

  // search for delimiter (12.5µs width --> size <= 4 zerobits)
  while (recvSynBits(DATA, 2) > 0);
  
  
  //Serial.println((RFMOD.SpiReadStatus(CC1101_MARCSTATE) & 0x1F), HEX);
  recvSynBits(RX_BUFFER, FIFO_BUFFER);
  
  Serial.print(DATA[0]);Serial.print(DATA[1]);Serial.print("#");
  for (int i = 0 ; i < FIFO_BUFFER; i++)
  {
    Serial.print(RX_BUFFER[i]);
  }
  Serial.println();

  /*
  getData0Size(RX_BUFFER,16,0);
  byte data0Size = 0;
  byte data1Size = 0;
  byte RTCalSize = 0;
  byte TRCalSize = 0;
  data0Size = getData0Size(RX_BUFFER01,16,0);
  data1Size = getData1Size(RX_BUFFER01,32,data0Size);
  RTCalSize = getRTCalSize(RX_BUFFER01,64,data0Size);
  TRCalSize = getTRCalSize(RX_BUFFER01,128,data0Size + RTCalSize);
  */
  RFMOD.SpiStrobe(CC1101_SFRX);
}

byte getData0Size(byte *data,int searchWidth,int startIndex)
{
  int i = startIndex;
  int size = 0;
  // if data[i]==0 then we are not yet in the data-0 field
  while (data[i] < 1) i++;
  Serial.print("DATA-0 [");
  while (data[i] > 0) {Serial.print("*");size++;i++;};
  while (data[i] < 1) {Serial.print(".");size++;i++;};
  Serial.print("] ");Serial.print(size);Serial.println();
  return size;
}

byte getData1Size(byte *data,int searchWidth,int startIndex)
{
  int i = startIndex;
  int size = 0;
  // if data[i]==0 then we are not yet in the data-0 field
  while (data[i] < 1) i++;
  Serial.print("DATA-1 [");
  while (data[i] > 0) {Serial.print("*");size++;i++;};
  while (data[i] < 1) {Serial.print(".");size++;i++;};
  Serial.print("] ");Serial.print(size);Serial.println();
  return size;
}

byte getRTCalSize(byte *data,int searchWidth,int startIndex)
{
  int i = startIndex;
  int size = 0;
  // if data[i]==0 then we are not yet in the data-0 field
  while (data[i] < 1) i++;
  Serial.print("RTCAL [");
  while (data[i] > 0) {Serial.print("*");size++;i++;};
  while (data[i] < 1) {Serial.print(".");size++;i++;};
  Serial.print("] ");Serial.print(size);Serial.println();
  return size;
}

byte getTRCalSize(byte *data,int searchWidth,int startIndex)
{
  int i = startIndex;
  int size = 0;
  // if data[i]==0 then we are not yet in the data-0 field
  while (data[i] < 1) i++;
  Serial.print("TRCAL [");
  while (data[i] > 0) {Serial.print("*");size++;i++;};
  while (data[i] < 1) {Serial.print(".");size++;i++;};
  Serial.print("] ");Serial.print(size);Serial.println();
  return size;
}

void delayNbit(byte n)
{
  for (byte i = 0; i < n; i++){
    while(!GDO2_STATE);
    //GDO0_LOW;
    while(GDO2_STATE);
  }
}

void sendSynBit(byte b)
{
  //delayNbit(9);
  if (b == 0)
  {
    while(!GDO2_STATE);
    GDO0_LOW;
    while(GDO2_STATE);
  }
  else
  {
    while(!GDO2_STATE);
    GDO0_HIGH;
    while(GDO2_STATE);
  }
}

int recvSynBits(byte *buffer,int size)
{
  //GDO0_IN;
  //RFMOD.SpiStrobe(CC1101_SRX);
  //delayNbit(9);
  int qsum = 0;
  for (int i = 0; i < size; i++)
  {
    while (GDO2_STATE); // wait for falling edge to check signal state
    //while (!GDO2_STATE);
    buffer[i] = GDO0_STATE;
    qsum += GDO0_STATE;
    while (!GDO2_STATE); // wait for next cycle  
  }
  return qsum;
}

void sendAsynBit(byte b, float pw)
{
  if (b == 0)
  {
    GDO0_LOW;
    delayMicroseconds(pw);
  }
  else if (b == 1)
  {
    GDO0_HIGH;
    delayMicroseconds(pw);
  }
  GDO0_LOW;
}


bool searchPowerUp(byte bits)
{
  byte data_pu[bits];
  if (recvSynBits(data_pu, bits) < 2)
   {  
    return 0;
  }
  else
  {
    return 1;
  }
}
