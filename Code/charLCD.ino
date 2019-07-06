//UI on character LCD
//Includes a main menu
//Displays on LCD list of songs saved on SD
//Scroll through songs using rotary encoder C

#include "Adafruit_HT1632.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Tone.h>
#include <limits.h>
#include <SPI.h>
#include <SdFat.h>
#include <LiquidCrystal.h>
SdFat SD;

//define pins for LED matrix
#define HT_CS   22
#define HT_DATA 24
#define HT_WR   26

Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS);
Adafruit_7segment sseg1 = Adafruit_7segment();
Adafruit_7segment sseg2 = Adafruit_7segment();
Adafruit_7segment sseg3 = Adafruit_7segment();

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const byte rs = 39, en = 41, d4 = 43, d5 = 45, d6 = 47, d7 = 49;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//SD constants
#define CS_PIN 53
#define CSV_DELIM ','
File file;

//define musical pins
#define pz1Pin 62
#define pz2Pin 63
#define pz3Pin 64
#define pz4Pin 65
#define pz5Pin 66
#define drumPin 67

Tone tone1;
Tone tone2;
Tone tone3;
Tone tone4;
Tone tone5;

//define switch pins
#define testPin 34
#define ndPin 33
#define playPin 37
#define stopPin 35
#define tempoPin 36
#define modePin 31


//define pins for rotary encoder C
#define SWCpin 13
#define RECpinA 12
#define RECpinB 11

//ints associated with the rotary encoder C
int Ccount = -1;
int ClastCount = Ccount;
int cState;
int cLast;

String songFile = "OIMID.TXT";

byte REST = 0;
byte SUSTAIN = 1;
byte currentOctave = 3;
byte noteDur = 1;
int LEDtempo;
int tempo;
int totalBeats;
byte lastFile = 0;
int saveBuffer0[48][6][2];
int saveBuffer1[48][6][2];
int compBuffer[12][6][2] = {{{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,1}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,2}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,3}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,4}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,5}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,6}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,7}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,8}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,9}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,10}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,11}},
                            {{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,12}}};
                            

int octave[7][12] = {{NOTE_C1, NOTE_CS1, NOTE_D1, NOTE_DS1, NOTE_E1, NOTE_F1, NOTE_FS1, NOTE_G1, NOTE_GS1, NOTE_A1, NOTE_AS1, NOTE_B1}, 
                     {NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2},
                     {NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3},
                     {NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4},
                     {NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5},
                     {NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6},
                     {NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7}};

char LCDlist[2][20];
byte LCDmode = 0;
byte songCount;


void setup() {
  Serial.begin(9600);
  pinMode(SWCpin, INPUT_PULLUP);
  pinMode(RECpinA, INPUT);
  pinMode(RECpinB, INPUT);
  lcd.begin(16, 2);

  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  songCount = fileNum();
}

void loop(){
  if (LCDmode == 0){
    LCDmainMenu();
  }
  if (LCDmode == 1){
    LCDload();
  }
}


void LCDmainMenu(){
  //check if rotary encoder C has moved
  cState = digitalRead(RECpinA);
  if((cLast == LOW) && (cState == HIGH)){
    if(digitalRead(RECpinB) == LOW && Ccount > 0){
      Ccount--;
    }
    else if (digitalRead(RECpinB) == HIGH && Ccount < 1){
      Ccount++;
    }
  }
  cLast = cState;
  if (Ccount != ClastCount){
    ClastCount = Ccount;
    lcd.clear();
    if (Ccount == 0){
      lcd.setCursor(0,0);
      lcd.print(">LOAD");
      lcd.setCursor(1,1);
      lcd.print("NEW SONG");
    }
    if (Ccount == 1){
      lcd.setCursor(1,0);
      lcd.print("LOAD");
      lcd.setCursor(0,1);
      lcd.print(">NEW SONG");
    }
  }
  if (digitalRead(SWCpin) == LOW){
    if (Ccount == 0){
      LCDmode = 1;
      redrawLoad();
    }
    if (Ccount == 1){
      LCDmode = 2;
    }
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
}

void LCDload() {
  //check if rotary encoder C has moved
  cState = digitalRead(RECpinA);
  if((cLast == LOW) && (cState == HIGH)){
    if(digitalRead(RECpinB) == LOW && Ccount > -1){
      Ccount--;
    }
    else if (digitalRead(RECpinB) == HIGH && Ccount < (songCount - 1)){
      Ccount++;
    }
  }
  cLast = cState;
  if (Ccount != ClastCount){
    ClastCount = Ccount;
    redrawLoad();
  }
  if (digitalRead(SWCpin) == LOW){
    if (Ccount == -1){
      LCDmode = 0;
      Ccount = 0;
    }
    else{
      Serial.println(char2string(LCDlist[0]));
    }
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
}


//Converts char array[20] to string (excluding the file extension)
String char2string(char fileChar[20]){
  String fileStr = "";
  for (byte x = 0; x < 20; x++){
    if (String(fileChar[x]) != "."){
      fileStr = fileStr + fileChar[x];
    }
    else if (String(fileChar[x]) == "."){
      return fileStr;
    }
  }
}


//redraws character LCD with the approporiate 2 file names--------------------------------------------------------------------------------------------------
void redrawLoad(){
  char fnArray[20];
  byte n = 0;
  SD.chdir();
  while (file.openNext(SD.vwd(), O_RDONLY)) {
    if (!file.isHidden() && !file.isDir()) {
      file.getName(fnArray, 20);
      if (n == Ccount){
        for (byte x = 0; x < 20; x++){
          LCDlist[0][x] = fnArray[x];
        }
      }
      if (n == Ccount + 1){
        for (byte x = 0; x < 20; x++){
          LCDlist[1][x] = fnArray[x];
        }
      }
      n++;
    }
    file.close();
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(">");
  if (Ccount == -1){
    lcd.setCursor(1,0);
    lcd.print("BACK");
    lcd.setCursor(1,1);
    lcd.print(LCDlist[1]);
  }
  else if (Ccount < (songCount - 1)){
    for (byte y = 0; y < 2; y++){
      lcd.setCursor(1,y);
      lcd.print(LCDlist[y]);
    }
  }
  else if (Ccount == (songCount - 1)){
    lcd.setCursor(1,0);
    lcd.print(LCDlist[0]);
  }
}


//returns the number of songs saved on the SD card-----------------------------------------------------------------------------------------------
byte fileNum(){
  byte fileCount = 0;
  SD.chdir();
  while (file.openNext(SD.vwd(), O_READ)) {
    if (!file.isHidden() && !file.isDir()) {
      fileCount++;
    }
    file.close();
  }
  return fileCount;
}
