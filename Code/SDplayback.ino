#include "Adafruit_HT1632.h"
#include "Adafruit_GFX.h"
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

byte REST = 0;
byte SUSTAIN = 1;
byte currentOctave = 3;
int tempo;
int totalBeats;
int playBuffer[12][6][2];

int octave[7][12] = {{NOTE_C1, NOTE_CS1, NOTE_D1, NOTE_DS1, NOTE_E1, NOTE_F1, NOTE_FS1, NOTE_G1, NOTE_GS1, NOTE_A1, NOTE_AS1, NOTE_B1}, 
                     {NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2},
                     {NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3},
                     {NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4},
                     {NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5},
                     {NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6},
                     {NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7}};

void setup() {
  Serial.begin(9600);
  pinMode(drumPin, OUTPUT);
  tone1.begin(pz1Pin);
  tone2.begin(pz2Pin);
  tone3.begin(pz3Pin);
  tone4.begin(pz4Pin);
  tone5.begin(pz5Pin);
  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  matrix.clearScreen();
  matrix.setRotation(0);

  
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  //open file on SD
  file = SD.open("OIMID.TXT");
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
    }
    if (n <= 12 && n != 0){
      playBuffer[n-1][0][0] = t1;
      playBuffer[n-1][0][1] = t2;
      playBuffer[n-1][1][0] = t3;
      playBuffer[n-1][1][1] = t4;
      playBuffer[n-1][2][0] = t5;
      playBuffer[n-1][2][1] = t6;
      playBuffer[n-1][3][0] = t7;
      playBuffer[n-1][3][1] = t8;
      playBuffer[n-1][4][0] = t9;
      playBuffer[n-1][4][1] = t10;
      playBuffer[n-1][5][0] = t11;
      playBuffer[n-1][5][1] = t12;
    }
    if (n > 12){
      if (playBuffer[0][0][0] != REST && playBuffer[0][0][0] != SUSTAIN){
        tone1.play(playBuffer[0][0][0], (playBuffer[0][0][1]*tempo));
      }
      if (playBuffer[0][1][0] != REST && playBuffer[0][1][0] != SUSTAIN){
        tone2.play(playBuffer[0][1][0], (playBuffer[0][0][1]*tempo));
      }
      if (playBuffer[0][2][0] != REST && playBuffer[0][2][0] != SUSTAIN){
        tone3.play(playBuffer[0][2][0], (playBuffer[0][2][1]*tempo));
      }
      if (playBuffer[0][3][0] != REST && playBuffer[0][3][0] != SUSTAIN){
        tone4.play(playBuffer[0][3][0], (playBuffer[0][3][1]*tempo));
      }
      if (playBuffer[0][4][0] != REST && playBuffer[0][4][0] != SUSTAIN){
        tone5.play(playBuffer[0][4][0], (playBuffer[0][4][1]*tempo));
      }
      if (playBuffer[0][5][0] == 1){
        digitalWrite(drumPin, !digitalRead(drumPin));
      }
      redrawMatrix(1);
 
      for (byte x = 0; x < 11; x++){
        for (byte y = 0; y < 6; y++){
          for(byte z = 0; z < 2; z++){
            playBuffer[x][y][z] = playBuffer[x+1][y][z];
          }
        }
      }
      playBuffer[11][0][0] = t1;
      playBuffer[11][0][1] = t2;
      playBuffer[11][1][0] = t3;
      playBuffer[11][1][1] = t4;
      playBuffer[11][2][0] = t5;
      playBuffer[11][2][1] = t6;
      playBuffer[11][3][0] = t7;
      playBuffer[11][3][1] = t8;
      playBuffer[11][4][0] = t9;
      playBuffer[11][4][1] = t10;
      playBuffer[11][5][0] = t11;
      playBuffer[11][5][1] = t12;
      
      while (millis() < (noww + tempo)){
        ;
      }
    }
    
    if (n == totalBeats){
      for (byte m = 0; m < 12; m++){
        noww = millis();
        if (playBuffer[m][0][0] != REST && playBuffer[m][0][0] != SUSTAIN){
          tone1.play(playBuffer[m][0][0], (playBuffer[m][0][1]*tempo));
        }
        if (playBuffer[m][1][0] != REST && playBuffer[m][1][0] != SUSTAIN){
          tone2.play(playBuffer[m][1][0], (playBuffer[m][0][1]*tempo));
        }
        if (playBuffer[m][2][0] != REST && playBuffer[m][2][0] != SUSTAIN){
          tone3.play(playBuffer[m][2][0], (playBuffer[m][2][1]*tempo));
        }
        if (playBuffer[m][3][0] != REST && playBuffer[m][3][0] != SUSTAIN){
          tone4.play(playBuffer[m][3][0], (playBuffer[m][3][1]*tempo));
        }
        if (playBuffer[m][4][0] != REST && playBuffer[m][4][0] != SUSTAIN){
          tone5.play(playBuffer[m][4][0], (playBuffer[m][4][1]*tempo));
        }
        if (playBuffer[m][5][0] == 1){
          digitalWrite(drumPin, !digitalRead(drumPin));
        }
        redrawMatrix((2*m)+1);
        while (millis() < (noww + tempo)){
          ;
        }
      }
    }
    n++;
  }
  file.close();
}

void loop() {
  // put your main code here, to run repeatedly:

}



void redrawMatrix(byte beat){
  matrix.clearScreen();
  for (int8_t n = 0; n < 12; n++){
    for (byte m = 0; m < 6; m++){
      if (m < 5){
        for (int8_t o = 0; o < 12; o++){
          if (playBuffer[n][m][0] == octave[currentOctave][o]){
            for (byte p = 0; p < ((2*playBuffer[n][m][1])-1); p++){
              matrix.setPixel(((2*n)+p+1), (13-o));
            }
          }
          /*if ((playBuffer[n][m][0] < octave[currentOctave][0]) && (playBuffer[n][m][0] != REST) && (playBuffer[n][m][0] != SUSTAIN)){
            matrix.setPixel(22, 15);
            matrix.setPixel(23, 15);
          }
          if (playBuffer[n][m][0] > octave[currentOctave][11]){
            matrix.setPixel(22, 0);
            matrix.setPixel(23, 0);
          }*/
        }
      }
      else{
        if (playBuffer[n][m][0] == 1){
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
      if (playBuffer[(beat-1)/2][r][0] == octave[currentOctave][s]){
        matrix.clrPixel(beat, (13-s));
        matrix.setPixel(0, (13-s));
      }
      else if (playBuffer[(beat-1)/2][r][0] == SUSTAIN){
        if (playBuffer[(beat-1)/2][r][1] == octave[currentOctave][s]){
          matrix.clrPixel(beat, (13-s));
          matrix.setPixel(0, (13-s));
        }
      }
    }
  }
  if (playBuffer[(beat-1)/2][5][0] == 1){
    matrix.clrPixel(beat, 14);
    matrix.setPixel(0, 14);
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
