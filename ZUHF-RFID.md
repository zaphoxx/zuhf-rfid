Name: ZUHF-RFID
Description: Arduino Sketch to run a self build UHF RFID Reader (Read/Write);
This sketch is specifically coded for the Arduino Due. It most likely will not 
work with other Arduino boards. 
Author:       Manfred Heinz
Last Update:  06.03.2021
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
 
Files:

- ZUHF-RFID-DEV_06032021.7z
    - Main sketch file(s)

- ZUHF_CC1101.zip 
    - library to control the RX-Module (CC1101 Chip)
    - library with crc5 / crc16 functions needed
    - definitions for the CC1101 registers
    - Contains also files which define global vars

- SPI_UART_CC1101.zip
    - library to control the TX-Module (CC1101 Chip)
