#ifndef ZUHF_MENU_H
#define ZUHF_MENU_H

#include <TS_BUTTON.h>
#include <Adafruit_GFX.h>    
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>
#include <ZUHF_VARS.h>

MCUFRIEND_kbv tft; 

#define LED_PIN1 40
#define LED_PIN2 38

#define LCD_CS A3 
#define LCD_CD A2 
#define LCD_WR A1 
#define LCD_RD A6 
#define LCD_RESET A4 

#define TS_MINX 100
#define TS_MINY 215 //200
#define TS_MAXX 907 // 875
#define TS_MAXY 862 // 863

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

#define BLACK   0x0000
#define BLUE    0x001F
#define RED       0xF800
#define LIGHTBLUE 0x07FF
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF

#define MAX_LINES 256

enum MENU_STATES
{
  M_MAIN,
  M_IMAIN,
  M_CONFIG,
  M_ICONFIG,
  M_RESULT,
  M_WAIT
};

extern int repetitions;
extern byte packet_delay;
extern byte paPower;
extern byte agc1;
extern byte agc2;
extern READER_STATES reader_state;
extern byte sync1,sync0;

MENU_STATES menuState = M_MAIN;
//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

boolean lightOn = false;
uint16_t bcolor = RED;
String resultBuffer[MAX_LINES];
uint16_t resultLines = 0;

/* Main Menu Buttons */
TS_Button configButton;// = TS_Button(&tft,&ts,0,60,480,40,RED,WHITE,WHITE,"Configurations",2,30,0);
TS_Button runButton;// = TS_Button(&tft,&ts,0,100,480,40,RED,WHITE,WHITE,"Start Run",2,30,0);
TS_Button syncSwitch;
 
/* Config Menu Buttons */
TS_Button backButton;
const uint8_t nPowerButtons = 5;
TS_Button powerButtons[nPowerButtons];
const char *powerTexts[nPowerButtons] = {"-9.8dBm", "-5.0dBm", "-0.3dBm", "+5.2dBm", "+10.7dBm"};
const char powerVals[nPowerButtons] = {0x12,0x13,0x13,0x14,0x15};
char paVal = powerVals[0];

const uint8_t nAgc1Buttons = 3;
TS_Button agc1Buttons[nAgc1Buttons];
const char *agc1Texts[nAgc1Buttons] = {"0x90","0x91","0x92"};
const char agc1Vals[nAgc1Buttons] = {0x90,0x91,0x92};

const uint8_t nAgc2Buttons = 8;
TS_Button agc2Buttons[nAgc2Buttons];
const char *agc2Texts[nAgc2Buttons] = {"0","1","2","3","4","5","6","7"};
const char agc2Vals[nAgc2Buttons] = {0,1,2,3,4,5,6,7};

const uint8_t nRepButtons = 4;
TS_Button repButtons[nRepButtons];
const char *repTexts[nRepButtons] = {"1","10","100","inf"};
const char repVals[nRepButtons] = {1,10,100,0};

const uint8_t nDelayButtons = 4;
TS_Button delayButtons[nDelayButtons];
const char *delayTexts[nDelayButtons] = {"0us","20us","500us","1s"};
const uint16_t delayVals[nDelayButtons] = {0,20,500,1000};





void InitConfigMenu()
{
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.fillScreen(BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);

  tft.println("ConfigurtionMenu");
  tft.println("PowerSetting");
  for (int i = 0; i < nPowerButtons; i++)
  {
    powerButtons[i].Draw(powerButtons[i].selected);
  }

  tft.setCursor(0,74);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Automatic Gain Control 0");
  for (int i = 0; i < nAgc1Buttons; i++){
    agc1Buttons[i].Draw(agc1Buttons[i].selected);
  }
  
  tft.setCursor(0,124);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Automatic Gain Control 2");
  for (int i = 0; i < nAgc2Buttons; i++){
    agc2Buttons[i].Draw(agc2Buttons[i].selected);
  }
  tft.setCursor(0,174);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Repetitions");
  for (int i = 0; i < nRepButtons; i++){
    repButtons[i].Draw(repButtons[i].selected);
  }
  tft.setCursor(0,224);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Delay between Runs");
  for (int i = 0; i < nDelayButtons; i++){
    delayButtons[i].Draw(delayButtons[i].selected);
  }
  backButton.Draw(false);
}

