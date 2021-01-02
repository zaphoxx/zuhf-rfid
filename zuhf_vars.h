/*
* global vars
*/
#define BUFFER_SIZE 255

enum READER_STATES
{
	R_INIT,
	R_WAIT,
	R_START,
	R_SELECT,
	R_QUERY,
	R_QUERYREP,
	R_QUERYADJ,
	R_ACK,
	R_NAK,
	R_CW,
	R_POWERUP,
	R_POWERDOWN,
	R_TEST
};



// CONSTANTS (READER CONFIGURATION)

// Fixed number of slots (2^(FIXED_Q))  
const byte FIXED_Q              = 0;

// Termination criteria
const int MAX_INVENTORY_ROUND = 50;
const int MAX_NUM_QUERIES     = 1000;     // Stop after MAX_NUM_QUERIES have been sent

// Query command (Q is set in code)
byte QUERY_CODE[4] = {1,0,0,0};  // QUERY command
byte DR            = 0;          // TRcal divide ratio (8)
byte M[2]          = {0,0};      // cycles per symbol (FM0 encoding)
byte TREXT         = 0;          // pilot tone; 0 -> no pilot tone
byte SEL_ALL[2]    = {0,0};      // which Tags respond to the Query: ALL TAGS
byte SEL_SL[2]     = {1,1};      // which Tags respond to the Query: SELECTED TAGS
byte SESSION[4][2]    = {{0,0},{0,1},{1,0},{1,1}};      // session for the inventory round
byte TARGET        = 0;          // inventoried flag is A or B

// valid values for Q
const byte Q_VALUE [16][4] =  
{
	{0,0,0,0}, {0,0,0,1}, {0,0,1,0}, {0,0,1,1}, 
	{0,1,0,0}, {0,1,0,1}, {0,1,1,0}, {0,1,1,1}, 
	{1,0,0,0}, {1,0,0,1}, {1,0,1,0}, {1,0,1,1},
	{1,1,0,0}, {1,1,0,1}, {1,1,1,0}, {1,1,1,1}
};  


/*
* The settings below assume a pulsewidth of 12.5µs (cc1101 datarate at 80 kBaud)
* 1 TARI = 25µs <--> data0 duration = 25µs
*/
const byte DATA0[] = {1,0};
const byte DATA1[] = {1,1,1,0};
const byte DELIM[] = {0};
const byte RTCAL[] = {1,1,1,1,1,0}; // length of (data0 + data1)
const byte TRCAL[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}; // max 3 * length(RTCAL) 16*12.5µs = 200µs --> BLF = 8/200µs = 40kHz

/*
* FRAMESYNC: delim + data0 + rtcal 
*/
byte FRAMESYNC[] = {0,1,0,1,1,1,1,1,0}; // delim + data0 + rtcal
/*
* QUERYREP: framesync + 4 x data0
*/
byte QUERYREP[] = {0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0};
/*
* PREAMBLE: delim + data0 + rtcal + trcal
*/
//				  {d,da0,rtcal      ,trcal                          }
byte PREAMBLE[] = {0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};

 
byte TAGSETTLE[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

READER_STATES reader_state = R_START;