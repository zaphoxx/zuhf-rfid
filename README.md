# zuhf-rfid
Blog about my journey investigation uhf-rfid with arduino and cc1101. This project has already been going on with more are less time spent for a few weeks and I decided now to put the information I learned along the way down as a resource mainly for myself but make it available for others who might be interested.

## Background
The whole project came out of another project related to UHF-RFID but using SDR. The goal of the other project was to get an RFID reader up and running using SDR. Work has already done on this field by nkargas and the project can be found here https://github.com/nkargas/Gen2-UHF-RFID-Reader. Based on nkargas project I could get a very basic RFID reader up and running. However the reader as developed by nkargas only reads out the tags EPC and basically stops there. Adam Laurie has started to take this a step further using a bladeRF and has published his fork of the original reader here https://github.com/AdamLaurie/Gen2-UHF-RFID-Reader.
With SDR the critical part is usually the time between receiving the RN16 value from the tag and responding with an ACK to receive the actual EPC. The time difference should not be longer than RTCal (Tari * 2.5) which is typically in the range of (with a max Tari of 25µs) < 50µs. Due to latencys this can be difficult to satisfy and makes the SDR solution sometimes unreliable.

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

