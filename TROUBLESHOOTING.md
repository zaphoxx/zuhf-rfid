# TROUBLESHOOTING

## ISSUE - Tag does not respond to an REQ_RN request

### Description
If the tag properly responds to the QUERY request and confirms the interrogators ACK with a valid EPC response but fails to respond to an subsequent REQ_RN request then
this might be a timing issue. Once the tag replies to your REQ_RN timing is no problem anymore but up to this point the interrogator has to respond or send requests within ~ 500ms
after tags last reply.

### Solution
So for the specific issue where you get a proper EPC response the timing within the function ```read_epc()``` in the ZUHF_CC1101 library might be too long. Check the description
in the function and try to lower the settings for the variables x1, x2 within the function. See function snippet and description below.
I found x1=12,x2=32 or x1=14,x2=30 to be good working values. I experienced that for x2=32 the timing might not work for some rfid chips (e.g. MONZA4) but x2=30 worked all good.
Whereas for a Alien Higgs3 Chip the x2=32 value worked all good.
See also function snippet below for more information:

```
/********************************************************************************************************************************
* FUNCTION NAME: read_epc()
* FUNCTION     : searches and reads epc data from response
* INPUT        : epc_data structure to be filled in
* OUTPUT       : returns true if crc16 check is ok; if not it will still fill the
*                epc_data structure but the crc16 flag will set to false;
* INFO         : pay special attention to the variables x1,x2 below; 
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
   *  then you might try to tune the value for TX_UNIT.SendCW(x1/x2) a bit. Try to make it as low as
   *  possible. Values > 14 are typically to long and will violate the max repsonse time reader -> tag.
   *  If the value is to low then the reader might not have enough time to capture the radiosignal.
   */

  // Sweet spot values are currently x1=12, x2=32 (or x1=14, x2=30) for the SendCW(x1/x2) values below
  byte x1 = 12;
  byte x2 = 32;
  TX_UNIT.SendCW(x1); // x1: critical value, change only if really necessary and you know what you are doing!
  while(TX_GDO0_STATE)
  {
    if (RX_GDO0_STATE)
    {
      // x2: TX_UNIT.SendCW(32)
      // If tags with another chip (e.g. Monza or Alien Higgs) dont respond to an req_rn then this CW timing of
      // the read_epc might be too large which causes the tag to fall back into its initial state 
      // (from acknowledged --> arbitrate); the req_rn command will then be ignored by the tag
      // In order to stay within the response time restraints you can try to lower the value from below TX_UNIT.SendCW(xx)
      TX_UNIT.SendCW(x2); // x2: similar to above but not as sensitive as above.
      while(RX_GDO0_STATE);
      RX_UNIT.SpiReadBurstReg(CC1101_RXFIFO, epc_data, PLEN);
      found = true;
      break;
    }
  }
```
