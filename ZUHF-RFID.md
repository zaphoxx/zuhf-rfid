# ZUHF-RFID

## Name: ZUHF-RFID

##Description: Arduino Sketch to run a self build UHF RFID Reader (Read/Write);
This sketch is specifically coded for the Arduino Due. It most likely will not 
work with other Arduino boards.

## Author:       Manfred Heinz

## Last Update:  06.03.2021

Copyright (C) 2021  Manfred Heinz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
## Files:

- in folder ZUHF-RFID-DEV/ 
    - ZUHF-RFID-DEV.ino
    - Main sketch file(s)

- in folder library/ZUHF_CC1101/
    - library to control the RX-Module (CC1101 Chip)
    - library with crc5 / crc16 functions needed
    - definitions for the CC1101 registers
    - Contains also files which define global vars

- in folder library/SPI_UART_CC1101/
    - library to control the TX-Module (CC1101 Chip)
- in folder cli/
    - zuhf-cli.py - python commandline interface

## Installation
- Copy the library folders ```./ZUHF_CC1101/``` and ```./SPI_UART_CC1101/``` into your Arduino library folder ```<ArduinoPath>/library/```. 
- Copy the folder ```./ZUHF-RFID-DEV/``` into your Arduino Projects/Sketch folder.
- Open the Arduino IDE, open the ZUHF-RFID.ino sketch.
- Select your Arduino Due board in the 'Tools' section and select the port your board is connected to.
- Upload the sketch to the board.
- Once this is complete you can close the Arduino IDE.
- Now you can use the CLI tool ```zuhf-cli.py``` as described below. Make sure you note which port your board is connected to.

## zuhf-cli.py

### usage (zuhf-cli.py needs to be run as privileged user otherwise you wont be able to connect with the port)

```bash

    └─$ sudo python3 zuhf-cli.py -h                                                                                                                                              

     @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
          @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
        @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
      !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
     :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


   usage: zuhf-cli.py [-h] [-p PORT] [-b BAUDRATE] [-tp {17,18,19,20,21,22}] [-t TIMEOUT] [-r REPETITIONS] [-block BLOCK_ADDR] [-mem {0,1,2,3}] [-n N_WORDS] [-data DATA] [-write]
                      [-lock]

   optional arguments:
     -h, --help            show this help message and exit
     -p PORT               serial port e.g. COM7,COM14
     -b BAUDRATE           baudrate of serial
     -tp {17,18,19,20,21,22}
                           tx_power - index
     -t TIMEOUT            serial read timeout
     -r REPETITIONS        repetitions to run
     -block BLOCK_ADDR     blockaddress to read/write from
     -mem {0,1,2,3}        memory block to read / write from e.g. user block
     -n N_WORDS            read n words from mem block, max is 32 words
     -data DATA            word data to write to mem block
     -write
     -lock
```
#### Selecting the right port
To find the right port you can use the following command
```
└─$ sudo dmesg | grep -i '\(arduino\|tty\)'
[    0.610773] printk: console [tty0] enabled
[    3.391005] 00:05: ttyS0 at I/O 0x3f8 (irq = 4, base_baud = 115200) is a 16550A
[    3.402548] 00:06: ttyS1 at I/O 0x2f8 (irq = 3, base_baud = 115200) is a 16550A
[    6.541678] systemd[1]: Created slice system-getty.slice.
[33709.456826] usb 2-2.2: Product: Arduino Due Prog. Port
[33709.456827] usb 2-2.2: Manufacturer: Arduino (www.arduino.cc)
[33709.498375] cdc_acm 2-2.2:1.0: ttyACM0: USB ACM device
```
In the example above you can see that my Arduino Due is connected at ```/dev/ttyACM0```. 

### Running the reader with zuhf-cli.py
So to run a simple command you can then do
```
└─$ sudo python3 zuhf-cli.py -p /dev/ttyACM0

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-----------------------------------
port      :         /dev/ttyACM0
baudrate  :               250000
tx_power  :                   22
timeout   :                    3
repetitions:                 1000
block_addr:                    0
mem_block :                    3
n_words   :                    1
data      :                 1337
write_flag:                False
lock_flag :                False
-----------------------------------
<press 'return' to start>

b'REP#'
b'TXP#'
b'READ#'
b'DUMMY#'                                                                       
b'RUN#'
[TAG-DATA]/HANDLE] OK                                                           
##############################################################
Stored PC: 0x34 0x0
```
So running the cli without other arguments will repeat 1000 times to try to read the first word from user memory (memory block 3). This would be equivalent to the following command
```
sudo python3 zuhf-cli.py -p /dev/ttyACM0 -n 1 -mem 3
```
where parameter ```-n 2``` defines the number of words to read and ```-mem 3``` defines from which memory block the reader should pull the information. You can also define from which block address you want to start reading by using the parameter ```-block <addr>``` where address is 0th,1st,2nd,3rd,4th,... word of memblock.
e.g.
```
sudo python3 zuhf-cli.py -p /dev/ttyACM0 -n 1 -mem 3 -block 4
```
will try to read 2 words from memory block 3 starting at the 4th word of the memory block.

!ATTENTION! From my experience it is safe to read up to 8 words at once. Trying to read more than that might cause inconsistent results and is not recommended. It is safer to perform multiple reads with a smaller amount of words.

#### writing data to the tag/card
You can write data to a memory block (if not locked) using the following command
```
└─$ sudo python3 zuhf-cli.py -p /dev/ttyACM0 -mem 3 -n 2 -data 'deadbeef' -write 

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[WARNING]You are about to potentially overwrite data!!!
<continue y/n ?>y
[info] Using following settings:
-----------------------------------
port      :         /dev/ttyACM0
baudrate  :               250000
tx_power  :                   22
timeout   :                    3
repetitions:                 1000
block_addr:                    0
mem_block :                    3
n_words   :                    2
data      :             deadbeef
write_flag:                 True
lock_flag :                False
-----------------------------------
<press 'return' to start>

b'\xde\xad\xbe\xef'
b'#'
b'#'
b'#'
b'#'
b'#'
b'#'
b'#'
b'#'
b'REP#'
b'TXP#'
b'WRITE#'
b'#WRITE1#'                                                                     
b'#WRITE MODE#'                                                                 
b'DUMMY#'
b'RUN#'
[TAG-DATA]/HANDLE] OK                                                           
##############################################################
Stored PC: 0x34 0x0
EPC: 0xe2 0x0 0x0 0x1b 0x95 0x17 0x1 0x84 0x12 0x10 0xbe 0xef
CRC16: 0x74 0x4b
##############################################################
# WRITE # deadbeef written to MEMBLOCK 3 ADDR 0 #                               
# WRITE # deadbeef written to MEMBLOCK 3 ADDR 0 #                               
b'read nWords: #'
b'2#'
[MEM BLOCK 3]
###############
0x000000: de ad
0x000001: be ef
###############
```
So to write data you need to provide the ```-write``` parameter and the ```-data <data>``` parameter with the data to write. The data needs to be in hex format MSB first and the number of bytes provided needs to be a multiple of 2 (n words). If the write is successful it will try to read n words as specified by parameter ```-n``` from the specified memory location so you can verify the write.

