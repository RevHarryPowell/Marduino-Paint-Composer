#include <Tone.h>
#include <limits.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;

#define CS_PIN 53

// example can use comma or semicolon
#define CSV_DELIM ','

File file;

#define pz1Pin 32
#define pz2Pin 34
#define pz3Pin 36
#define pz4Pin 38
#define pz5Pin 40
#define drumPin 54

byte REST = 0;
byte SUSTAIN = 1;
int tempo = 85;

Tone tone1;
Tone tone2;
Tone tone3;
Tone tone4;
Tone tone5;


void setup() {
  Serial.begin(9600);
  pinMode(drumPin, OUTPUT);
  tone1.begin(pz1Pin);
  tone2.begin(pz2Pin);
  tone3.begin(pz3Pin);
  tone4.begin(pz4Pin);
  tone5.begin(pz5Pin);
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  //open file on SD
  file = SD.open("OUTERISO.TXT");
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

    if (t1 != REST && t1 != SUSTAIN){
      tone1.play(t1, (t2*tempo));
    }
    if (t3 != REST && t3 != SUSTAIN){
      tone2.play(t3, (t4*tempo));
    }
    if (t5 != REST && t5 != SUSTAIN){
      tone3.play(t5, (t6*tempo));
    }
    if (t7 != REST && t7 != SUSTAIN){
      tone4.play(t7, (t8*tempo));
    }
    if (t9 != REST && t9 != SUSTAIN){
      tone5.play(t9, (t10*tempo));
    }
    if (t11 == 1){
      digitalWrite(drumPin, !digitalRead(drumPin));
    }
    while (millis() < (noww + tempo)){
      ;
    }

    /*songMatrix[n][0][0] = t1;
    songMatrix[n][0][1] = t2;
    songMatrix[n][1][0] = t3;
    songMatrix[n][1][1] = t4;
    songMatrix[n][2][0] = t5;
    songMatrix[n][2][1] = t6;
    songMatrix[n][3][0] = t7;
    songMatrix[n][3][1] = t8;
    songMatrix[n][4][0] = t9;
    songMatrix[n][4][1] = t10;
    songMatrix[n][5][0] = t11;
    songMatrix[n][5][1] = t12;*/
    n++;
  }
  file.close();
  //play();
}

void loop() {
  // put your main code here, to run repeatedly:

}



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



//------------------------------------------------------------------------------
int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
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


/*void play(){
  unsigned long noww;
  for (int x = 0; x < sizeof(songMatrix)/sizeof((songMatrix)[0]); x++){
    noww = millis();
    if (songMatrix[x][0][0] != REST && songMatrix[x][0][0] != SUSTAIN){
      tone1.play(songMatrix[x][0][0], songMatrix[x][0][1]*tempo);
    }
    if (songMatrix[x][1][0] != REST && songMatrix[x][1][0] != SUSTAIN){
      tone2.play(songMatrix[x][1][0], songMatrix[x][1][1]*tempo);
    }
    if (songMatrix[x][2][0] != REST && songMatrix[x][2][0] != SUSTAIN){
      tone3.play(songMatrix[x][2][0], songMatrix[x][2][1]*tempo);
    }
    if (songMatrix[x][3][0] != REST && songMatrix[x][3][0] != SUSTAIN){
      tone4.play(songMatrix[x][3][0], songMatrix[x][3][1]*tempo);
    }
    if (songMatrix[x][4][0] != REST && songMatrix[x][4][0] != SUSTAIN){
      tone5.play(songMatrix[x][4][0], songMatrix[x][4][1]*tempo);
    }
    if (songMatrix[x][5][0] != 0){
      digitalWrite(drumPin, !digitalRead(drumPin));
    }
    while (millis() < (noww + tempo)){
      ;
    }
  }
}*/
