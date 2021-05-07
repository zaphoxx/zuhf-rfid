## Recap

In the previous HOWTO_PART2 I described what you need and how to get your first interaction with a uhf-rfid tag going. Hopefully you could observere a tag response with your SDR hardware.
I did not go into detail about what the arduino sketch is actually doing and was is happening when the reader talks to the tag. In the following I will try to shed some light on this.

## Things behind the curtain - what is happening ?

In this section I will briefly go thru the main code of the arduino sketch, what it does and what you see in the signals you observed.

Usually the first important component of an arduino sketch is the ```setup()``` function. However in our case there is not happening much in there.
```c
void setup()
{
  // delay to avoid known reset issue - see also https://forum.arduino.cc/index.php?topic=256771.75
  delay(1000);
  /* SERIAL CONNECT */
  Serial.begin(250000);
  for (int i = 0; i < sizeof(CWA); i++) CWA[i]=0xff;
  reader_state = R_INIT;
}
```
We establish a serial communication with a 250000 baudrate, preset the array CWA and set the reader_state to R_INIT. The more interesting action happens then in the function ```loop()```.

This is the full loop() function:

```
void loop()
{
  switch(reader_state)
  {
    case R_INIT:
      counter = 0;
      tx_version = TX_UNIT.Init();
      delay(10);
      if ((tx_version == 0x14 or tx_version == 0x04))
      {
        Serial.println("[CC1101] Modules Check - OK");
        TX_UNIT.SpiStrobe(CC1101_SFSTXON);
        delay(500);
        Serial.println("**** START RUN ****");
        reader_state = R_START;  
      }
      else
      {
        Serial.println("[CC1101] Error on CC1101 module initialization");
        reader_state = R_WAIT;
        delay(100);
      }
      break;
      
    case R_START:
      /* LET FIFOBUFFER EMPTY FROM A PREVIOUS RUN AND SET TX INTO IDLE MODE */
      while ((TX_UNIT.SpiReadStatus(CC1101_TXBYTES) & 0x7f) > 0);
      TX_UNIT.SpiStrobe(CC1101_SIDLE);
      delay(5);
      /* ********** START TX AND SEND CW FOR TAG SETTLE ********** */
      TX_UNIT.SpiStrobe(CC1101_SFTX);
      delay(5);
      TX_UNIT.SpiStrobe(CC1101_STX);
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 20);
      /* ********** *********************************** ********** */
      reader_state = R_QUERY;
      break;

    case R_QUERY:
      send_default_query();
      TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 40);
      reader_state = R_START;
      delay(20);
      break;

    default:
      reader_state = R_INIT;
      break;
    }
}
```
But lets take a look step by step. First thing you will notice is the switch statement. This basically controls in which state the reader is in for the current loop. For this example the reader can be in three states: R_INIT, R_START and R_QUERY. 

Let's check the R_INIT state first. In this state the RF-Module is first initialized using ```TX_UNIT.init()```. Which means the registers of the CC1101 chip are configured as needed (e.g. set working frequency to 868MHz) and an initial reset of the chip. In addition the init() function returns a value which corresponds to the chips versionnumber. This value can be used to check if chip is still working fine. This check is done in the following if statement. If everything is ok the reader will flush the CC1101 fifobuffer ```X_UNIT.SpiStrobe(CC1101_SFTX);``` and switch the chip into TX mode ```TX_UNIT.SpiStrobe(CC1101_STX);```. Now the chip is ready to send out signals.
The first signal we send out is a continous wave signal for a length of 20bytes which corresponds in our case 20 x 4 x 12.5 [µs] = 1000 [µs]. ```TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 20);``` (we write the data into the fifo buffer and the chip takes care of actually sending the signal out). This cw signal serves as activation signal to powerup and activate the tag. Right after sending the cw (continues wave) the reader switches into R_QUERY state and we send the actual query to the chip ```send_default_query();```. This function builds a default query signal so we dont have to worry about any specific parameters for now. Once this is out of the way we send out another cw signal ```TX_UNIT.SpiWriteBurstReg(CC1101_TXFIFO, CWA, 40);``` and put the reader back into R_START status. After that the cycle repeats until we poweroff the arduino.



