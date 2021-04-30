## Recap

In the previous HOWTO_PART2 I described what you need and how to get your first interaction with a uhf-rfid tag going. Hopefully you could observere a tag response with your SDR hardware.
I did not go into detail about what the arduino sketch is actually doing and was is happening when the reader talks to the tag. In the following I will try to shed some light on this.

## Things behind the curtain - what is happening ?

In this section I will briefly go thru the main code of the arduino sketch, what it does and what you see in the signals you observed.

Usually the first important component of an arduino sketch is the ```setup()``` function. However in our case there is not happening much in there.
```
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


