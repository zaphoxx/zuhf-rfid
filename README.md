# zuhf-rfid
Blog about my journey investigation uhf-rfid with arduino and cc1101. This project has already been going on with more are less time spent for a few weeks and I decided now to put the information I learned along the way down as a resource mainly for myself but make it available for others who might be interested.

## Background
The whole project came out of another project related to UHF-RFID but using SDR. The goal of the other project was to get an RFID reader up and running using SDR. Work has already done on this field by nkargas and the project can be found here https://github.com/nkargas/Gen2-UHF-RFID-Reader. Based on nkargas project I could get a very basic RFID reader up and running. However the reader as developed by nkargas only reads out the tags EPC and basically stops there. Adam Laurie has started to take this a step further using a bladeRF and has published his fork of the original reader here https://github.com/AdamLaurie/Gen2-UHF-RFID-Reader.
With SDR the critical part is usually the time between receiving the RN16 value from the tag and responding with an ACK (with the received RN16) to receive the actual EPC. Due to latencys this can be difficult to satisfy and makes the SDR solution sometimes unreliable.This problem has been outlined by the original authors and by others trying to implement the uhf rfid reader with SDR.

## Another Approach
A friend noted that there is a TI Chip CC1101 which can be used (from the datasheet)
* Ultra low-power wireless applications operating in the 315/433/**868**/**915** MHz ISM/SRD bands
* Wireless alarm and security systems
* Industrial monitoring and control
* ...
In addition it can handle ASK modulation which is the used modulation technique for UHF RFID. So this sounds promising and theoretically one should be able to do at least the same as with the above mentioned SDR solution avoiding the reliability issues.
Also there exists an Arduino library for the CC1101. Therefore using an Arduino (e.g. the Nano) for controlling and programming the CC1101 seems like a straightforward choice.

## Roadblock(s)
There are a few roadblocks on getting to a working solution. The first and most important one would be that even though a could reproduce the above mentioned SDR solution I actually have only negligible experience with RF not even talking about how the CC1101 works and how it can "programmed" to do my bidding. Which means there will be (and already was as I am already in the middle of it) tons of research and learning necessary on the way. That is the main reason I started this block hoping someone will stumple accross it and perhaps see the mistakes I made (and believe me I did a lot already) and perhaps she has some additional insight and can give me some helpful advice.
Another issue I encountered was that even though it seems like some people have already tried the same before there is very little information around about how specifically they did realize it. There are some nice publications available but these dont go into to much detail regarding the technical realization. TI has a forum where you can find some information in particular regarding operation of the cc1101 but often the advice found will be to use the SmartRF software (it is free to download and use but you have to login and sign with blood that you wont use it for any mischief) to e.g. find valid register settings.

## Resources
This a list of resources I have been using for this project. It is not a final list and new links or documents might be added in time. The list is not strictly in order of importance but I divided it into two sections where the top list are necessary resources like datasheets and specifications, whereas the second list is a list of helpful resources I found during my research.

Datasheets/Specifications/libs
* CC1101 Datasheet https://www.ti.com/lit/ds/symlink/cc1101.pdf
* https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf
* Arduino Lib for CC1101: I started by using https://github.com/simonmonk/CC1101_arduino and modifying it as needed in the further process. There are other CC1101 libraries for Arduino available but this seemed to me the simplest to get started with and to explore the capabilities of the cc1101 chip.
* cc1101 - Programming Output Powers https://www.ti.com/lit/an/swra151a/swra151a.pdf
* cc1101 - setting registers for ASK/OOK modulation https://www.ti.com/lit/pdf/swra215
* cc1101 - usage of gdox pins - https://www.ti.com/lit/an/swra121a/swra121a.pdf?ts=1609398291195&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC1101%253FkeyMatch%253DASK%2BMODULATION%2BCC1101%2526tisearch%253DSearch-EN-everything%2526usecase%253DGPN
* cc1101 - packet transmission basics - https://www.ti.com/lit/an/swra109c/swra109c.pdf?ts=1609543498401&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC1101%253Futm_source%253Dgoogle%2526utm_medium%253Dcpc%2526utm_campaign%253Depd-null-null-GPN_EN-cpc-pf-google-wwe%2526utm_content%253DCC1101%2526ds_k%253D%257B_dssearchterm%257D%2526DCM%253Dyes%2526gclsrc%253Daw.ds%2526%2526gclid%253DCjwKCAiArbv_BRA8EiwAYGs23Cd4vk5J0NyYSean69jrkUv2pj_sYKPD1O72e1YVFU3YXXbui-JnrhoCMYsQAvD_BwE


Helpful resources
* https://www.eetimes.com/tutorial-radio-basics-for-uhf-rfid-part-i/
* https://www.grspy.com/cloning-the-remote-control-of-an-rc-switch-using-ti-cc1101/
* https://e2e.ti.com/support/wireless-connectivity/other-wireless/f/667/t/157309
* https://home.zhaw.ch/~kunr/NTM1/praktikum/standards_and_literature/E2E_chapter03-rfid-handbook.pdf
* https://greatscottgadgets.com/sdr/
* https://www.ti.com/product/CC1101?keyMatch=ASK%20MODULATION%20CC1101&tisearch=Search-EN-everything&usecase=GPN

## Hardware
I am currently using the following hardware:
* Instead of purchasing just the chip and having to solder things myself I decided to opt out for a already available module https://www.alibaba.com/product-detail/Taidacent-RF-Wireless-Transceiver-Module-Radio_1859941763.html for a few $ per piece.
* Arduino Nano as MCU
* HackRF One (mainly for monitoring and basic analysis of the signals produced by the CC1101 module)

## Basic Settings

To operate the CC1101 you have to set registers to the appropriate values. It can be very confusing on how the registers need to be setup and the cc1101 datasheet is quite impressive but the explanations in the documentation are mainly targeted towards a reader that already understands how these things work in general. However an approach that was quite helpful was to use the smartRF software to find some basic settings and then manually adjust some additional registers as needed.

To get an idea of the settings needed for UHF RFID in general the specification for gs1-epc-gen2v2-uhf can be used as guideline (https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf).

![ASK Envelope](https://github.com/zaphoxx/zuhf-rfid/blob/main/ASK-Modulation-RF-envelope.png)

![POWERDOWN/UP](https://github.com/zaphoxx/zuhf-rfid/blob/main/powerdownup-waveform-parameters.png)

![PIE Symbols](https://github.com/zaphoxx/zuhf-rfid/blob/main/PIE-Symbols.png)

```
  CC1101 Freq = 868MHz
  CC1101 DataRate = 80kBaud
  CC1101 Modulation = ASK/OOK
  byte paTable[8] = {0x27,0xc0,0x00,0x00,0x00,0x00,0x00,0x00};

  1 Pulsewidth = 12.5µs
  1 TARI = 25µs (DATA0)
  const byte DATA0[] = {1,0};
  const byte DATA1[] = {1,1,1,0};
  const byte DELIM[] = {0};
  const byte RTCAL[] = {1,1,1,1,1,0}; // length of (data0 + data1)
  const byte TRCAL[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}; // max 3 * length(RTCAL) 16*12.5µs = 200µs --> BLF = 8/200µs = 40kHz
```

