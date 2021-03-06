#ifndef CONSOLEMENU_H
#define CONSOLEMENU_H

#include <ZUHF_BUFFER.h>
#include <ZUHF_VARS.h>

enum M_STATES{
  M_MAIN,
  M_CONFIG,
  M_RESULT,
  M_EXIT
};

extern M_STATES current_Menu;
extern void logo();
extern byte tx_power;
extern uint32_t repetitions;
extern byte agc2;
extern byte agc0;
extern uint32_t packet_delay;

extern byte tag_settle;
extern byte cw1;
extern byte cw2;
extern byte sync1,sync0;

extern TAG_INFO tags_read[1000];
extern int tags_found;


bool cmd_received = 0;

void runMenu();
char getInput();
void printMain();
void runMain();
void clearScreen();

void runMenu() {
  switch(current_Menu){
    case(M_MAIN):
      printMain();
      runMain();
      break;
    case(M_EXIT):
      break;
    default:
      printMain();
      runMain();
      break;
  }
}

char getInput(){
  char rx_byte;
  if (Serial.available()){
    rx_byte = Serial.read();
    Serial.println(rx_byte);
    cmd_received = 1;
    return rx_byte;
  } 
}

void printSummary(){
  clearScreen();
  int unique_tags_found = 0;
  byte epc[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
  Serial.println("\t*********************** EPC *************************");
  //Serial.println(n_tag_responses,DEC);
  
  for (int i = 0; i < tags_found; i++){
    if (epc[11] != tags_read[i].EPC_Data[11] and tags_read[i].CRC_OK){
      memcpy(epc, tags_read[i].EPC_Data,12);
	    Serial.print("\tEPC: ");
      for (int j = 0; j < 12; j++){
        Serial.print(epc[j],HEX);Serial.print(" ");
        //Serial.print(TAGS_READ[i].EPC_Data[j],HEX);Serial.print(" ");
      }
	    Serial.print("\tCRC16 OK: ");
	    Serial.print(tags_read[i].CRC_OK,DEC);
      Serial.println();
		
		  byte L 		= ( tags_read[i].StoredPC & L_MASK ) >> 11;
		  bool UMI 	= ( tags_read[i].StoredPC & UMI_MASK ) >> 10;
		  bool XI 	= ( tags_read[i].StoredPC & XI_MASK ) >> 9;
		  bool T		= ( tags_read[i].StoredPC & T_MASK ) >> 8;
		
		  Serial.print("\tEPC Length: ");Serial.print(L,DEC); Serial.print(" Words ("); Serial.print(L * 16);Serial.println(" Bits)"); 
		  Serial.print("\tUMI: ");Serial.print(UMI);Serial.print("\tXI: ");Serial.print(XI);Serial.print("\tT: ");Serial.println(T);
	  }
  }
  Serial.println("\t*********************** *** *************************");
  Serial.print("\t[ ");Serial.print(tags_found);Serial.print(" / ");Serial.print(repetitions); Serial.println(" ]");
  Serial.print("\tRate: ");Serial.println(( (float) tags_found / (float) repetitions) * 100);
  Serial.println("\t********************* *** ***********************");
  
  char rx_byte;
  while(rx_byte != 'x' and rx_byte != 'X'){
    rx_byte = getInput();
	delay(100);
  }
}

void printMain(){
  clearScreen();
  Serial.println("********* MainMenu ********");
  Serial.println("  (c)onfiguration Menu");
  Serial.println("  (l)ast Run");
  Serial.println("  (s)tart Run");
  Serial.println("***************************");
}


void printConfig(){
  clearScreen();
  // 0x11 --> 0x11/0x80 (5.2 dBm) 0x12/0x27 (-9.8 dBm) 0x13/0x67 (-5.0 dBm) 0x14/0x50 (-0.3 dBm) 0x16/0xc0 (9.8 dBm) byte PaTable[8] = {0x00,0x80,0x27,0x67,0x50,0x80,0xc0,0x00}; //
  Serial.println("********** Configuration ***********");
  Serial.println("\tTX Power");
  if (tx_power == 0x12) Serial.print(" *");
  Serial.println("\t(1) -9.8 dBm");
  if (tx_power == 0x13) Serial.print(" *");
  Serial.println("\t(2) -5.0 dBm");
  if (tx_power == 0x14) Serial.print(" *");
  Serial.println("\t(3) -0.3 dBm");
  if (tx_power == 0x11 or tx_power == 0x15) Serial.print(" *");
  Serial.println("\t(4)  5.2 dBm");
  if (tx_power == 0x16) Serial.print(" *");
  Serial.println("\t(5)  9.8 dBm");
  Serial.println("************************************");
  Serial.println("\tRepetitions");
  if (repetitions == 1) Serial.print(" *");
  Serial.println("\t(6) 1 ");
  if (repetitions == 200) Serial.print(" *");
  Serial.println("\t(7) 100");
  if (repetitions == 500) Serial.print(" *"); 
  Serial.println("\t(8) 500");
  if (repetitions == 1000) Serial.print(" *"); 
  Serial.println("\t(9) 1000");
  Serial.println("************************************");
  Serial.println("\t(a) AGC Control");
  Serial.println("\t(d) Delay Control");
  Serial.println("\t(c) CW Control");
  Serial.println("************************************");
}


void printAGC()
{
  clearScreen();
  Serial.println("********** AGC Control ***********");
  Serial.println("\tAGC0");
  if (agc0 == 0x90) Serial.print(" *");
  Serial.println("\t(0) 0x90");
  if (agc0 == 0x91) Serial.print(" *");
  Serial.println("\t(1) 0x91");
  if (agc0 == 0x92) Serial.print(" *");
  Serial.println("\t(2) 0x92");
  Serial.println("************************************");
  Serial.println("\tAGC2");
  if (agc2 == 0x03) Serial.print(" *");
  Serial.println("\t(3)  0x03");
  if (agc2 == 0x04) Serial.print(" *");
  Serial.println("\t(4)  0x04");
  if (agc2 == 0x05) Serial.print(" *");
  Serial.println("\t(5)  0x05");
  if (agc2 == 0x06) Serial.print(" *");
  Serial.println("\t(6)  0x06");
  if (agc2 == 0x07) Serial.print(" *");
  Serial.println("\t(7)  0x07");
  Serial.println("************************************");
}


void runAGC(){
  char rx_byte;
  printAGC();
  while(rx_byte != 'x' and rx_byte != 'X'){
    cmd_received = 0; 
    rx_byte = getInput();
    switch(rx_byte){
      /* AGC CONTROLS */
      case('0'):
        clearScreen();
        agc0 = 0x90;
        printAGC();
        break;
      case('1'):
        clearScreen();
        agc0 = 0x91;
        printAGC();
        break;
      case('2'):
        clearScreen();
        agc0 = 0x92;
        printAGC();
        break;
      case('3'):
        clearScreen();
        agc2 = 0x03;
        printAGC();
        break;
      case('4'):
        clearScreen();
        agc2 = 0x04;
        printAGC();
        break;
      case('5'):
        clearScreen();
        agc2 = 0x05;
        printAGC();
        break;
      case('6'):
        clearScreen();
        agc2 = 0x06;
        printAGC();
        break;
      case('7'):
        clearScreen();
        agc2 = 0x07;
        printAGC();
        break;
      default:
        if (cmd_received){
          printAGC();
        }
        break;
    }
   }
}


void runConfig(){
  char rx_byte;
  printConfig();
  while(rx_byte != 'x' and rx_byte != 'X'){
    cmd_received = 0; 
    rx_byte = getInput();
    switch(rx_byte){
      /* TX POWER OPTIONS */
      case('1'):
        clearScreen();
        tx_power = 0x12;
        printConfig();
        break;
      case('2'):
        clearScreen();
        tx_power = 0x13;
        printConfig();
        break;
      case('3'):
        clearScreen();
        tx_power = 0x14;
        printConfig();
        break;
      case('4'):
        clearScreen();
        tx_power = 0x15;
        printConfig();
        break;
      case('5'):
        clearScreen();
        tx_power = 0x16;
        printConfig();
        break;
      /* *************** */
      case('6'):
        clearScreen();
        repetitions = 1;
        printConfig();
        break;
      case('7'):
        clearScreen();
        repetitions = 200;
        printConfig();
        break;
      case('8'):
        clearScreen();
        repetitions = 500;
        printConfig();
        break;
      case('9'):
        clearScreen();
        repetitions = 1000;
        printConfig();
        break;
      case('a'):
        clearScreen();
        //printAGC();
        runAGC();
		printConfig();
        break;
      default:
        if (cmd_received){
          printConfig();
        }
        break;
    }
   }
}


void runMain(){  
   char rx_byte;
   printMain();
   while(rx_byte != 'S' and rx_byte != 's'){
    cmd_received = 0; 
    rx_byte = getInput();
    switch(rx_byte){
      case('C'):
      case('c'):
        Serial.println("TODO: Switch to Configuration Menu");
        runConfig();
        printMain();
        break;
      case('L'):
      case('l'):
        printSummary();
        printMain();
        break;
      case('#'):
        clearScreen();
        logo();
        delay(1000);
        printMain();
        break;
      default:
        if (cmd_received){
          printMain();
        }
        break;
    }
    delay(10);
   }
}


void clearScreen(){
  if (Serial) {
    Serial.write(27);
    Serial.print("[2J");
  }
}

#endif
