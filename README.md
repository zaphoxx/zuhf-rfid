# Creating Your Own UHF RFID Reader From Scratch
# ZUHF-RFID by Zaphoxx
## TL;DR
This project has already been going for a while with longer breaks inbetween and I decided to put the information I learned along the way down as a resource mainly for myself but also to make it available for others who might be interested in building a UHF RFID reader/writer from scratch. The image below shows the UHF-Reader. It works fine on very small distances (1-2cm) for the following commands.
* READ - read from any unlocked memory area
* WRITE - write to any unlocked memory area
* LOCK - specify memory area to password protect or permalock (the exact functionality depends on the memory area itself)
* ACCESS - read/write/lock password protected memory areas
* TEARS - this is a special functionality to test if tearing during e.g. the write operation shows some interesting behaviour (DANGEROUS! This can make your card useless!)
* TEARLOCK - similar as TEARS but in conjunction with the LOCK functionality (DANGEROUS! This can make your card useless!)

The commands have been tested on the following types of tags: Impinj Monza 4E, Alien Higgs 3, Impinj Monza R6 (limited functionality as there is no user memory and effectively no reserved memory, lock mechanism works differently for this chip; see also chips datasheet). I have another couple of tags where the commands work all fine but I do not have any information on the chip type for these. 

I started putting a HOWTO guide (https://github.com/zaphoxx/zuhf-rfid/blob/main/HOWTO.md) together in case you are interested in going into that direction. As of today Part1 and Part2 of the HOWTO is ready, any feedback for improvements is highly appreciated. Other than that enjoy. Further sections will be added in the future as soon as i have the time for it.

If you already put the hardware together and you just want to get started reading/writing to a tag you can refer to (https://github.com/zaphoxx/zuhf-rfid/blob/main/ZUHF-RFID.md) which briefly describes how to use the cli zuhf-cli.py.

![UHF-READER-SHIELD](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhfreader2.png)

## Background
The whole project came out of another project related to UHF-RFID but using SDR. The goal of the other project was to get an RFID reader up and running using SDR. Work has already been done on this field by nkargas and the project can be found here https://github.com/nkargas/Gen2-UHF-RFID-Reader. Based on nkargas project I could get a very basic RFID reader up and running. However the reader as developed by nkargas only reads out the tags EPC and basically stops there. Adam Laurie has started to take this a step further using a bladeRF and has published his fork of the original reader here https://github.com/AdamLaurie/Gen2-UHF-RFID-Reader. See also Adam Laurie great talk about the topic here: https://www.youtube.com/watch?v=QKi1OH8Zstk .
With SDR the critical part is usually the time between receiving the RN16 value from the tag and responding with an ACK (with the received RN16) to receive the actual EPC. Due to latencys this can be difficult to satisfy and makes the SDR solution sometimes unreliable.This problem has been outlined by the original authors and by others trying to implement the uhf rfid reader with SDR.

## Another Approach
A friend noted that there is a TI Chip CC1101 which can be used (as stated in the datasheet) for
* Ultra low-power wireless applications operating in the 315/433/**868**/**915** MHz ISM/SRD bands
* Wireless alarm and security systems
* Industrial monitoring and control
* ...
In addition it can handle ASK modulation which is the used modulation technique for UHF RFID and has a maximum baudrate of 500 kBauds. So this sounds promising and theoretically one should be able to do at least the same as with the above mentioned SDR solution avoiding the reliability issues.
Also there exists an Arduino library for the CC1101. Therefore using an Arduino (e.g. the Nano) for controlling and programming the CC1101 seems like a straightforward choice.

## Roadblock(s)
There are a few roadblocks on getting to a working solution. The first and most important one would be that even though a could reproduce the above mentioned SDR solution I actually have only negligible experience with RF not even talking about how the CC1101 works and how it can "programmed" to do my bidding. Which means there will be (and already was as I am already in the middle of it) tons of research and learning necessary on the way. That is the main reason I started this block hoping someone will stumple accross it and perhaps see the mistakes I made (and believe me I did a lot already) and perhaps she has some additional insight and can give me some helpful advice.
Another issue I encountered was that even though it seems like some people have already tried the same before there is very little information around about how specifically they did realize it. There are some nice publications available but these dont go into to much detail regarding the technical realization. TI has a forum where you can find some information in particular regarding operation of the cc1101 but often the advice found will be to use the SmartRF software (it is free to download and use but you have to login and sign with blood that you wont use it for any mischief) to e.g. find valid register settings.

With using the Arduino there is the caveat that the memory available is quite limited and you have to be careful on how you are using your resources when programming the Firmware. At the moment it looks like I should get along with the memory space available. But in case I run into memory issues there is the option to use e.g ~~a Raspberry Pi~~ Arduino Due or similar as alternative. ~~For the Raspberry Pi there are also cc1101 libraries available and transferring the code from Arduino to the Pi shouldnt be an issue.~~

## Resources
This a list of resources I have been using for this project. It is not a final list and new links or documents might be added in time. The list is not strictly in order of importance but I divided it into two sections where the top list are necessary resources like datasheets and specifications, whereas the second list is a list of helpful resources I found during my research.

SDR projects on UHF-RFID on github
* https://github.com/nkargas/Gen2-UHF-RFID-Reader - I have been using this as main source for general uhr-rfid related settings, code snippets etc. (in combination with the specification documents on gen2v2 uhf listed below). 
* https://github.com/AdamLaurie/Gen2-UHF-RFID-Reader - this is a fork of the previous link but focusing more on the implementation on a bladerf rather then the usrp.

Datasheets/Specifications/libs
* CC1101 Datasheet https://www.ti.com/lit/ds/symlink/cc1101.pdf
* https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf
* Arduino Lib for CC1101: I started by using https://github.com/simonmonk/CC1101_arduino and modifying it as needed in the further process. There are other CC1101 libraries for Arduino available but this seemed to me the simplest to get started with and to explore the capabilities of the cc1101 chip.
* cc1101 - Programming Output Powers https://www.ti.com/lit/an/swra151a/swra151a.pdf
* cc1101 - setting registers for ASK/OOK modulation https://www.ti.com/lit/pdf/swra215
* cc1101 - usage of gdox pins - https://www.ti.com/lit/an/swra121a/swra121a.pdf?ts=1609398291195&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC1101%253FkeyMatch%253DASK%2BMODULATION%2BCC1101%2526tisearch%253DSearch-EN-everything%2526usecase%253DGPN
* cc1101 - packet transmission basics - https://www.ti.com/lit/an/swra109c/swra109c.pdf?ts=1609543498401&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC1101%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Depd-null-null-GPN_EN-cpc-pf-google-wwe%2526utm_content%253DCC1101%2526ds_k%253D%257B_dssearchterm%257D%2526DCM%253Dyes%2526gclsrc%253Daw.ds%2526%2526gclid%253DCjwKCAiArbv_BRA8EiwAYGs23Cd4vk5J0NyYSean69jrkUv2pj_sYKPD1O72e1YVFU3YXXbui-JnrhoCMYsQAvD_BwE
* Arduino library for CC1101


Helpful resources
* https://www.eetimes.com/tutorial-radio-basics-for-uhf-rfid-part-i/
* https://www.grspy.com/cloning-the-remote-control-of-an-rc-switch-using-ti-cc1101/
* https://e2e.ti.com/support/wireless-connectivity/other-wireless/f/667/t/157309
* https://home.zhaw.ch/~kunr/NTM1/praktikum/standards_and_literature/E2E_chapter03-rfid-handbook.pdf
* https://greatscottgadgets.com/sdr/
* https://www.ti.com/product/CC1101?keyMatch=ASK%20MODULATION%20CC1101&tisearch=Search-EN-everything&usecase=GPN

## Hardware
I am currently using the following hardware:
* Instead of purchasing just the chip and having to solder things myself I decided to opt out for a already available module ~~https://www.alibaba.com/product-detail/Taidacent-RF-Wireless-Transceiver-Module-Radio_1859941763.html~~ for example from here (https://www.amazon.de/gp/product/B084BSMCQG/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1) for a few $ per piece.
* ~~Arduino Nano as MCU~~ Arduino Due
* HackRF One (mainly for monitoring and basic analysis of the signals produced by the CC1101 module)
* (optional) a custom pcb board to avoid pins getting loose and to enhance connection of the spi wirings. (but all can also be done by using wires and a breadboard.)

![Cthuloid UHF-RFID](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/cthuloid_pcb.jpg)
![Holistical UHF-RFID](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/holistical_pcb.png)

## Basic Settings

To operate the CC1101 you have to set registers to the appropriate values. It can be very confusing on how the registers need to be setup and the cc1101 datasheet is quite impressive but the explanations in the documentation are mainly targeted towards a reader that already understands how these things work in general. However an approach that was quite helpful was to use the smartRF software to find some basic settings and then manually adjust some additional registers as needed.

To get an idea of the settings needed for UHF RFID in general the specification for gs1-epc-gen2v2-uhf can be used as guideline (https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf).

![ASK Envelope](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/ASK-Modulation-RF-envelope.png)

![POWERDOWN/UP](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/powerdownup-waveform-parameters.png)

![PIE Symbols](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/PIE-Symbols.png)

```
  CC1101 Freq = 868MHz
  CC1101 DataRate = 80kBaud
  CC1101 Modulation = ASK/OOK
  byte paTable[8] = {0x27,0xc0,0x00,0x00,0x00,0x00,0x00,0x00}; // putting the low end to 0x27 is not necessary and you can also use 0x00 instead

  1 Pulsewidth = 12.5µs
  1 TARI = 25µs (DATA0)
  const byte DATA0[] = {1,0};
  const byte DATA1[] = {1,1,1,0};
  const byte DELIM[] = {0};
  const byte RTCAL[] = {1,1,1,1,1,0}; // length of (data0 + data1)
  const byte TRCAL[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}; // max 3 * length(RTCAL) 16*12.5µs = 200µs --> BLF = 8/200µs = 40kHz
```
To make live easier I ordered a very simply pcb to have the arduino and the cc1101 module properly connected and soldered on the same board.
![pcb board setup](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/rfid-setup.jpg)

After some trial and error and getting the coding to work properly I finally got a query signal going out. Unfortunately I do not get a RN16 response from tag. This is the point where I seem to hit a brick wall. From what I can say the settings look good. I also tried to send out a query followed by several queryrep commands but nothing seems to get the tag to give me any response. (see query signal below). 
The current full signal is (1500µs settle-time for the tag + query command + 250µs cw + x times queryrep with cw + powerdown) and this sending out repeatedly.
I do monitor the signals with the HackRF. I checked the SDR solution and there I can clearly see the tag response to the query/queryrep. So I will need to do some more analyzing of the SDR solution to see if I am still missing something. (if you have any idea on why I do not getting a signal, feel free to drop me a message).
![query signal](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/query-signal.jpg)
![full sequence](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/query_queryrep-sequence.jpg)

From what i see comparing the sdr approach with the cc1101 approach the main difference are the powerlevels reaching the tag. The usrp provides a much higher powerlevel then the cc1101 does. So i might need to amplify cc1101 output power to get a tag response.
Not sure if this is it but its the one difference that sticks out.

## Dont trust the products headline ##
Painful lesson learned. The modules I ordered are not designed for 868 MHz but for 430 MHz instead. So if you are also going into that direction make sure the modules you order are specifically for 868 MHz. Don't trust the main description, checkout the detailed description. In my case the description stated 868 MHz but in the detailed description it did say 430MHz instead. It can be really frustrating (and time and coffee consuming) when the module you have does not work the way you think it should. Until I get the new actual modules I will use the ones I have as workaround. As you can see in the next section, you can get a signal but you have to basically glue the modules right to the tag at the right location on the tag and even then you might still not get a signal. (a little tinfoil at the right location might help to get more of the signal reflected on the the tag).

## First Success - Seeing a Tag Response ##
Woohoo! I finally got a tag response. The setup is quite improvised and the tag and antennas are basically in direct contact but now that I do have a signal I can work with that and see how to improve distance, power etc.

![TagResponse](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/tag-RN16-response.jpg)

![Improvised Setup](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/improvisedSetup.jpg)

So lets check a response and see if the structure (FM0 encoded reply) makes sense with what we would expect. From the image below you can see that the signal we identified as tag response aligns with the expectations. The tag response consists of a FM0 preamble, a RN16 and a dummy data1 bit.

![TagResponse Example](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/tagRN16-example.png)

## New Considerations ##
So as I already mentioned above the space availabe especially for dynamic memory is quite tight on the Arduino. Therefore I am currently considering if I should move over to an old RaspberryPi now before moving on with the actual UHF-RFID protocol. There is a raspberry library available by spaceteddy (https://github.com/SpaceTeddy/CC1101) which I will take a look into to see how much additional work this might take. The thing is that I will have to port to raspberry later anyway so it would make sense to do it right away as I am not an experienced programmer and trying to squeeze everything into the Arduino might cause me more headaches then necessary and it would distract me from the actual fun part of this project anyways.

## The RaspberryPi debacle ##
Back to square one. After some reading and experimantation with an old raspberry pi I had at hand I realized that this is not a good option at all. The raspberry is not suited for this kind of realtime application. What happens is that due to the interrupts on the raspi the module drops the TX and suddenly stops sending anymore which is really bad when you try to interact with an passive tag that keeps itself alive relying on the same. After 3 days of trying to still convince the raspiberry NOT to drop the signal I decided to go back to the arduino which did work fine so far.

## Getting the RN16 ##
Still working with the current modules (the ones for the 430MHz) and messy setup (everything taped together in close proximity). What I do have now is the following:

* TX unit: Arduino + cc1101 module
* RX unit: Arduino + cc1101 module

(The other option would be to connect the RX module as second slave to the arduino which does work fine but the wiring is getting very messy then.)
Both units can communicate via the serial connection and are additionally connected to use a digital trigger (to trigger the RX on/off);
The serial connection will be necessary to communicate the captured RN16 to the TX module so the reader can send a valid ACK signal to the tag.

The current status is that I can read out (not very reliable yet due to the used modules but it works) the RN16. From the screenshots below you can see that the receiver can read the full tag response as expected. You can see the FM0 preamble and the actual RN16 differently colored in the image. The FM0 preamble is expected be (001011011100) followed by the RN16 + 1 dummy bit (which in this case is just 11 at the end; the arduino serial.print() omits leading zeros, so these need to be added in case the byte shows less then 8 bits).

![RN16 Signal from RX module](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/rn16_signal.png)

## Time for upgrades ##
The modules I used before were specifically designed for around 430MHz. I ordered new ones that are clearly designed for 868MHz instead. Also I decided to upgraded the Arduino to an Arduino Due, to be on the safe side and to have the option to add some controls, display etc. in the long run. ~~The receiver part can still use the Arduino Nano and communicate with the TX unit via serial communication to exchange e.g. the captured RN16 and other information.~~ Unfortunately the Serial Communication does not necessarily within the same loop. So instead of using a second arduino unit I connect both cc1101 modules to the arduino due. This requires reprogramming of the USART0 (or USART1) to function as SPI interface. (More information on how to do that can be found here https://forum.arduino.cc/index.php?topic=283766.0 and in the Atmel SAM3X8x datasheet https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjqw-nd8N3uAhUUnBQKHSFfDK0QFjAAegQIAhAC&url=https%3A%2F%2Fww1.microchip.com%2Fdownloads%2Fen%2FDeviceDoc%2FAtmel-11057-32-bit-Cortex-M3-Microcontroller-SAM3X-SAM3A_Datasheet.pdf&usg=AOvVaw1rt7QIPIgPCcuY6Eg1SAzz) 

The new modules arrived and if they work (some don't which is again very frustrating as lot of time goes into finding the root cause) but if they work the signal is as expected much better. The tag/card still needs to be VERY close (1-2 mm) to the TX antenna to get the tags RN16 response. I guess I will have to live with that for now until I to my own design and allow for different antennas to be attached.

## One step foward two steps back ##
Unfortunately I accidently killed my Arduino Board. A wire got loose, touched the board somewhere and ** poof ** lights out. Will need to order a new one which will take a couple of days before I will be able to continue working on the project. Seems like I am stuck at this stage for a while now. Hopefully by the weekend I will have all I need to get things foward next week.

## Getting tags EPC ##
![Reader-Tag-Communication](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/successfull_tag_epc_signal.jpg)
![Tag Data From Response](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/tag_data_epc.jpg)

# Putting in all together
It has been around 3 months now since I started this project and I have come a long way. It is finally working. I do have a simple but functional UHF RFID reader for 868MHz. Tags that are not password protected can be modified with these commands. For password protected tag I still need to implement the ACCESS command which will be next on the list. Currently the following commands are fully functional and implemented:

* QUERY
* ACK
* REQ_RN
* READ
* WRITE
* LOCK

TODO:
* ACCESS
* KILL
* AUTHENTICATE and related commands
* ...

In addition to the above commands I will start putting together a simple gui so that it is more user friendly to use.
The "reader" can read the tag in a distance range of about 1-2 cm. For the write operation to work properly the tag needs to be in the 1cm range as the tags chip needs more power for write operations.

I will be starting to setup a Howto guide in case someones wants to rebuild the reader for her own experiments. Below some screenshots / fotos of the reader in operation.

![a](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhfreader1.png)
![b](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhfreader3.png)
![c](https://github.com/zaphoxx/zuhf-rfid/blob/main/images/uhfread3.png)

