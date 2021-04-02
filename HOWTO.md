# Howto build a UHF-RFID-Reader

Welcome to this tutorial. I want to share with you my experiences I collected the past (ugh it is already almost half a year) several months trying to build an UHF-RFID-Reader. If you found this HOWTO guide you will probably already have seen the related README with some additional information on the project. (https://github.com/zaphoxx/zuhf-rfid/blob/main/README.md). The guide will be structured in several steps. As of today not all steps are available yet.
The guide describes a UHF-RFID-Reader the way I build it. I do NOT say this is the only way to do it. There are other options and if you feel like deviating from the path (e.g. using someting else then Arduino Due as MCU or design your own RF modules.) If you do so feel free to let me know I would be most curious how you did and if it shows any improvements. Now I guess lets get started!

## Things you need

There are a few things that you will need to purchase before being able to put an UHF-RFID Reader together

* Arduino Due - you can use different boards here (e.g. teensy should work to) but you will then have to rewrite most of the code. So unless you are up for an extra challenge and you know exactly what you are doing I would recommend to stick to the Arduino Due for now. The price for this Arduino Board is ~15-20 Euros depending where you purchase it. 
* RF-Modules - you will need at least two RF-modules (one as receiver and one as transceiver). The project is based on the TI CC1101 Chip and the RF modules I used have that chip onboard. !!! ATTENTION !!! The important piece here is that you need to make sure that the RF-module is designed for the frequency you want to use which should be either 868MHz or 915MHz. I used 868MHz but 915MHz should (with no guarantees) work as well. I made that mistake to and purchased rf-modules with the CC1101 chip which were not designed for 868MHz (but 433MHz) and went through a lot of pain. Be careful some vendors put 868MHz in the title but in the fineprint you will often find the other frequency. An example for modules that do work at the correct frequency can be found here: (https://www.amazon.de/gp/product/B084BSMCQG/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1); There are lots of other options around. You could even design and build your own RF-modules using e.g. (https://easyeda.com/); a set of 5 modules costs around 30 Euros on Amazon.
* Antenna for your rf-module - depending on where you purchase your rf-module an antenna might be or might be not part of it. Also some modules are designed with an SMA connector and some are not. For the modules as mentioned in the link above the antenna is part of it and you just need to solder it to the board. These antennas work fine and do the job. Once you have everything up and running you might want to modify antenna design to improve signals and range a bit.
* one or two UHF-RFID tags/cards to play with; make sure you pick the correct ones which operate at 868MHz, typically the cards operate in a wide range from ~860MHz-960MHz. !(https://www.amazon.de/-/en/Yaron-Iso18000-960MHz-H3-Alien-Plastic/dp/B072562JLS/ref=sr_1_3?dchild=1&keywords=uhf+rfid+karte&qid=1617356757&sr=8-3)
* libraries and example Arduino sketches from here (https://github.com/zaphoxx/zuhf-rfid)


## Things you might want
These are the things that are not necessary for the build but you might consider them as things will be much more difficult especially when trying to find the root cause of an issue.

* HackRF One - Thats the one I used because I had it at hand. You dont need that specifically. Any SDR (Software Defined Radio) that can be used with gnuradio and which can receive at the working frequency of 868MHz should do. Check out the following website if you dont have one (https://blog.bliley.com/10-popular-software-defined-radios-sdr). It will give you a good overview of what is available to what price. The SDR is basically just for monitoring the signals send by the reader and by the tag/card. It will definitely make things much easier when you can monitor the signals the reader and tags send out. I wouldnt recommend going through this project without that. It also helps to understand what is happening under the hood. 
* GnuRadio-Companion - Software to use your HackRF One or other SDR with. You can download the software here (https://wiki.gnuradio.org/index.php/InstallingGR)
* PCB Board with the necessary wiring. This is basically just a 'UHF-RFID-Shield' for your Arduino Due. You might get away using cables and a breadboard (it does work). But it is much better if you do have a pcb board with the necessary wiring where things are fix and there is no danger of cables getting loose and accidently frying the Arduino Board (this did happen...). The downside is of course that you wont be able to quickly change input/output pins and similar. Maybe want to do the first steps with a breadboard and once you are confident things work as expected you can switch to a custom shield.
* some basic familiarity with gnuradio, arduino, soldering and programming in general. 
* e.g. wires, solderiron, etc. things you need to get your module connected to the arduino.
* patience and persistance - thats a big one especially when things dont work as expected :)

## Things to keep in mind

* Reader Range - you might have read or heard that UHF-RFID plays big with large scan ranges (few meters). Well, you wont get that here. If you are lucky you will get something in the order of about 1-2cm. So dont put your expectations to high regarding the reader range. Once you have the reader working you can experiment with different antennas, positioning etc. to see if you can improve the scan range. Also the area in which the reader can read/write the tag might be in a very small window in space. So if it doesnt work right away dont fret you might need to find the sweet spot where the reader can read/write the card. 
* Reader Commands - the code as it is at the moment does not support all the possible commands a commercial reader can send to the tag/card. Check out the README.md for details on that. Feel free to add new commands to your code and improve your setup.

## Things connected - a first check

In this section I will go through how the RF-Modules should be connected to the Arduino Due starting with RF-Module that will be the tranceiver part. At the end you will be able to see the request the module sends to the card and hopefully the tag response with the 'HackRf One' (or anotherone). (this step you can try on a breadboard). So you need
* Arduino Due (for this first part you can also use an Arduino Nano, but the wiring and code is slightly different. I will add some schematics and code for the Nano further below.)
* RF-Module
* Shield or Breadboard and cables

The CC1101 Chip on the RF-Module can be programmed using SPI communication (https://www.arduino.cc/en/reference/SPI)(https://en.wikipedia.org/wiki/Serial_Peripheral_Interface). But we are NOT going to connect the first RF-Module (TX) to the SPI pins on the Arduino Board. Instead we will connect it to the USART1 (RX2/TX2 on the Board) and reprogram the USART1 so it can be used as SPI. We will save the SPI pins for later for the RX RF-Module. 
Now for the TX you need to connect the MISO to RX2 and MOSI to TX2 pin respectively of the Arduino Due. The CLK needs to be connected to PIN A0 (PIN 78) on the Arduino Board. The select PIN CSN needs to be connected to RTS1 which is PIN 23 (A.14) on the Board. The CC1101 operates at a voltage of 3.0 - 3.7 V so the RF-Modules VDD should be connected to the 3.3V connector on the Arduino (not the 5V connector).
The CC1101 Chip has two output PINS called GDO0 and GDO2 (there is also a GDO1 if you are wondering but it is not broken out on the RF-module board and we wont need it) which also need to be connected to the Arduino. These are free to choose but see below for the settings I use. For reference you can also check check out the Arduino Due Pin Layout her ![PinLayout](https://www.14core.com/wp-content/uploads/2015/06/Arduino-Due-Pinout-Diagram-Illustration-Complete-Pin-Diagram-1.jpg).
For the setup I use with the Arduino I have the following connections:

* RF-MODULE CSN --> Arduino Pin 23
* RF-MODULE MISO --> Arduino RX2
* RF-MODULE MOSI --> Arduino TX2
* RF-MODULE CLK --> Arduino A0
* RF-MODULE GDO0 --> Arduino Pin 34
* RF-MODULE GDO2 --> Arduino Pin 36
* RF-MODULE VDD --> Arduino 3.3V connector
* RF-MODULE GND --> Arduino GND
Reference numbers are as printed on the board.
![Wiring](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/connections_tx_image.jpg)
With the wiring complete we will next take a look on how to get the Arduino IDE started and a load a first test program. Keep fingers pressed so we will see a tag response in our SDR monitor.

## Things get started - observe your first tag response (hopefully!)

Ok, so now you have the wiring complete and are eager to see the rf module in action. Ok!
Fire up your Arduino IDE and make sure you have the libraries (SPI_UART_CC1101 and ZUHF_CC1101) imported. You find these in the github repository under '/libraries/'. Check out (https://www.arduino.cc/en/guide/libraries) on how to manually import libraries.
Once you have the libraries setup you need to load the Arduino sketch 'uhf-rfid_PART1' into the IDE. Make sure the additional file 'UHF-RFID.h' is present in the same folder.
I am assuming you are using the same pins as described above if not this will not work right away and you will have to make adjustments in the appropriate files (e.g. SPI_UART_CC1101.h).

Now start your SDR device with gnuradio-companion. For an example setup with HackRF One see image below (![gnuradio_setup](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/gnuradio_setup.png)). Make sure you are using either 'Complex to Mag' or 'Complex to Mag_square'. The source to use will depend on your SDR but with HackRF the osmocom source is good to go. Set the frequency to 868MHz and the sample rate to 1e6 (you can go lower if you are having issues here, but 1e6 should be good in most cases). For visualization I chose 'QT Gui Time Sink' with Number of Points set to 1e6 (same here lower it down if you are experiencing problems). Start your gnuradio setup to start listening.

Once you uploaded the sketch to the arduino due and all is connected properly you should start seeing a signal on gnuradios monitor window (similar the images). ![signal1](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhf-rfid_part1_signal01.png)![signal1](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhf-rfid_part1_signal02.png) The arduino sketch is setup in a way that it keeps repeatedly sending a simple query request. Once you hold a uhf rfid card / tag close to your antenna you should start seeing a tag response on the monitor window similar to the images below ![signal3](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhf-rfid_part1_signal03.png)![signal4](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhf-rfid_part1_signal04.png).

Don't worry if you do not see a tag response right away. As long as you can observe a proper query signal it might just take some patience to find the right distance to the antenna and orientation tag <-> antenna. Once you get a signal play around with the antennas orientation (SDR antenna vs. rf-module antenna vs. tag orientation) to improve your observed signal.

Congratulations! Your rf-module can "communicate" with UHF-RFID card/tag.

## Things behind the curtain - what is happening ?

In this section I will briefly go thru the main code of the arduino sketch, what it does and what you see in the signals you observed.