void RunConfigMenu()
{
    
  for (int i = 0; i < nPowerButtons; i++){
    if (powerButtons[i].Pressed()){
      paPower = powerVals[i];
      if(Serial){
        Serial.print("output Power: ");Serial.println(powerTexts[i]);
      }
    }
  }
  for (int i = 0; i < nAgc1Buttons; i++){
    if (agc1Buttons[i].Pressed()){
      agc1 = agc1Vals[i];
      if(Serial){
        Serial.print("AGC0: ");Serial.println(agc1Texts[i]);
      }
    }
    
  }
  for (int i = 0; i < nAgc2Buttons; i++){
    if (agc2Buttons[i].Pressed()){
      agc2 = agc2Vals[i];
      if(Serial){
        Serial.print("AGC2: ");Serial.println(agc2Texts[i]);
      }
    }
    
  }
  for (int i = 0; i < nRepButtons; i++){
    if (repButtons[i].Pressed()){
      repetitions = repVals[i];
      if(Serial){
        Serial.print("n Repetitions: ");Serial.println(repTexts[i]);
      }
    }
    
  }
  for (int i = 0; i < nDelayButtons; i++){
    if (delayButtons[i].Pressed()){
      packet_delay = delayVals[i];
      if(Serial){
        Serial.print("delay: ");Serial.println(delayTexts[i]);
      }
    }
  }
  if (backButton.Pressed()){
    menuState = M_IMAIN;
  }
  delay(200);
}

void InitMainMenu()
{
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.fillScreen(BLACK);
  tft.setCursor(480 / 2 - 140,10);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.println("*** ZUHF-RFID ***");
  configButton.Draw(false);
  runButton.Draw(false);
  syncSwitch.Draw(false);
}

READER_STATES RunMainMenu()
{
  READER_STATES rstate = R_WAIT;
  if (configButton.Pressed()){
    lightOn = !lightOn;
    digitalWrite(LED_PIN1, lightOn);
    digitalWrite(LED_PIN2, HIGH);
    menuState = M_ICONFIG;
  }
  if (runButton.Pressed()){
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    rstate = R_START;
  }
  if (syncSwitch.Pressed()){
    sync1 = ~sync1;
    sync0 = ~sync0;
    char *syncText;
    if (sync1 == 0xad){
      syncText = "Syncword 0xAD23";
      syncSwitch.text = syncText;
      syncSwitch.Draw(false);
    }else{
      syncText = "Inv Syncword ";
      syncSwitch.text = syncText;
      syncSwitch.Draw(true);
    }
    
  }
  delay(200);
  return rstate;
}


