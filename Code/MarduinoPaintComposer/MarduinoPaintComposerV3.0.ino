/*
~~Marduino Paint Composer v3.0~~
*composePlay() written to allow for limited playback (only to the end of saveBuffer1) during COMPOSE mode
  >Only audio currently working - need to incorporate LED matrix

¡STILL TO DO!
**Ability to start a new song
**Ability to make existing song longer
**Copy/Paste function???
*/

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

//define pins for rotary encoder A
#define SWApin 7
#define REApinA 6
#define REApinB 5
//define pins for rotary encoder B
#define SWBpin 10
#define REBpinA 9
#define REBpinB 8
//define pins for rotary encoder C
#define SWCpin 13
#define RECpinA 12
#define RECpinB 11

//ints associated with the rotary encoder A
int count = 1;
int lastCount = count;
int aState;
int aLast;
//ints associated with the rotary encoder B
int Bcount = 5;
int BlastCount = Bcount;
int bState;
int bLast;
//ints associated with the rotary encoder C
int Ccount = -1;
int ClastCount = Ccount;
int cState;
int cLast;

//used for debouncing dbPin in one scenario
unsigned long dbTime;

String songFile;

char LCDlist[2][20];
byte LCDmode = 0;
byte songCount;

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
int saveBufferX[48][6][2];
int matrixBuffer[12][6][2] = {{{REST,1},{REST,1},{REST,1},{REST,1},{REST,1},{0,1}},
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




void setup() {
  Serial.begin(9600);
  
  pinMode(ndPin, INPUT_PULLUP);
  pinMode(testPin, INPUT_PULLUP);
  pinMode(playPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  pinMode(tempoPin, INPUT_PULLUP);
  pinMode(modePin, INPUT_PULLUP);
  pinMode(SWApin, INPUT_PULLUP);
  pinMode(REApinA, INPUT);
  pinMode(REApinB, INPUT);
  pinMode(SWBpin, INPUT_PULLUP);
  pinMode(REBpinA, INPUT);
  pinMode(REBpinB, INPUT);
  pinMode(SWCpin, INPUT_PULLUP);
  pinMode(RECpinA, INPUT);
  pinMode(RECpinB, INPUT);
  pinMode(drumPin, OUTPUT);
  
  tone1.begin(pz1Pin);
  tone2.begin(pz2Pin);
  tone3.begin(pz3Pin);
  tone4.begin(pz4Pin);
  tone5.begin(pz5Pin);
  lcd.begin(16, 2);
  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  matrix.setRotation(0);
  matrix.clearScreen();
  sseg1.begin(0x70);
  sseg2.begin(0x71);
  sseg3.begin(0x72);
  sseg1.writeDisplay();
  sseg2.writeDisplay();
  sseg3.writeDisplay();
  aLast = digitalRead(REApinA);

  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  
  songCount = fileNum();
}


void loop(){
  //COMPOSE mode
  while (digitalRead(modePin) == HIGH){
    if (LCDmode == 0){
      LCDmainMenu();
    }
    if (LCDmode == 1){
      LCDloadList();
    }
    if (LCDmode == 2){
      composer();
    }
  }
  //PLAYBACK mode
  while (digitalRead(modePin) == LOW){
    LCDplayList();
  }
}


//composer-------------------------------------------------------------------------------------------------------------------------------------
void composer() {
  //check if rotary encoder A has moved
  aState = digitalRead(REApinA);
  if((aLast == LOW) && (aState == HIGH)){
    if(digitalRead(REApinB) == LOW){
      if(digitalRead(SWApin) == LOW){
        if(count > 10){
          count = count - 10;
        }
        else if(count <= 10){
          count = 1;
        }
      }
      else if(digitalRead(SWApin) == HIGH && count > -6){
        count--;
      }
    }
    else{
      if(digitalRead(SWApin) == LOW){
        if(count < (totalBeats - 21)){
          count = count + 10;
        }
        else if(count >= (totalBeats - 21)){
          count = (totalBeats - 11);
        }
      }
      else if(digitalRead(SWApin) == HIGH && count < (totalBeats - 11)){
        count++;
      }
    }
  }
  aLast = aState;

  //check if rotary encoder B has moved
  bState = digitalRead(REBpinA);
  if((bLast == LOW) && (bState == HIGH)){
    if(digitalRead(REBpinB) == LOW){
      if(digitalRead(SWApin) == LOW  && currentOctave < 6){
        currentOctave++;
        redrawMatrix(matrixBuffer);
        matrix.setPixel(15, Bcount);
        matrix.setPixel(0, Bcount);
        matrix.writeScreen();
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else if(digitalRead(SWApin) == HIGH){
        Bcount--;
      }
    }
    else{
      if(digitalRead(SWApin) == LOW  && currentOctave > 0){
        currentOctave--;
        redrawMatrix(matrixBuffer);
        matrix.setPixel(15, Bcount);
        matrix.setPixel(0, Bcount);
        matrix.writeScreen();
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else if(digitalRead(SWApin) == HIGH){
        Bcount++;
      }
    }
    if (Bcount == 1){
      if (currentOctave < 6){
        Bcount = 14;
        currentOctave++;
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else{
        Bcount = 2;
      }
    }
    if (Bcount == 15){
      if (currentOctave > 0){
        Bcount = 2;
        currentOctave--;
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else{
        Bcount = 14;
      }
    }
  }
  bLast = bState;

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
   
  //redraw LED matrix if rotary encoder A has moved
  if (count != lastCount){
    lastCount = count;
    reloadBuffer(count);
    redrawMatrix(matrixBuffer);
    matrix.setPixel(15, Bcount);
    matrix.setPixel(0, Bcount);
    matrix.writeScreen();
    sseg1.print(count + 7);
    sseg1.writeDisplay();
  }
  //move cursor up or down if rotary encoder B has moved
  if (Bcount != BlastCount){
    BlastCount = Bcount;
    redrawMatrix(matrixBuffer);
    matrix.setPixel(15, Bcount);
    matrix.setPixel(0, Bcount);
    matrix.writeScreen();
  }
  //move > between "SAVE" and "SAVE&QUIT" if rotary encoder C has moved
  if (Ccount != ClastCount){
    ClastCount = Ccount;
    if (Ccount == 0){
      lcd.setCursor(0,1);
      lcd.print(">SAVE  SAVE&QUIT");
    }
    if (Ccount == 1){
      lcd.setCursor(0,1);
      lcd.print(" SAVE >SAVE&QUIT");
    }
  }

  //plays song while button is held down
  if (digitalRead(playPin) == LOW){
    Serial.println("play");
    int n = count;
    byte sB = 0;
    if (n % 48 == 1){
      n = -1;
    }
    while (digitalRead(playPin) == LOW){
      Serial.println(n);
      if (n % 48 == 1){
        sB++;
      }
      composePlay(n, sB);
      if (n == -1){
        n = count + 1;
      }
      else{
        n++;
      }
    }
  }

  //if rotary encoder B has been pressed
  if (digitalRead(SWBpin) == LOW){
    placeNote();
    while (digitalRead(SWBpin) == LOW){
      ;
    }
  }

  //if rotary encoder C has been pressed
  if (digitalRead(SWCpin) == LOW){
    if (Ccount == 0){
      lcd.setCursor(0,1);
      lcd.print("save            ");
      save();
      lcd.setCursor(0,1);
      lcd.print(">SAVE  SAVE&QUIT");
    }
    if (Ccount == 1){
      lcd.setCursor(0,1);
      lcd.print("save            ");
      save();
      LCDmode = 0;
      Ccount = 0;
      matrix.clearScreen();
      sseg1.clear();
      sseg1.writeDisplay();
      sseg2.clear();
      sseg2.writeDisplay();
      sseg3.clear();
      sseg3.writeDisplay();
    }
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
  
  //test button plays sample tone for reference
  if (digitalRead(testPin) == LOW){
    testNote();
    while (digitalRead(testPin) == LOW){
      ;
    }
  }
  
  //nd button allows user to change note duration with REA
  while (digitalRead(ndPin) == LOW){
    noteDuration();
  }
  while (digitalRead(tempoPin) == LOW){
    tempoAdj();
  }

  //save if switch has been changed to PLAYBACK
  if (digitalRead(modePin) == LOW){
    lcd.setCursor(0,1);
    lcd.print("save            ");
    save();
    LCDmode = 0;
    ClastCount = -1;
    matrix.clearScreen();
    sseg1.clear();
    sseg1.writeDisplay();
    sseg2.clear();
    sseg2.writeDisplay();
    sseg3.clear();
    sseg3.writeDisplay();
  }
}


//plays part of the song in COMPOSE mode, while play button is held down--------------------------------------------------------------------
void composePlay(int beat, byte sB){
  if (beat == -1){
    beat = count;
  }
  long noww = millis();
  if (sB == 0){
    if (saveBuffer0[(beat - 1) % 48][0][0] != REST && saveBuffer0[(beat - 1) % 48][0][0] != SUSTAIN){
      tone1.play(saveBuffer0[(beat - 1) % 48][0][0], (saveBuffer0[(beat - 1) % 48][0][1]*tempo));
    }
    if (saveBuffer0[(beat - 1) % 48][1][0] != REST && saveBuffer0[(beat - 1) % 48][1][0] != SUSTAIN){
      tone2.play(saveBuffer0[(beat - 1) % 48][1][0], (saveBuffer0[(beat - 1) % 48][0][1]*tempo));
    }
    if (saveBuffer0[(beat - 1) % 48][2][0] != REST && saveBuffer0[(beat - 1) % 48][2][0] != SUSTAIN){
      tone3.play(saveBuffer0[(beat - 1) % 48][2][0], (saveBuffer0[(beat - 1) % 48][2][1]*tempo));
    }
    if (saveBuffer0[(beat - 1) % 48][3][0] != REST && saveBuffer0[(beat - 1) % 48][3][0] != SUSTAIN){
      tone4.play(saveBuffer0[(beat - 1) % 48][3][0], (saveBuffer0[(beat - 1) % 48][3][1]*tempo));
    }
    if (saveBuffer0[(beat - 1) % 48][4][0] != REST && saveBuffer0[(beat - 1) % 48][4][0] != SUSTAIN){
      tone5.play(saveBuffer0[(beat - 1) % 48][4][0], (saveBuffer0[(beat - 1) % 48][4][1]*tempo));
    }
    if (saveBuffer0[(beat - 1) % 48][5][0] == 1){
      digitalWrite(drumPin, !digitalRead(drumPin));
    }
  }
  if (sB == 1){
    if (saveBuffer1[(beat - 1) % 48][0][0] != REST && saveBuffer1[(beat - 1) % 48][0][0] != SUSTAIN){
      tone1.play(saveBuffer1[(beat - 1) % 48][0][0], (saveBuffer1[(beat - 1) % 48][0][1]*tempo));
    }
    if (saveBuffer1[(beat - 1) % 48][1][0] != REST && saveBuffer1[(beat - 1) % 48][1][0] != SUSTAIN){
      tone2.play(saveBuffer1[(beat - 1) % 48][1][0], (saveBuffer1[(beat - 1) % 48][0][1]*tempo));
    }
    if (saveBuffer1[(beat - 1) % 48][2][0] != REST && saveBuffer1[(beat - 1) % 48][2][0] != SUSTAIN){
      tone3.play(saveBuffer1[(beat - 1) % 48][2][0], (saveBuffer1[(beat - 1) % 48][2][1]*tempo));
    }
    if (saveBuffer1[(beat - 1) % 48][3][0] != REST && saveBuffer1[(beat - 1) % 48][3][0] != SUSTAIN){
      tone4.play(saveBuffer1[(beat - 1) % 48][3][0], (saveBuffer1[(beat - 1) % 48][3][1]*tempo));
    }
    if (saveBuffer1[(beat - 1) % 48][4][0] != REST && saveBuffer1[(beat - 1) % 48][4][0] != SUSTAIN){
      tone5.play(saveBuffer1[(beat - 1) % 48][4][0], (saveBuffer1[(beat - 1) % 48][4][1]*tempo));
    }
    if (saveBuffer1[(beat - 1) % 48][5][0] == 1){
      digitalWrite(drumPin, !digitalRead(drumPin));
    }
  }
  if (sB > 1){
    ;
  }
  while (millis() < (noww + tempo)){
    ;
  }
}


//Displays the Main Menu on the LCD-------------------------------------------------------------------------------------------------------------------------------
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
      redrawLCD();
    }
    if (Ccount == 1){
      LCDmode = 2;
    }
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
  
  if (digitalRead(modePin) == LOW){
    LCDmode = 0;
    Ccount = 0;
    ClastCount = 0;
    redrawLCD();
    matrix.clearScreen();
    sseg1.clear();
    sseg1.writeDisplay();
    sseg2.clear();
    sseg2.writeDisplay();
    sseg3.clear();
    sseg3.writeDisplay();
  }
}

//Shows list of songs to load on LCD--------------------------------------------------------------------------------------------------------------------
void LCDloadList() {
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
    redrawLCD();
  }
  if (digitalRead(SWCpin) == LOW){
    if (Ccount == -1){
      LCDmode = 0;
      Ccount = 0;
    }
    else{  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(LCDlist[0]);
      lcd.setCursor(0,1);
      lcd.print("load");
      songFile = char2string(LCDlist[0]) + ".TXT";
      startComposer();
      lcd.setCursor(0,1);
      lcd.print(">SAVE  SAVE&QUIT");
      reloadBuffer(1);
      redrawMatrix(matrixBuffer);
      matrix.setPixel(15, Bcount);
      matrix.setPixel(0, Bcount);
      matrix.writeScreen();
      sseg1.print(8);
      sseg2.writeDigitNum(0, (currentOctave + 1));
      sseg2.writeDigitNum(4, 1);
      sseg1.writeDisplay();
      sseg2.writeDisplay();
      LCDmode = 2;
      Ccount = 0;
      ClastCount = 0;
    }
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
  if (digitalRead(modePin) == LOW){
    LCDmode = 0;
    Ccount = 0;
    ClastCount = 0;
    redrawLCD();
    matrix.clearScreen();
    sseg1.clear();
    sseg1.writeDisplay();
    sseg2.clear();
    sseg2.writeDisplay();
    sseg3.clear();
    sseg3.writeDisplay();
  }
}


//Shows list of songs to load on LCD--------------------------------------------------------------------------------------------------------------------
void LCDplayList() {
  //check if rotary encoder C has moved
  cState = digitalRead(RECpinA);
  if((cLast == LOW) && (cState == HIGH)){
    if(digitalRead(RECpinB) == LOW && Ccount > 0){
      Ccount--;
    }
    else if (digitalRead(RECpinB) == HIGH && Ccount < (songCount - 1)){
      Ccount++;
    }
  }
  cLast = cState;
  if (Ccount != ClastCount){
    ClastCount = Ccount;
    redrawLCD();
  }
  if (digitalRead(SWCpin) == LOW){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(LCDlist[0]);
    songFile = char2string(LCDlist[0]) + ".TXT";
    play();
    Ccount = 0;
    ClastCount = 1;
    
    while (digitalRead(SWCpin) == LOW){
      ;
    }
  }
  
  //check if rotary encoder B has moved to adjust octave
  bState = digitalRead(REBpinA);
  if((bLast == LOW) && (bState == HIGH)){
    if(digitalRead(REBpinB) == LOW && currentOctave > 0){
      currentOctave--;
      sseg2.writeDigitNum(0, currentOctave + 1);
      sseg2.writeDisplay();
    }
    else if (digitalRead(REBpinB) == HIGH && currentOctave < 6){
      currentOctave++;
      sseg2.writeDigitNum(0, currentOctave + 1);
      sseg2.writeDisplay();
    }
  }
  bLast = bState;
  
  if (digitalRead(modePin) == HIGH){
    Ccount = 0;
    ClastCount = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(">LOAD");
    lcd.setCursor(1,1);
    lcd.print("NEW SONG");
    matrix.clearScreen();
    sseg1.clear();
    sseg1.writeDisplay();
    sseg2.clear();
    sseg2.writeDisplay();
    sseg3.clear();
    sseg3.writeDisplay();
  }
}


//redraws character LCD with the approporiate 2 file names--------------------------------------------------------------------------------------------------
void redrawLCD(){
  songCount = fileNum();
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


//adjust tempo when tempo button held down-----------------------------------------------------------------------------------------------
void tempoAdj(){
  unsigned long noww = millis();
  while (digitalRead(tempoPin) == LOW){
    //check if rotary encoder A has moved
    aState = digitalRead(REApinA);
    if((aLast == LOW) && (aState == HIGH)){
      if(digitalRead(REApinB) == LOW){
        if(digitalRead(SWApin) == LOW){
          if(LEDtempo > 82){
            LEDtempo = LEDtempo - 50;
            tempo = round(60000/LEDtempo);
          }
          else if(LEDtempo <= 42){
            LEDtempo = 33;
            tempo = round(60000/LEDtempo);
          }
        }
        else if(digitalRead(SWApin) == HIGH && LEDtempo > 33){
          LEDtempo--;
          tempo = round(60000/LEDtempo);
        }
      }
      else{
        if(digitalRead(SWApin) == LOW){
          if(LEDtempo < 951){
            LEDtempo = LEDtempo + 50;
            tempo = round(60000/LEDtempo);
          }
          else if(LEDtempo >= 951){
            LEDtempo = 1000;
            tempo = round(60000/LEDtempo);
          }
        }
        else if(digitalRead(SWApin) == HIGH && LEDtempo < 1000){
          LEDtempo++;
          tempo = round(60000/LEDtempo);
        }
      }
      sseg3.print(LEDtempo);
      sseg3.writeDisplay();
    }
    aLast = aState;
  
    if (millis() >= (noww + tempo)){
      digitalWrite(drumPin, !digitalRead(drumPin));
      noww = millis();
    }
  }
}


//places (or deletes) the note chosen by user by pressing SWBpin--------------------------------------------------------------------------------------------------------------------------
void placeNote(){
  //sbIndex is the index in saveBuffer where the placed note will go
  byte sbIndex = (count + 6) % 48;
  //if placed beat lies in saveBuffer0
  if (sbIndex > 6 || count < 10){
    //for percussion beat
    if (Bcount == 14){
      if (saveBuffer0[sbIndex][5][0] == 0){
        saveBuffer0[sbIndex][5][0] = 1;
      }
      else{
        saveBuffer0[sbIndex][5][0] = 0;
      }
    }
    else{
      byte f = 6;   //the tone # that is OK to write to (aka lowest unused tone) (6 means all tones in use) (7 means notes will overlap) (>9 means delete tone #(f-10))
      for (byte g = 0; g < 5; g++){
        if (saveBuffer0[sbIndex][g][0] == octave[currentOctave][13-Bcount]){
          f = g + 10;
          break;
        }
      }
      //if placed note will overlap with existing note of same pitch
      if (f == 6){
        for (byte g = 0; g < 5; g++){
          for (byte h = 0; h < noteDur; h++){
            if ((sbIndex + h) < 48){
              if (saveBuffer0[sbIndex + h][g][0] == octave[currentOctave][13-Bcount] || saveBuffer0[sbIndex + h][g][1] == octave[currentOctave][13-Bcount]){
                f = 7;
              }
            }
            //if note crosses into saveBuffer1
            else{
              if (saveBuffer1[(sbIndex + h) - 48][g][0] == octave[currentOctave][13-Bcount] || saveBuffer1[(sbIndex + h) - 48][g][1] == octave[currentOctave][13-Bcount]){
                f = 7;
              }
            }
          }
        }
      }
      if (f == 6){
        for (byte g = 0; g < 5; g++){
          for (byte h = 0; h < noteDur; h++){
            if ((sbIndex + h) < 48){
              if (saveBuffer0[sbIndex + h][g][0] != REST){
                break;
              }
            }
            else{
              if (saveBuffer1[(sbIndex + h) - 48][g][0] != REST){
                break;
              }
            }
            if (h == (noteDur - 1)){
              if (f > g){
                f = g;
              }
            }
          }
        }
      }
      //flash to show beat is full
      if (f == 6){
        for (byte h = 0; h < 2; h++){
          for (byte i = 0; i < 16; i++){
            matrix.setPixel(15, i);
          }
          matrix.writeScreen();
          delay(200);
          redrawMatrix(matrixBuffer);
          matrix.setPixel(0, Bcount);
          matrix.setPixel(15, Bcount);
          matrix.writeScreen();
          delay(200);
        }
      }
      //flash to show note will overlap
      if (f == 7){
        for (byte h = 0; h < 2; h++){
          for (byte i = 0; i < 24; i++){
            matrix.setPixel(i, Bcount);
          }
          matrix.writeScreen();
          delay(200);
          redrawMatrix(matrixBuffer);
          matrix.setPixel(0, Bcount);
          matrix.setPixel(15, Bcount);
          matrix.writeScreen();
          delay(200);
        }
      }
      else if (f < 6){
        saveBuffer0[sbIndex][f][0] = octave[currentOctave][13-Bcount];
        saveBuffer0[sbIndex][f][1] = noteDur;
        for (byte h = 1; h < noteDur; h++){
          //if beat is in saveBuffer0
          if ((sbIndex + h) < 48){
            saveBuffer0[sbIndex + h][f][0] = SUSTAIN;
            saveBuffer0[sbIndex + h][f][1] = octave[currentOctave][13-Bcount];
          }
          //if beat is in saveBuffer1
          else{
            saveBuffer1[(sbIndex + h) - 48][f][0] = SUSTAIN;
            saveBuffer1[(sbIndex + h) - 48][f][1] = octave[currentOctave][13-Bcount];
          }
        }
      }
      else if (f > 7){
        byte delDur = saveBuffer0[sbIndex][f-10][1];
        for (byte d = 0; d < delDur; d++){
          if ((sbIndex + d) < 48){
            saveBuffer0[sbIndex + d][f-10][0] = REST;
            saveBuffer0[sbIndex + d][f-10][1] = 1;
          }
          //if note being deleted crosses into saveBuffer1
          else{
            saveBuffer1[(sbIndex + d) - 48][f-10][0] = REST;
            saveBuffer1[(sbIndex + d) - 48][f-10][1] = 1;
          }
        }
      }
    }
  }
  //if the placed beat lies in saveBuffer1
  else{
    //for percussion beat
    if (Bcount == 14){
      if (saveBuffer1[sbIndex][5][0] == 0){
        saveBuffer1[sbIndex][5][0] = 1;
      }
      else{
        saveBuffer1[sbIndex][5][0] = 0;
      }
    }
    else{
      byte f = 6;   //the tone # that is OK to write to (aka lowest unused tone) (6 means all tones in use) (>9 means delete tone #(f-10))
      for (byte g = 0; g < 5; g++){
        if (saveBuffer1[sbIndex][g][0] == octave[currentOctave][13-Bcount]){
          f = g + 10;
          break;
        }
      }
      //if placed note will overlap with existing note of same pitch
      if (f == 6){
        for (byte g = 0; g < 5; g++){
          for (byte h = 0; h < noteDur; h++){
            if (saveBuffer1[sbIndex + h][g][0] == octave[currentOctave][13-Bcount] || saveBuffer1[sbIndex + h][g][1] == octave[currentOctave][13-Bcount]){
              f = 7;
            }
          }
        }
      }
      if (f == 6){   
        for (byte g = 0; g < 5; g++){
          for (byte h = 0; h < noteDur; h++){
            if (saveBuffer1[sbIndex + h][g][0] != REST){
              break;
            }
            if (h == (noteDur - 1)){
              if (f > g){
                f = g;
              }
            }
          }
        }
      }
      //flash to show beat is full
      if (f == 6){
        for (byte h = 0; h < 2; h++){
          for (byte i = 0; i < 16; i++){
            matrix.setPixel(15, i);
          }
          matrix.writeScreen();
          delay(200);
          redrawMatrix(matrixBuffer);
          matrix.setPixel(0, Bcount);
          matrix.setPixel(15, Bcount);
          matrix.writeScreen();
          delay(200);
        }
      }
      //flash to show note will overlap
      if (f == 7){
        for (byte h = 0; h < 2; h++){
          for (byte i = 0; i < 24; i++){
            matrix.setPixel(i, Bcount);
          }
          matrix.writeScreen();
          delay(200);
          redrawMatrix(matrixBuffer);
          matrix.setPixel(0, Bcount);
          matrix.setPixel(15, Bcount);
          matrix.writeScreen();
          delay(200);
        }
      }
      else if (f < 6){
        saveBuffer1[sbIndex][f][0] = octave[currentOctave][13-Bcount];
        saveBuffer1[sbIndex][f][1] = noteDur;
        for (byte h = 1; h < noteDur; h++){
          saveBuffer1[sbIndex + h][f][0] = SUSTAIN;
          saveBuffer1[sbIndex + h][f][1] = octave[currentOctave][13-Bcount];
        }
      }
      else if (f > 7){
        byte delDur = saveBuffer1[sbIndex][f-10][1];
        for (byte d = 0; d < delDur; d++){
          saveBuffer1[sbIndex + d][f-10][0] = REST;
          saveBuffer1[sbIndex + d][f-10][1] = 1;
        }
      }
    }
  }
    
  reloadBuffer(count);
  redrawMatrix(matrixBuffer);
  matrix.setPixel(15, Bcount);
  matrix.setPixel(0, Bcount);
  matrix.writeScreen();
}


//Converts char array[20] to string (excluding the file extension)----------------------------------------------------------------------------------------
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


//while nd button is pressed, allow user to change note duration with REA----------------------------------------------------------------------------------
void noteDuration(){
  //check if rotary encoder A has moved
  aState = digitalRead(REApinA);
  if((aLast == LOW) && (aState == HIGH)){
    if(digitalRead(REApinB) == LOW && noteDur > 1){
      noteDur--;
    }
    if(digitalRead(REApinB) == HIGH && noteDur < 36){
      noteDur++;
    }
    sseg2.print(0);
    byte dT = (noteDur/10U) % 10;
    byte dO = (noteDur/1U) % 10;
    if (dT != 0){
      sseg2.writeDigitNum(3, dT);
    }
    sseg2.writeDigitNum(4, dO);
    sseg2.writeDigitNum(0, (currentOctave + 1));
    sseg2.writeDisplay();
  }
  aLast = aState;
}


//plays a sample of the note the cursor is on for reference---------------------------------------------------------------------------------------------------------------
void testNote(){
  if (Bcount == 14){
    delay(100);
    digitalWrite(drumPin, !digitalRead(drumPin));
  }
  else{
    tone1.play(octave[currentOctave][13-Bcount], 500);
    tone2.play(octave[currentOctave][13-Bcount], 500);
    tone3.play(octave[currentOctave][13-Bcount], 500);
    tone4.play(octave[currentOctave][13-Bcount], 500);
    tone5.play(octave[currentOctave][13-Bcount], 500);
  }
}



//Stitch together temp file into a long file meant for playback and storage-------------------------------------------------------------------------------------------------
void save(){
  //save saveBuffer0 and savebuffer1 to their respective temp files
  SD.chdir("/Temp");
  file = SD.open(String(lastFile) + ".TXT", FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  // Rewind the file to desired point
  file.seek(0);
  for (byte x = 0; x < 48; x++){
    for (byte y = 0; y < 6; y++){
      if (y < 5){
        for (byte z = 0; z < 2; z++){
          file.print(saveBuffer0[x][y][z]);
          file.print(",");
        }
      }
      else{
          file.print(saveBuffer0[x][y][0]);
          file.print(",");
          file.print(saveBuffer0[x][y][1]);
      }
    }
    file.print("\r\n");
  }
  file.close();
  file = SD.open(String(lastFile + 1) + ".TXT", FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  // Rewind the file to desired point
  file.seek(0);
  for (byte x = 0; x < 48; x++){
    for (byte y = 0; y < 6; y++){
      if (y < 5){
        for (byte z = 0; z < 2; z++){
          file.print(saveBuffer1[x][y][z]);
          file.print(",");
        }
      }
      else{
          file.print(saveBuffer1[x][y][0]);
          file.print(",");
          file.print(saveBuffer1[x][y][1]);
      }
    }
    file.print("\r\n");
  }
  file.close();
    
  //determine number of temp files
  byte fileCount = 0;
  SD.chdir("/Temp");
  while (file.openNext(SD.vwd(), O_READ)) {
    if (!file.isHidden()) {
      fileCount++;
    }
    file.close();
  }

  //determine the totaly number of beats in the song
  file = SD.open(String(fileCount) + ".TXT");
  if (!file) {
    Serial.println("Error determining totalBeats");
    return;
  }
  file.seek(0);
  // Read the file and print fields
  int16_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
  byte n = 0;
  while (file.available()) {
    if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
      Serial.println("read error");
      int ch;
      int nr = 0;
      // print part of file after error.
      while ((ch = file.read()) > 0 && nr++ < 100) {
        Serial.write(ch);
      }
      break;            
    }
    n++;
  }
  totalBeats = (48*(fileCount - 1)) + n;

  //write metadate to header
  SD.chdir();
  SD.remove(LCDlist[0]);
  file = SD.open(songFile, FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  file.seek(0);
  file.print(LEDtempo);
  file.print(",");
  file.print(totalBeats);
  for (byte x = 0; x < 10; x++){
    file.print(",");
    file.print(0);
  }
  file.print("\r\n");
  file.close();

  //open temp files and append them to save file
  for (byte m = 1; m < (fileCount + 1); m++){
    SD.chdir("/Temp");
    // open temp file
    file = SD.open(String(m) + ".TXT");
    if (!file) {
      Serial.println("no more temp");
    }
    //load temp file into saveBufferX
    file.seek(0);
    // Read the file and print fields.
    n = 0;
    while (file.available()) {
      if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
        Serial.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = file.read()) > 0 && nr++ < 100) {
          Serial.write(ch);
        }
        break;            
      }
      saveBufferX[n][0][0] = t1;
      saveBufferX[n][0][1] = t2;
      saveBufferX[n][1][0] = t3;
      saveBufferX[n][1][1] = t4;
      saveBufferX[n][2][0] = t5;
      saveBufferX[n][2][1] = t6;
      saveBufferX[n][3][0] = t7;
      saveBufferX[n][3][1] = t8;
      saveBufferX[n][4][0] = t9;
      saveBufferX[n][4][1] = t10;
      saveBufferX[n][5][0] = t11;
      saveBufferX[n][5][1] = t12;
      n++;
    }
    file.close();

    lcd.setCursor(3+m,1);
    lcd.print(".");
  
    SD.chdir();
    // Create a file.
    file = SD.open(songFile, FILE_WRITE);
    if (!file) {
      Serial.println("open failed");
      return;
    }
    //Append saveBufferX to saveFile
    file.seek(-1);
    if (m != fileCount){
      for (byte x = 0; x < 48; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for (byte z = 0; z < 2; z++){
              file.print(saveBufferX[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(saveBufferX[x][y][0]);
              file.print(",");
              file.print(saveBufferX[x][y][1]);
          }
        }
        file.print("\r\n");
      }
    }
    else if (m == fileCount){
      for (byte x = 0; x < n; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for (byte z = 0; z < 2; z++){
              file.print(saveBufferX[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(saveBufferX[x][y][0]);
              file.print(",");
              file.print(saveBufferX[x][y][1]);
          }
        }
        file.print("\r\n");
      }
    }
    file.close();
  }
  Serial.println("Save");
}


//refresh Buffer from saveBuffers when scrolling left/right---------------------------------------------------------------------------------
void reloadBuffer(int beat){
  if (beat < 1){
    beat = 1;
  }
  //determine which temp file to load
  byte f = (beat/48);
  if (beat%48 != 0){
    f++;
  }
  if (f != lastFile){
    //save saveBuffer0 and savebuffer1 to their respective temp files
    if (lastFile != 0){
      SD.chdir("/Temp");
      file = SD.open(String(lastFile) + ".TXT", FILE_WRITE);
      if (!file) {
        Serial.println("open failed");
        return;
      }
      // Rewind the file to desired point
      file.seek(0);
      for (byte x = 0; x < 48; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for (byte z = 0; z < 2; z++){
              file.print(saveBuffer0[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(saveBuffer0[x][y][0]);
              file.print(",");
              file.print(saveBuffer0[x][y][1]);
          }
        }
        file.print("\r\n");
      }
      file.close();
      file = SD.open(String(lastFile + 1) + ".TXT", FILE_WRITE);
      if (!file) {
        Serial.println("open failed");
        return;
      }
      // Rewind the file to desired point
      file.seek(0);
      for (byte x = 0; x < 48; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for (byte z = 0; z < 2; z++){
              file.print(saveBuffer1[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(saveBuffer1[x][y][0]);
              file.print(",");
              file.print(saveBuffer1[x][y][1]);
          }
        }
        file.print("\r\n");
      }
    }
    file.close();
    
    lastFile = f;
    SD.chdir("/Temp");
    //open file on SD
    file = SD.open(String(f) + ".TXT");
    if (!file) {
      Serial.println("open failed");
      return;
    }
    // Rewind the file to desired point
    file.seek(0);
    // Read the file and print fields.
    int16_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
    byte n = 0;
    while (file.available()) {
      if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
        Serial.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = file.read()) > 0 && nr++ < 100) {
          Serial.write(ch);
        }
        break;            
      }
      saveBuffer0[n][0][0] = t1;
      saveBuffer0[n][0][1] = t2;
      saveBuffer0[n][1][0] = t3;
      saveBuffer0[n][1][1] = t4;
      saveBuffer0[n][2][0] = t5;
      saveBuffer0[n][2][1] = t6;
      saveBuffer0[n][3][0] = t7;
      saveBuffer0[n][3][1] = t8;
      saveBuffer0[n][4][0] = t9;
      saveBuffer0[n][4][1] = t10;
      saveBuffer0[n][5][0] = t11;
      saveBuffer0[n][5][1] = t12;
      n++;
    }
    file.close();
    //Load saveBuffer1 with next file
    //open file on SD
    file = SD.open(String(f+1) + ".TXT");
    if (!file) {
      Serial.println("open failed");
      return;
    }
    // Rewind the file to desired point
    file.seek(0);
    // Read the file and print fields.
    n = 0;
    while (file.available()) {
      if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
        Serial.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = file.read()) > 0 && nr++ < 100) {
          Serial.write(ch);
        }
        break;            
      }
      saveBuffer1[n][0][0] = t1;
      saveBuffer1[n][0][1] = t2;
      saveBuffer1[n][1][0] = t3;
      saveBuffer1[n][1][1] = t4;
      saveBuffer1[n][2][0] = t5;
      saveBuffer1[n][2][1] = t6;
      saveBuffer1[n][3][0] = t7;
      saveBuffer1[n][3][1] = t8;
      saveBuffer1[n][4][0] = t9;
      saveBuffer1[n][4][1] = t10;
      saveBuffer1[n][5][0] = t11;
      saveBuffer1[n][5][1] = t12;
      n++;
    }
    file.close();
  }
  //load matrixBuffer from the saveBuffers
  //sbIndex is the index in saveBuffer0 to begin loading into matrixBuffer
  byte sbIndex = (beat - 1) % 48;
  if (sbIndex < 37){
    byte k = 0;
    for (byte m = sbIndex; m < (sbIndex + 12); m++){
      for (byte o = 0; o < 6; o++){
        matrixBuffer[k][o][0] = saveBuffer0[m][o][0];
        matrixBuffer[k][o][1] = saveBuffer0[m][o][1];
      }
      k++;
    }
  }
  // if matrixBuffer will be split accross saveBuffer0 and saveBuffer1
  else{
    byte k = 0;
    for (byte m = sbIndex; m < 48; m++){
      for (byte o = 0; o < 6; o++){
        matrixBuffer[k][o][0] = saveBuffer0[m][o][0];
        matrixBuffer[k][o][1] = saveBuffer0[m][o][1];
      }
      k++;
    }
    for (byte p = 0; p < (sbIndex - 36); p++){
      for (byte q = 0; q < 6; q++){
        matrixBuffer[k][q][0] = saveBuffer1[p][q][0];
        matrixBuffer[k][q][1] = saveBuffer1[p][q][1];
      }
      k++;
    }
  }
}


//plays file back directly from SD card using a 12-beat buffer----------------------------------------------------------------------------------------------------
void play(){
  //open file on SD
  file = SD.open(songFile);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  // Rewind the file for read.
  file.seek(0);
  // Read the file and print fields.
  int16_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
  int n = 0;
  unsigned long noww;
  while (file.available()) {
    noww = millis();
    if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
      Serial.println("read error");
      int ch;
      int nr = 0;
      // print part of file after error.
      while ((ch = file.read()) > 0 && nr++ < 100) {
        Serial.write(ch);
      }
      break;            
    }
    if (n == 0){
      tempo = round(60000/t1);
      totalBeats = t2;    
      sseg2.writeDigitNum(0, (currentOctave + 1));
      sseg2.writeDisplay();
      sseg3.print(t1);
      sseg3.writeDisplay();
    }
    if (n <= 12 && n != 0){
      matrixBuffer[n-1][0][0] = t1;
      matrixBuffer[n-1][0][1] = t2;
      matrixBuffer[n-1][1][0] = t3;
      matrixBuffer[n-1][1][1] = t4;
      matrixBuffer[n-1][2][0] = t5;
      matrixBuffer[n-1][2][1] = t6;
      matrixBuffer[n-1][3][0] = t7;
      matrixBuffer[n-1][3][1] = t8;
      matrixBuffer[n-1][4][0] = t9;
      matrixBuffer[n-1][4][1] = t10;
      matrixBuffer[n-1][5][0] = t11;
      matrixBuffer[n-1][5][1] = t12;
    }
    if (n > 12){
      if (matrixBuffer[0][0][0] != REST && matrixBuffer[0][0][0] != SUSTAIN){
        tone1.play(matrixBuffer[0][0][0], (matrixBuffer[0][0][1]*tempo));
      }
      if (matrixBuffer[0][1][0] != REST && matrixBuffer[0][1][0] != SUSTAIN){
        tone2.play(matrixBuffer[0][1][0], (matrixBuffer[0][0][1]*tempo));
      }
      if (matrixBuffer[0][2][0] != REST && matrixBuffer[0][2][0] != SUSTAIN){
        tone3.play(matrixBuffer[0][2][0], (matrixBuffer[0][2][1]*tempo));
      }
      if (matrixBuffer[0][3][0] != REST && matrixBuffer[0][3][0] != SUSTAIN){
        tone4.play(matrixBuffer[0][3][0], (matrixBuffer[0][3][1]*tempo));
      }
      if (matrixBuffer[0][4][0] != REST && matrixBuffer[0][4][0] != SUSTAIN){
        tone5.play(matrixBuffer[0][4][0], (matrixBuffer[0][4][1]*tempo));
      }
      if (matrixBuffer[0][5][0] == 1){
        digitalWrite(drumPin, !digitalRead(drumPin));
      }
      redrawMatrixPlay(1);
 
      for (byte x = 0; x < 11; x++){
        for (byte y = 0; y < 6; y++){
          for(byte z = 0; z < 2; z++){
            matrixBuffer[x][y][z] = matrixBuffer[x+1][y][z];
          }
        }
      }
      matrixBuffer[11][0][0] = t1;
      matrixBuffer[11][0][1] = t2;
      matrixBuffer[11][1][0] = t3;
      matrixBuffer[11][1][1] = t4;
      matrixBuffer[11][2][0] = t5;
      matrixBuffer[11][2][1] = t6;
      matrixBuffer[11][3][0] = t7;
      matrixBuffer[11][3][1] = t8;
      matrixBuffer[11][4][0] = t9;
      matrixBuffer[11][4][1] = t10;
      matrixBuffer[11][5][0] = t11;
      matrixBuffer[11][5][1] = t12;

      sseg1.print(n-12);
      sseg1.writeDisplay();
      
      while (millis() < (noww + tempo)){
        ;
      }
    }
    
    if (n == totalBeats){
      for (byte m = 0; m < 12; m++){
        noww = millis();
        if (matrixBuffer[m][0][0] != REST && matrixBuffer[m][0][0] != SUSTAIN){
          tone1.play(matrixBuffer[m][0][0], (matrixBuffer[m][0][1]*tempo));
        }
        if (matrixBuffer[m][1][0] != REST && matrixBuffer[m][1][0] != SUSTAIN){
          tone2.play(matrixBuffer[m][1][0], (matrixBuffer[m][0][1]*tempo));
        }
        if (matrixBuffer[m][2][0] != REST && matrixBuffer[m][2][0] != SUSTAIN){
          tone3.play(matrixBuffer[m][2][0], (matrixBuffer[m][2][1]*tempo));
        }
        if (matrixBuffer[m][3][0] != REST && matrixBuffer[m][3][0] != SUSTAIN){
          tone4.play(matrixBuffer[m][3][0], (matrixBuffer[m][3][1]*tempo));
        }
        if (matrixBuffer[m][4][0] != REST && matrixBuffer[m][4][0] != SUSTAIN){
          tone5.play(matrixBuffer[m][4][0], (matrixBuffer[m][4][1]*tempo));
        }
        if (matrixBuffer[m][5][0] == 1){
          digitalWrite(drumPin, !digitalRead(drumPin));
        }
        redrawMatrixPlay((2*m)+1);

        sseg1.print(totalBeats - (11-m));
        sseg1.writeDisplay();
      
        while (millis() < (noww + tempo)){
          ;
        }
      }
    }
    n++;

    if (digitalRead(stopPin) == LOW){
      while (digitalRead(stopPin) == LOW){
        ;
      }
      matrix.clearScreen();
      sseg1.clear();
      sseg1.writeDisplay();
      sseg2.clear();
      sseg2.writeDisplay();
      sseg3.clear();
      sseg3.writeDisplay();
      file.close();
      return;
    }
    
  }
  file.close();
}



//redraws LED matrix using 'matrixx' matrix (used in COMPOSE mode)------------------------------------------------------------------------------------------
void redrawMatrix(int matrixx[12][6][2]){
  matrix.clearScreen();
  byte startIndex;
  if (count > 0){
    for (byte k = 0; k < 5; k++){
      if ((matrixx[7][k][0] < octave[currentOctave][0]) && (matrixx[7][k][0] > SUSTAIN)){
        matrix.setPixel(15, 15);
      }
      if ((matrixx[7][k][0] == SUSTAIN) && (matrixx[7][k][1] < octave[currentOctave][0])){
        matrix.setPixel(15, 15);
      }
      if (matrixx[7][k][0] > octave[currentOctave][11]){
        matrix.setPixel(15, 0);
      }
      if ((matrixx[7][k][0] == SUSTAIN) && (matrixx[7][k][1] > octave[currentOctave][11])){
        matrix.setPixel(15, 0);
      }
    }
    startIndex = 0;
  }
  //if matrix overscrolled below 1
  else{
    for (byte k = 0; k < 5; k++){
      if ((matrixx[count+6][k][0] < octave[currentOctave][0]) && (matrixx[count+6][k][0] > SUSTAIN)){
        matrix.setPixel(15, 15);
      }
      if ((matrixx[count+6][k][0] == SUSTAIN) && (matrixx[count+6][k][1] < octave[currentOctave][0])){
        matrix.setPixel(15, 15);
      }
      if (matrixx[count+6][k][0] > octave[currentOctave][11]){
        matrix.setPixel(15, 0);
      }
      if ((matrixx[count+6][k][0] == SUSTAIN) && (matrixx[count+6][k][1] > octave[currentOctave][11])){
        matrix.setPixel(15, 0);
      }
    }
    startIndex = (1 - count);
    for (byte k = 1; k < (2 - (2*count)); k++){
      if (k%2 == 1){
        for (byte j = 0; j < 16; j++){
          if (j%2 == 0){
            matrix.setPixel(k, j);
          }
        }
      }
      else{
        for (byte j = 0; j < 16; j++){
          if (j%2 == 1){
            matrix.setPixel(k, j);
          }
        }
      }
    }
  }
  for (byte n = startIndex; n < 12; n++){
    for (byte m = 0; m < 6; m++){
      if (m < 5){
        for (int8_t o = 0; o < 12; o++){
          if (matrixx[n-startIndex][m][0] == octave[currentOctave][o]){
            matrix.setPixel((2*n)+1, (13-o));
          }
          if (matrixx[n-startIndex][m][0] == SUSTAIN && matrixx[n-startIndex][m][1] == octave[currentOctave][o]){
            matrix.setPixel((2*n)+1, (13-o));
            if (((2*n)+1) != 1){
              matrix.setPixel((2*n), (13-o));
            }
          }
        }
      }
      else{
        if (matrixx[n-startIndex][m][0] == 1){
          matrix.setPixel((2*n)+1, 14);
        }
      }
    }
  }
}


//redraws martix starting at beat number 'beat' (used in PLAYBACK mode)-------------------------------------------------------------------------------------
void redrawMatrixPlay(byte beat){
  matrix.clearScreen();
  for (int8_t n = 0; n < 12; n++){
    for (byte m = 0; m < 6; m++){
      if (m < 5){
        for (int8_t o = 0; o < 12; o++){
          if (matrixBuffer[n][m][0] == octave[currentOctave][o]){
            matrix.setPixel((2*n)+1, (13-o));
          }
          if (matrixBuffer[n][m][0] == SUSTAIN && matrixBuffer[n][m][1] == octave[currentOctave][o]){
            matrix.setPixel((2*n)+1, (13-o));
            if (((2*n)+1) != 1){
              matrix.setPixel((2*n), (13-o));
            }
          }
        }
      }
      else{
        if (matrixBuffer[n][m][0] == 1){
          matrix.setPixel((2*n)+1, 14);
        }
      }
    }
  }
  for (byte q = 0; q < 16; q++){
    matrix.setPixel(beat, q);
  }
  for (byte r = 0; r < 5; r++){
    for (int8_t s = 0; s < 12; s++){
      if (matrixBuffer[(beat-1)/2][r][0] == octave[currentOctave][s]){
        matrix.clrPixel(beat, (13-s));
        matrix.setPixel(0, (13-s));
      }
      else if (matrixBuffer[(beat-1)/2][r][0] == SUSTAIN){
        if (matrixBuffer[(beat-1)/2][r][1] == octave[currentOctave][s]){
          matrix.clrPixel(beat, (13-s));
          matrix.setPixel(0, (13-s));
        }
      }
    }
  }
  if (matrixBuffer[(beat-1)/2][5][0] == 1){
    matrix.clrPixel(beat, 14);
    matrix.setPixel(0, 14);
  }
  matrix.writeScreen();
}


//load song file and split it into many smaller temp files in /temp---------------------------------------------------------------------------------------------------------
void startComposer(){
  //load the song and get metadata, determine number of files for splitting
  int tempBuffer[48][6][2];   // temp matrix to store 48 beats, save as "1.txt", "2.txt", etc. in /Temp
  byte fileCount;               // number of temp files needed for the splitting process
  byte remainder;
  SD.chdir();
  //open file on SD
  file = SD.open(songFile);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  // Rewind the file to desired point
  file.seek(0);
  // Read the file and print fields.
  int16_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;
  while (file.available()) {
    if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
      || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
      Serial.println("read error");
      int ch;
      int nr = 0;
      // print part of file after error.
      while ((ch = file.read()) > 0 && nr++ < 100) {
        Serial.write(ch);
      }
      break;            
    }
    //Load metadata
    tempo = round(60000/t1);
    LEDtempo = t1;
    totalBeats = t2;
    sseg3.print(LEDtempo);
    sseg3.writeDisplay();
    remainder = totalBeats % 48;
    if (remainder == 0){
      fileCount = (totalBeats / 48);
    }
    else{
      fileCount = ((totalBeats / 48) + 1);
    }
    file.close();        
  }

  if(SD.exists("Temp")){
    SD.chdir("/Temp");
    SD.vwd()->rmRfStar();    //recursive delete /temp if folder exists
  }
  SD.chdir();
  SD.mkdir("Temp");          //create new /temp

  //Load the song and split it up
  for (int m = 1; m < fileCount + 1; m++){
    SD.chdir();
    //open file on SD
    file = SD.open(songFile);
    if (!file) {
      Serial.println("open failed");
      return;
    }
    // Rewind the file to desired point
    file.seek(0);
    int n = 0;
    // Read the file and print fields.
    while (file.available()) {
      if (csvReadInt16(&file, &t1, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t2, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t3, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t4, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t5, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t6, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t7, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t8, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t9, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t10, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t11, CSV_DELIM) != CSV_DELIM
        || csvReadInt16(&file, &t12, CSV_DELIM) != '\n') {
        Serial.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = file.read()) > 0 && nr++ < 100) {
          Serial.write(ch);
        }
        break;            
      }
      if(n > (48*(m-1)) && n < ((48*m)+1)){
        tempBuffer[-(48*(m-1) - (n-1))][0][0] = t1;
        tempBuffer[-(48*(m-1) - (n-1))][0][1] = t2;
        tempBuffer[-(48*(m-1) - (n-1))][1][0] = t3;
        tempBuffer[-(48*(m-1) - (n-1))][1][1] = t4;
        tempBuffer[-(48*(m-1) - (n-1))][2][0] = t5;
        tempBuffer[-(48*(m-1) - (n-1))][2][1] = t6;
        tempBuffer[-(48*(m-1) - (n-1))][3][0] = t7;
        tempBuffer[-(48*(m-1) - (n-1))][3][1] = t8;
        tempBuffer[-(48*(m-1) - (n-1))][4][0] = t9;
        tempBuffer[-(48*(m-1) - (n-1))][4][1] = t10;
        tempBuffer[-(48*(m-1) - (n-1))][5][0] = t11;
        tempBuffer[-(48*(m-1) - (n-1))][5][1] = t12;
      }
      else if(n > (48*m)+1){
        file.close();
      }
      n++;
    }
    
    lcd.setCursor(3+m,1);
    lcd.print(".");
    
    SD.chdir("/Temp");
    // Create new split file
    file = SD.open(String(m) + ".TXT", FILE_WRITE);
    if (!file) {
      Serial.println("open failed");
      return;
    }
    //upload tempBuffer to split file
    if (m != fileCount){
      for (byte x = 0; x < 48; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for(byte z = 0; z < 2; z++){
              file.print(tempBuffer[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(tempBuffer[x][y][0]);
              file.print(",");
              file.print(tempBuffer[x][y][1]);
          }
        }
        file.print("\r\n");
      }
    }
    else if (m == fileCount){
      for (byte x = 0; x < remainder; x++){
        for (byte y = 0; y < 6; y++){
          if (y < 5){
            for(byte z = 0; z < 2; z++){
              file.print(tempBuffer[x][y][z]);
              file.print(",");
            }
          }
          else{
              file.print(tempBuffer[x][y][0]);
              file.print(",");
              file.print(tempBuffer[x][y][1]);
          }
        }
        file.print("\r\n");
      }
    }
    file.close();
  }    
}



//SD read functions below.  Who knows how they work? ------------------------------------------------------------------------------------------
int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    if (ch == delim || ch == '\n') {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}


int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}


int csvReadInt32(File* file, int32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
