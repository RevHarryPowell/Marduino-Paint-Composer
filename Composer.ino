#include "Adafruit_HT1632.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Tone.h>
#include <limits.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;

//define pins for LED matrix
#define HT_CS   22
#define HT_DATA 24
#define HT_WR   26

Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS);
Adafruit_7segment sseg1 = Adafruit_7segment();
Adafruit_7segment sseg2 = Adafruit_7segment();
Adafruit_7segment sseg3 = Adafruit_7segment();

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
int Ccount = 1;
int ClastCount = Ccount;
int cState;
int cLast;

String songFile = "OILONG.TXT";

byte REST = 0;
byte SUSTAIN = 1;
byte currentOctave = 3;
int LEDtempo;
int tempo;
int totalBeats;
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




void setup() {
  Serial.begin(9600);
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
  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  sseg1.begin(0x70);
  sseg2.begin(0x71);
  sseg3.begin(0x72);
  matrix.clearScreen();
  sseg1.writeDisplay();
  sseg2.writeDisplay();
  sseg3.writeDisplay();
  matrix.setRotation(0);
  aLast = digitalRead(REApinA);

  
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  /*// Remove existing file.
  SD.remove("COMPTEST.TXT"); 
  // Create a file.
  file = SD.open("COMPTEST.TXT", FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  //Write blank header
  file.print("0,0,0,0,0,0,0,0,0,0,0,0\r\n");
  redrawMatrix(compBuffer);
  sseg1.print(count);
  sseg1.writeDisplay();*/
  //open file on SD
  file = SD.open(songFile);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  // Rewind the file for read.
  file.seek(0);
  // Read the file and load 1st 12 beats into buffer and read metadata from header
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
      LEDtempo = t1;
      totalBeats = t2;
      sseg3.print(LEDtempo);
      sseg3.writeDisplay();
    }
    if (n <= 12 && n != 0){
      compBuffer[n-1][0][0] = t1;
      compBuffer[n-1][0][1] = t2;
      compBuffer[n-1][1][0] = t3;
      compBuffer[n-1][1][1] = t4;
      compBuffer[n-1][2][0] = t5;
      compBuffer[n-1][2][1] = t6;
      compBuffer[n-1][3][0] = t7;
      compBuffer[n-1][3][1] = t8;
      compBuffer[n-1][4][0] = t9;
      compBuffer[n-1][4][1] = t10;
      compBuffer[n-1][5][0] = t11;
      compBuffer[n-1][5][1] = t12;
    }
    n++;
  }
  redrawMatrix(compBuffer);
  sseg1.print(1);
  sseg2.writeDigitNum(0, (currentOctave + 1));
  sseg1.writeDisplay();
  sseg2.writeDisplay();
  file.close();
}


void loop() {
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
      else if(digitalRead(SWApin) == HIGH && count > 1){
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
  //redraw LED matrix if rotary encoder has moved
  if (count != lastCount){
    lastCount = count;
    reloadBuffer(count);
    redrawMatrix(compBuffer);
    sseg1.print(count);
    sseg1.writeDisplay();
  }
  //check if rotary encoder B has moved
  bState = digitalRead(REBpinA);
  if((bLast == LOW) && (bState == HIGH)){
    if(digitalRead(REBpinB) == LOW){
      if(digitalRead(SWApin) == LOW  && currentOctave > 0){
        currentOctave--;
        redrawMatrix(compBuffer);
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else if(digitalRead(SWApin) == HIGH && Bcount > -1){
        Bcount--;
      }
    }
    else{
      if(digitalRead(SWApin) == LOW  && currentOctave < 6){
        currentOctave++;
        redrawMatrix(compBuffer);
        sseg2.writeDigitNum(0, (currentOctave + 1));
        sseg2.writeDisplay();
      }
      else if(digitalRead(SWApin) == HIGH && Bcount < 11){
        Bcount++;
      }
    }
  }
  bLast = bState;
  //move cursor up or down if rotary encoder B has moved
  if (Bcount != BlastCount){
    BlastCount = Bcount;
  }
}



//refresh Buffer when scrolling left/right
void reloadBuffer(int beat){
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
  int n = 0;
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
    if (n < beat){
      ;
    }
    else if (n < (beat + 12)){
      compBuffer[n-beat][0][0] = t1;
      compBuffer[n-beat][0][1] = t2;
      compBuffer[n-beat][1][0] = t3;
      compBuffer[n-beat][1][1] = t4;
      compBuffer[n-beat][2][0] = t5;
      compBuffer[n-beat][2][1] = t6;
      compBuffer[n-beat][3][0] = t7;
      compBuffer[n-beat][3][1] = t8;
      compBuffer[n-beat][4][0] = t9;
      compBuffer[n-beat][4][1] = t10;
      compBuffer[n-beat][5][0] = t11;
      compBuffer[n-beat][5][1] = t12;
    }
    if (n == (beat + 12)){
      file.close();
      return;
    }
   /*if (n < 12){
      compBuffer[n][0][0] = t1;
      compBuffer[n][0][1] = t2;
      compBuffer[n][1][0] = t3;
      compBuffer[n][1][1] = t4;
      compBuffer[n][2][0] = t5;
      compBuffer[n][2][1] = t6;
      compBuffer[n][3][0] = t7;
      compBuffer[n][3][1] = t8;
      compBuffer[n][4][0] = t9;
      compBuffer[n][4][1] = t10;
      compBuffer[n][5][0] = t11;
      compBuffer[n][5][1] = t12;
    }
    if (n == 12){
      file.close();
      return;
    }*/
    n++;
  }
  file.close();
}

//redraws LED matrix starting at 'beat' number of 'matrixx' matrix
void redrawMatrix(int matrixx[12][6][2]){
  matrix.clearScreen();
  for (int8_t n = 0; n < 12; n++){
    for (byte m = 0; m < 6; m++){
      if (m < 5){
        for (int8_t o = 0; o < 12; o++){
          if (matrixx[n][m][0] == octave[currentOctave][o]){
            for (byte p = 0; p < ((2*matrixx[n][m][1])-1); p++){
              matrix.setPixel(((2*n)+p+1), (13-o));
            }
          }
          if ((matrixx[n][m][0] < octave[currentOctave][0]) && (matrixx[n][m][0] != REST) && (matrixx[n][m][0] != SUSTAIN)){
            matrix.setPixel(22, 15);
            matrix.setPixel(23, 15);
          }
          if (matrixx[n][m][0] > octave[currentOctave][11]){
            matrix.setPixel(22, 0);
            matrix.setPixel(23, 0);
          }
        }
      }
      else{
        if (matrixx[n][m][0] == 1){
          matrix.setPixel((2*n)+1, 14);
        }
      }
    }
  }
  matrix.writeScreen();
}


//SD read functions below.  Who knows how they work?
//------------------------------------------------------------------------------
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