void CreateButtons(uint8_t power, uint8_t agc1, uint8_t agc2, uint8_t repetitions, uint16_t delay)
{  

  configButton = TS_Button(&tft,&ts,0,60,480,40,RED,WHITE,WHITE,"Configurations",2,30,0);
  runButton = TS_Button(&tft,&ts,0,100,480,40,RED,WHITE,WHITE,"Start Run",2,30,0);
  syncSwitch = TS_Button(&tft,&ts,0,140,480,40,RED,WHITE,WHITE,"Invert SyncWord",2,30,0);
  
  for (int i = 0; i < nPowerButtons; i++)
  {
    powerButtons[i] = TS_Button(&tft,&ts,(i*(480 / nPowerButtons)),40,480 / nPowerButtons,30,WHITE,BLACK,BLACK,powerTexts[i],2,0,0);
    powerButtons[i].cursor_x_offset = 10;
    if (powerVals[i] == power){
        powerButtons[i].selected = true;
        //powerButtons[i].Draw(true);
    }
  }

  tft.setCursor(0,74);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Automatic Gain Control 0");
  for (int i = 0; i < nAgc1Buttons; i++){
    agc1Buttons[i] = TS_Button(&tft,&ts,(i*(480 / nAgc1Buttons)),90,480 / nAgc1Buttons,30,WHITE,BLACK,BLACK,agc1Texts[i],2,10,0);
    agc1Buttons[i].cursor_x_offset = 40;
    if (agc1Vals[i] == agc1){
        agc1Buttons[i].selected = true;
        //agc1Buttons[i].Draw(true);
    }
  }
  
  tft.setCursor(0,124);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Automatic Gain Control 2");
  for (int i = 0; i < nAgc2Buttons; i++){
    agc2Buttons[i] = TS_Button(&tft,&ts,(i*(480 / nAgc2Buttons)),140,480 / nAgc2Buttons,30,WHITE,BLACK,BLACK,agc2Texts[i],2,10,0);
    agc2Buttons[i].cursor_x_offset = 20;
    if (agc2Vals[i] == agc2){
        agc2Buttons[i].selected = true;
        //agc2Buttons[i].Draw(true);
    }
  }
  tft.setCursor(0,174);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Repetitions");
  for (int i = 0; i < nRepButtons; i++){
    repButtons[i] = TS_Button(&tft,&ts,(i*(480 / nRepButtons)),190,480 / nRepButtons,30,WHITE,BLACK,BLACK,repTexts[i],2,50,0);
    repButtons[i].cursor_x_offset = 20;
    if (repVals[i] == repetitions){
        repButtons[i].selected = true;
        //repButtons[i].Draw(true);
    }
  }
  tft.setCursor(0,224);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Delay between Runs");
  for (int i = 0; i < nDelayButtons; i++){
    delayButtons[i] = TS_Button(&tft,&ts,(i*(480 / nDelayButtons)),240,480 / nDelayButtons,30,WHITE,BLACK,BLACK,delayTexts[i],2,30,0);
    delayButtons[i].cursor_x_offset = 20;
    if (delayVals[i] == delay){
        delayButtons[i].selected = true;
        //delayButtons[i].Draw(true);
    }
  }
  backButton = TS_Button(&tft,&ts,0,280,480,40,RED,WHITE,WHITE,"BACK",2,480 / 2 + 30,0);
}


void InitMenu()
{
  pinMode(LED_PIN1,OUTPUT);
  pinMode(LED_PIN2,OUTPUT);
  digitalWrite(LED_PIN2,HIGH);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  tft.reset();
  tft.begin(0x9486);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  CreateButtons(0x80,0x90,0x03,0x00,500);
  menuState = M_IMAIN;
  //InitConfigMenu(0x80,0x90,0x03,0x00,500);
}

void PrintResult(String msg, bool reset_buffer)
{
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //tft.reset();
  //tft.begin(0x9486);
  //tft.setRotation(1);
  if (reset_buffer){
    for (int i = 0; i < MAX_LINES; i++){
      resultBuffer[i] = "";
    }
    resultLines = 0;
  }
  if (resultLines < MAX_LINES){
    resultBuffer[resultLines] = msg;
    resultLines++;
    
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(0,0);
    int istart;
    if (resultLines < 20) {
      istart = 0;
    }else{
      istart = resultLines - 20;
    }
    for (int i = istart; i < resultLines; i++){
      tft.println(resultBuffer[i]);
    }
  }
}



void RunMenu(){
  switch(menuState){
      case M_IMAIN:
        InitMainMenu();
        menuState = M_MAIN;
        break;
        
      case M_MAIN:
        reader_state = RunMainMenu();
        break;
        
      case M_ICONFIG:
        InitConfigMenu();
        menuState = M_CONFIG;
        break;
        
      case M_CONFIG:
        RunConfigMenu();
        break;
      
      case M_WAIT:
        break;
      case M_RESULT:
        break;
        
      default:
        menuState = M_IMAIN;
  }
}

#endif