//Stitches together multiple temp files from /Temp dir into one long file for playback
//Completely functional


#include "Adafruit_HT1632.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Tone.h>
#include <limits.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;

//SD constants
#define CS_PIN 53
#define CSV_DELIM ','
File file;

int LEDtempo = 706;
int totalBeats;
int saveBuffer0[48][6][2];


void setup() {
  Serial.begin(9600);
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  save();
}

//Stitch together temp file into a long file meant for playback and storage
void save(){
  //determine number of temp files
  byte fileCount = 0;
  SD.chdir("/Temp");
  while (file.openNext(SD.vwd(), O_READ)) {
    if (!file.isHidden()) {
      fileCount++;
    }
    file.close();
  }
  Serial.print("fileCount = ");
  Serial.println(fileCount);

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
  SD.remove("SAVE.TXT");
  file = SD.open("SAVE.TXT", FILE_WRITE);
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
   
  for (byte m = 1; m < (fileCount + 1); m++){
    SD.chdir("/Temp");
    // open temp file
    file = SD.open(String(m) + ".TXT");
    if (!file) {
      Serial.println("no more temp");
    }
    //load temp file into saveBuffer0
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
  
    SD.chdir();
    // Create a file.
    file = SD.open("SAVE.TXT", FILE_WRITE);
    if (!file) {
      Serial.println("open failed");
      return;
    }
    //Append saveBuffer0 to saveFile
    file.seek(-1);
    if (m != fileCount){
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
    }
    else if (m == fileCount){
      for (byte x = 0; x < n; x++){
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
    }
    file.close();
  }
  Serial.println("Done");
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

void loop() {
}
