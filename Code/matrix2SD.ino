#include <Tone.h>
#include <limits.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;

#define CS_PIN 53

// example can use comma or semicolon
#define CSV_DELIM ','

File file;

byte REST = 0;
byte SUSTAIN = 1;
int tempo = 85;

int songMatrix[][6][2] = {
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{1,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_DS5, 1},{NOTE_A4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_D4, 2},{NOTE_A4, 2},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{SUSTAIN, NOTE_D4},{SUSTAIN, NOTE_A4},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 3},{NOTE_A4, 3},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{SUSTAIN, NOTE_C5},{SUSTAIN, NOTE_A4},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{SUSTAIN, NOTE_C5},{SUSTAIN, NOTE_A4},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 2},{NOTE_A4, 2},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{SUSTAIN, NOTE_C5},{SUSTAIN, NOTE_A4},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{NOTE_C5, 2},{NOTE_A4, 2},{REST, 1},{REST, 1},{REST, 1},{0,0}},
  {{SUSTAIN, NOTE_C5},{SUSTAIN, NOTE_A4},{REST, 1},{REST, 1},{REST, 1},{0,0}}
};


void setup(){
  Serial.begin(9600);
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    return;
  }
  // Remove existing file.
   SD.remove("OUTERISO.TXT"); 
  // Create the file.
  file = SD.open("OUTERISO.TXT", FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    return;
  }
  //upload songMatrix to SD file
  for (int x = 0; x < sizeof(songMatrix)/sizeof((songMatrix)[0]); x++){
    for(int y = 0; y < 6; y++){
      if (y < 5){
        for(int z = 0; z < 2; z++){
          file.print(songMatrix[x][y][z]);
          file.print(",");
        }
      }
      else{
          file.print(songMatrix[x][y][0]);
          file.print(",");
          file.print(x);
      }
    }
    file.print("\r\n");
    Serial.println(x);
  }
  file.close();
}

void loop(){}
