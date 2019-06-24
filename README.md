# Marduino-Paint-Composer
A stripped-down Mario Paint Composer in a box using an Arduino Mega and a bunch of Adafruit Components

CODE IS A WORK IN PROGRESS

Code currently broken up into multiple parts for testing/development purposes


**SD2matrix** reads a TXT file from the SD card and compiles the contents into the proper array format

**matrix2SD** writes array to TXT file on SD card

**SDplayback** streams song from TXT file on SD cars (so as to avoid loading it all into RAM, avoiding song length constraints)

**Composer** displays song on LED matrix.  It allows scrolling through the song horizontally and vertically with 2 rotary encoders (etch-a-sketch style).  This is the most complex part of the code so far, so it's uploaded in versions.  Version changelogs are found at the top of each file.



**Song Array Format**

The song is stored in a 3-dimensional array[][6][2].  Elemnents in dimension 1 represent a single beat.  The 0th element is a header with some metadata.  It contains the song's tempo (in BMP) and the total number of beats.  After that, further elements look like this:

{{Piezo1 pitch, note duration},{Piezo2 pitch, note duration},{Piezo3  pitch, note duration},{Piezo4 pitch, note duration},{Piezo5 pitch, note duration},{Percussion, note index}}

Pitches are predefined in the Tone library.  Duration is in number of beats. REST means no note is played.  SUSTAIN means a note is already being played on this channel (in which case the note duration becomes the note being sustained). For the percussion channel, a 0 means no percussion hit, a 1 means percussion hit.



**Example Song Array (24 beats)**


int songMatrix[][6][2] = {

  {{706, 24},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}},
  
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{0,1}},
  
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{0,2}},
  
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{0,3}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,4}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,5}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,6}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,7}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,8}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,9}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,10}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,11}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,12}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,13}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,14}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{1,15}},
 
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,16}},
  
  {{NOTE_CS5, 1},{NOTE_G4, 1},{REST, 1},{REST, 1},{REST, 1},{1,17}},
  
  {{NOTE_C5, 1},{NOTE_FS4, 1},{REST, 1},{REST, 1},{REST, 1},{1,18}},
  
  {{NOTE_AS4, 1},{NOTE_E4, 1},{REST, 1},{REST, 1},{REST, 1},{1,19}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,20}},
  
  {{NOTE_AS2, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,21}},
  
  {{REST, 1},{REST, 1},{REST, 1},{REST, 1},{REST, 1},{0,22}},
  
  {{NOTE_CS5, 2},{NOTE_G4, 2},{REST, 1},{REST, 1},{REST, 1},{0,23}},
  
  {{SUSTAIN, NOTE_CS5},{SUSTAIN, NOTE_G4},{REST, 1},{REST, 1},{REST, 1},{0,24}}
  
  };
  
  
  
**SD TXT File Format**

The song array is stored in a TXT file on the SD card in a very similar format (except the NOTE, REST, and SUSTAIN variables are stored as literal ints).  Here is what the above array would look like in TXT format:


706,24,0,0,0,0,0,0,0,0,0,0,

554,1,392,1,0,1,0,1,0,1,0,1,

523,1,370,1,0,1,0,1,0,1,0,2,

466,1,330,1,0,1,0,1,0,1,0,3,

0,1,0,1,0,1,0,1,0,1,0,4,

117,1,0,1,0,1,0,1,0,1,1,5,

0,1,0,1,0,1,0,1,0,1,0,6,

117,1,0,1,0,1,0,1,0,1,1,7,

0,1,0,1,0,1,0,1,0,1,0,8,

117,1,0,1,0,1,0,1,0,1,1,9,

0,1,0,1,0,1,0,1,0,1,0,10,

117,1,0,1,0,1,0,1,0,1,1,11,

0,1,0,1,0,1,0,1,0,1,0,12,

117,1,0,1,0,1,0,1,0,1,1,13,

0,1,0,1,0,1,0,1,0,1,0,14,

117,1,0,1,0,1,0,1,0,1,1,15,

0,1,0,1,0,1,0,1,0,1,0,16,

554,1,392,1,0,1,0,1,0,1,1,17,

523,1,370,1,0,1,0,1,0,1,1,18,

466,1,330,1,0,1,0,1,0,1,1,19,

0,1,0,1,0,1,0,1,0,1,0,20,

117,1,0,1,0,1,0,1,0,1,0,21,

0,1,0,1,0,1,0,1,0,1,0,22,

554,2,392,2,0,1,0,1,0,1,0,23,

1,554,1,392,0,1,0,1,0,1,0,24

