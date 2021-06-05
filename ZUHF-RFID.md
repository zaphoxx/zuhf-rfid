# ZUHF-RFID

## Name: ZUHF-RFID

##Description: Arduino Sketch to run a self build UHF RFID Reader (Read/Write);
This sketch is specifically coded for the Arduino Due. It most likely will not 
work with other Arduino boards.

## Author:       Manfred Heinz

## Last Update:  05.05.2021

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

- in folder ```ZUHF-RFID/``` 
    - ```ZUHF-RFID.ino```
    - Main sketch file(s)

- in folder ```library/ZUHF_CC1101/```
    - library to control the RX-Module (CC1101 Chip)
    - library with crc5 / crc16 functions needed
    - definitions for the CC1101 registers
    - Contains also files which define global vars

- in folder ```library/SPI_UART_CC1101/```
    - library to control the TX-Module (CC1101 Chip)
- in folder ```cli/```
    - ```zuhf-cli.py``` - python commandline interface

## Installation
- import the zip files (libraries) with the Arduino Ide using ```Sketch-->Include Library-->Add .ZIP Library```, select the zip files SPI_UART_CC1101.zip and ZUHF_CC1101.zip for import.	
- Copy the folder ```./ZUHF-RFID/``` into your Arduino Projects/Sketch folder.
- Open the Arduino IDE, open the ZUHF-RFID.ino sketch.
- Select your Arduino Due board in the 'Tools' section and select the port your board is connected to.
- Upload the sketch to the board.
- Once this is complete you can close the Arduino IDE.
- Now you can use the CLI tool ```zuhf-cli.py``` as described below. Make sure you note which port your board is connected to.

## Hardware required
- Arduino Due
- 2 RF-Modules based on CC1101, where the RX-Module is connected to the Arduinos main SPI connectors and the TX-Module is connected to USART1 (TX2/RX2) (for more information please refer to the HOWTO_PART1);

## zuhf-cli.py
zuhf-cli.py is a simple python based command line interface to control the zuhf-rfid-reader. The cli requires python3 to be installed but other then that there are no other special libraries needed other then the standarts which are included with a fresh python3 installation. The cli communicates via serial communication with the reader. Make sure you check which usb port your reader is connected to. on windows this would be e.g. comXX whereas on linux that will be e.g. /dev/ttyACMx.
### usage (zuhf-cli.py needs to be run as privileged user otherwise you wont be able to connect with the serial port)

```bash

└─# ./zuhf-cli.py -h                                         

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


usage: zuhf-cli.py [-h] [-p PORT] [-b BAUDRATE] [-tp {17,18,19,20,21,22}] [-t TIMEOUT]
                   [-r REPETITIONS] [-block BLOCK_ADDR] [-mem {0,1,2,3}] [-n N_WORDS]
                   [-data DATA] [-mask LOCK_MASK] [-action LOCK_ACTION] [-access ACCESS_PWD]
                   [-write | -lock | -read | -monza]

optional arguments:
  -h, --help            show this help message and exit
  -p PORT               serial port e.g. COM7,COM14
  -b BAUDRATE           baudrate of serial
  -tp {17,18,19,20,21,22}
                        tx_power - index
  -t TIMEOUT            serial read timeout
  -r REPETITIONS        repetitions to run before terminating
  -block BLOCK_ADDR     blockaddress to read/write from
  -mem {0,1,2,3}        memory block to read / write from e.g. user block
  -n N_WORDS            read n words from mem block, max is 32 words
  -data DATA            word data to write to mem block
  -mask LOCK_MASK
  -action LOCK_ACTION
  -access ACCESS_PWD
  -write
  -lock
  -read
  -monza

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

#### Reading EPC information
So to run a simple command you can then do (make sure you see the ```Running ...``` statement before approaching the tag to the reader. Otherwise the reader wont be ready yet.)
```
─# ./zuhf-cli.py -p /dev/ttyACM0                            

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    3
n_words        :                    1
data           :                 1337
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                False
lock_flag      :                False
read_flag      :                False
monza          :                False
-------------------------------------
<press 'return' to start>

Running ...
[ACKNOWLEDGED]
[ACKNOWLEDGED]
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################
```
So running the cli without other arguments will repeat 1000 times to try to read a tags EPC information.

#### Reading selected memory areas
The following command will read 1 word from user memory starting at the 4th block (4th word) (numbering starts with 0 );
```
sudo python3 zuhf-cli.py -p /dev/ttyACM0 -n 1 -mem 3 -block 3
```
where parameter ```-n 1``` defines the number of words to read and ```-mem 3``` defines from which memory block the reader should pull the information. You can also define from which block address you want to start reading by using the parameter ```-block <addr>``` where address is 0th,1st,2nd,3rd,4th,... word of memblock.
Memory Blocks are usually defined as:
* 0 - Reserved Memory (Kill Password and Access Password locations)
* 1 - EPC Memory
* 2 - TID Memory
* 3 - User Memory

#### writing data to the tag/card
You can write data to a memory block (if not locked) using the following command
```
└─# ./zuhf-cli.py -p /dev/ttyACM0 -write -data deadbeef -mem 3 -block 0              

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


deadbeef 2
[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    3
n_words        :                    2
data           :             deadbeef
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                 True
lock_flag      :                False
read_flag      :                False
monza          :                False
-------------------------------------
<press 'return' to start>

b'\xde\xad\xbe\xef'
[+] WRITE MODE
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[WRITE] *
[REQ_RN] *
[REQ_RN(Handle)]
[+] WRITE COMPLETE
[REQ_RN] *
[REQ_RN(Handle)]
[+] WRITE COMPLETE
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################

```
So to write data you need to provide the ```-write``` parameter and the ```-data <data>``` parameter with the data to write. The data needs to be in hex format MSB first and the number of bytes provided needs to be a multiple of 2 (n words) (if you don't provide the -n parameter it will calculate n based on the provided data value). You can verify if the write was successful by trying to read the location after the write using

```bash
─# ./zuhf-cli.py -p /dev/ttyACM0 -read -data deadbeef -mem 3 -n 2

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    3
n_words        :                    2
data           :             deadbeef
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                False
lock_flag      :                False
read_flag      :                 True
monza          :                False
-------------------------------------
<press 'return' to start>

[+] Read Mode
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[MEM BLOCK 3]
###############
0x000000: de ad
0x000001: be ef
###############
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################

```

#### r/w Lock Memory Areas
You can also r/w lock or permalock memory areas. If you are experimenting with that feature and you want to be on the save side then make sure you do NOT lock the access password and you do not permalock any memory area (the later is not reversible). For details on which bits need to be set please refer to pg.89 in the official documentation https://www.gs1.org/sites/default/files/docs/epc/gs1-epc-gen2v2-uhf-airinterface_i21_r_2018-09-04.pdf.
For example if you want to r/w protect the 'Kill Password' you would need to send the following command. (This works if the access password is 0; once the lock is successful you still need to write an access password; See below on how to lock memory if access password is already set)

```bash
└─# ./zuhf-cli.py -p /dev/ttyACM0 -lock -mask 1000000000 -action 1000000000 

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    3
n_words        :                    1
data           :                 1337
lock_mask      :           1000000000
lock_action    :           1000000000
access_pwd     :             00000000
write_flag     :                False
lock_flag      :                 True
read_flag      :                False
monza          :                False
-------------------------------------
<press 'return' to start>

[+] LOCK MODE
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[LOCK] Successful
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################

```
So now access to the kill password is password protected. however you still need to set a password with the write command. once you done that the kill password is password protected.

writing an access password:

```bash
─# ./zuhf-cli.py -p /dev/ttyACM0 -write -data 12345678 -mem 0 -n 2 -block 2

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


12345678 2
[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    2
mem_block      :                    0
n_words        :                    2
data           :             12345678
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                 True
lock_flag      :                False
read_flag      :                False
monza          :                False
-------------------------------------
<press 'return' to start>

b'\x124Vx'
[+] WRITE MODE
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[WRITE] *
0
2
0
[REQ_RN] *
[REQ_RN(Handle)]
[+] WRITE COMPLETE
[REQ_RN] *
[REQ_RN(Handle)]
[+] WRITE COMPLETE
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################

```

Lets check if we can still read the 'Kill Password':

```bash
─# ./zuhf-cli.py -p /dev/ttyACM0 -read -mem 0 -n 4         

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    0
n_words        :                    4
data           :                 1337
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                False
lock_flag      :                False
read_flag      :                 True
monza          :                False
-------------------------------------
<press 'return' to start>

[+] Read Mode
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[READ] ERROR. Code: 4
[READ] Memory locked - The Tag memory location is locked or permalocked and is either not writeable1 or not readable.
[MEM BLOCK 0]
###############
0x000000: ff ff
0x000001: ff ff
0x000002: ff ff
0x000003: ff ff
###############
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################
```

Perfect, 'Kill Password' is locked now and not readable without the access password. (ATTENTION: For this example keep in mind that the 'Access Password' is not locked. Therefore the protection can be bypassed by simply setting the access password to 0).

So to read or write a 'Kill Password' you now need to provide the 'Access Password'.

#### Access Password Protected Memory Areas
Following on the previous example: To read the protected 'Kill Password' you can use the following command

```bash
└─# ./zuhf-cli.py -p /dev/ttyACM0 -access 12345678 -read -mem 0 -n 4

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    0
n_words        :                    4
data           :                 1337
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             12345678
write_flag     :                False
lock_flag      :                False
read_flag      :                 True
monza          :                False
-------------------------------------
<press 'return' to start>

1234 5678
12 34
56 78
[READACCESS]
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
SWITCH TO ACCESS
[ACCESS 1] OK
[SECURED]
[MEM BLOCK 0]
###############
0x000000: ca fe
0x000001: ba be
0x000002: 12 34
0x000003: 56 78
###############
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################

```
So to be able to access a password protected area you just add the ```-access <access password>``` to the action (read/write/lock) you want to perform.

So to unlock the 'Kill Password' and allow unrestricted access you would the perform the following command:


```bash
└─# ./zuhf-cli.py -p /dev/ttyACM0 -lock -mask 1000000000 -action 0000000000 -access 12345678

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    3
n_words        :                    1
data           :                 1337
lock_mask      :           1000000000
lock_action    :           0000000000
access_pwd     :             12345678
write_flag     :                False
lock_flag      :                 True
read_flag      :                False
monza          :                False
-------------------------------------
<press 'return' to start>

1234 5678
### LOCKACCESS ###
12 34
56 78
[LOCKACCESS]
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
SWITCH TO ACCESS
[ACCESS 1] OK
[SECURED]
[LOCK] Successful
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################
```

There you go! Now you should be able to read the 'Kill Password' without providing the access command. Let's try:
```bash
─# ./zuhf-cli.py -p /dev/ttyACM0 -read -mem 0 -n 4                       

 @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@
      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@
    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!
  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!
 :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : 


[info] Using following settings:
-------------------------------------
port           :         /dev/ttyACM0
baudrate       :               250000
tx_power       :                   22
timeout        :                    3
repetitions    :                 1000
block_addr     :                    0
mem_block      :                    0
n_words        :                    4
data           :                 1337
lock_mask      :           0000000000
lock_action    :           0000000000
access_pwd     :             00000000
write_flag     :                False
lock_flag      :                False
read_flag      :                 True
monza          :                False
-------------------------------------
<press 'return' to start>

[+] Read Mode
Running ...
[ACKNOWLEDGED]
[OPEN/SECURED]
[MEM BLOCK 0]
###############
0x000000: ca fe
0x000001: ba be
0x000002: 12 34
0x000003: 56 78
###############
[TAG-DATA]
##############################################################
Stored PC:    0x34 0x0
EPC:          300833B2DEADDEADBABABABA
CRC16:        0x65 0x1e
##############################################################
```
Perfect! So that's all there is for now. Have fun testing it!