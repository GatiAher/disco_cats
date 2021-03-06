/*
   MIDI File to LED lightshow
   Disco Cats
   Nov 11, 2021

  Use the MAX7219 chip (it has a latch signal so there is no flicker and rows are all equally bright)

  Given MIDI stream, lights correspond to pitch of notes played by instrument on channel 1

  0. Initialize inputs and outputs, MAX7219 config, begin serial, and test that LEDs work
  1. Read and decode MIDI message with a state machine
  2. Update matrix state
  3. Shift data onto hardware MAX7219 chip, 16-bit shift register with SPI of 4 wires (GND, CLK, CS, DIN)
*/

byte incomingByte;
byte note;
byte velocity;
int noteDown = LOW;
int channel = 1; // MIDI channel to respond to (in this case channel 2) change this to play different channel
//
//int LED_MATRIX[ 5 ][ 5 ] = {
//  {0, 0, 0, 0, 0},
//  {0, 0, 0, 0, 0},
//  {0, 0, 0, 0, 0},
//  {0, 0, 0, 0, 0},
//  {0, 0, 0, 0, 0}
//};

int LED_MATRIX[ 5 ][ 5 ] = {
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
};

// MIDI decoder state machine
int state = 0; // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting


//SPI pins for MAX7219
#define SPI_CLK 11 // CLOCK (on positive edge of CLK, copy data across shift register)  
#define SPI_CS 10 // LOAD (on positive edge of CS, copy shift register data to LED driver)
#define SPI_MOSI 12 // Master Output Seconday Input (data signal, MAX7219 is a secondary device)

//opcodes for MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

void setup() {
  /*
     Initialize inputs and outputs, MAX7219 config, begin serial, and test that LEDs work
  */
  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (baud rate of ttymidi)
  state = 0;

  // configure MAX7219
  digitalWrite(SPI_CS, HIGH);
  /* Turn off display test mode */
  spiTransfer(OP_DISPLAYTEST, 0);
  /* Set scanlimit is set to max */
  spiTransfer(OP_SCANLIMIT, 7);
  /* Set no-decode mode */
  spiTransfer(OP_DECODEMODE, 0);
  /* Turn off power-saving mode on startup */
  spiTransfer(OP_SHUTDOWN, 1);
  /* Set the brightness to a medium values */
  spiTransfer(OP_INTENSITY, 8);
  /* Clear the display */
  for (int i = 0; i < 8; i++) {
    spiTransfer(i + 1, 0);
  }

  // test
  playMatrix();
  delay(5000);



  // TODO: error when more than one LED on per row; perhaps because RSET shoudl be 120k Ohm but it 120 Ohm?
  //  // test: turn all LEDs ON
  //  for (int i = 0; i < 5; i++) {
  //    for (int j = 0; j < 5; j++) {
  //      LED_MATRIX[i][j] = 1;
  //    }
  //  }
  //  playMatrix();
  //  delay(5000);
  //
  //  // test: turn all LEDs OFF
  //  for (int i = 0; i < 5; i++) {
  //    for (int j = 0; j < 5; j++) {
  //      LED_MATRIX[i][j] = 0;
  //    }
  //  }
  //  playMatrix();
}


void loop() {
  /*
     Read and decode MIDI message with a state machine
  */
  if (Serial.available() > 0) {
    // read the incoming byte
    incomingByte = Serial.read();
    switch (state) {
      case 0:
        // look for as status-byte, our channel, note on
        if (incomingByte == (144 | channel)) {
          noteDown = HIGH;
          state = 1;
        }
        // look for as status-byte, our channel, note off
        if (incomingByte == (128 | channel)) {
          noteDown = LOW;
          state = 1;
        }
        break;

      case 1:
        // get the note to play or stop
        if (incomingByte < 128) {
          note = incomingByte;
          state = 2;
        }
        else {
          state = 0;  // reset state machine as this should be a note number
        }
        break;

      case 2:
        // get the velocity
        if (incomingByte < 128) {
          velocity = incomingByte;

          // if velocity = 0, treat as note off command
          if (velocity == 0) {
            noteDown = LOW;
          }
          updateMatrix(note, velocity, noteDown);
        }
        playMatrix();
        state = 0;  // reset state machine
        break;

      default:
        state = 0; // reset state machine
        break;
    }
  }
}


void updateMatrix(byte note, byte velocity, int down) {
  /*
     Update matrix state
  */
  if (note >= 0 && note <= 59) {
    // upper octaves
    switch (note % 12) {
      case 0:
        // note C
        LED_MATRIX[0][0] = (down) ? 1 : 0;
        break;

      case 1:
        // note C#
        LED_MATRIX[0][1] = (down) ? 1 : 0;
        break;

      case 2:
        // note D
        LED_MATRIX[0][2] = (down) ? 1 : 0;
        break;

      case 3:
        // note D#
        LED_MATRIX[0][3] = (down) ? 1 : 0;
        break;

      case 4:
        // note E
        LED_MATRIX[0][4] = (down) ? 1 : 0;
        break;

      case 5:
        // note E#
        LED_MATRIX[1][0] = (down) ? 1 : 0;
        break;

      case 6:
        // note F
        LED_MATRIX[1][1] = (down) ? 1 : 0;
        break;

      case 7:
        // note F#
        LED_MATRIX[1][2] = (down) ? 1 : 0;
        break;

      case 8:
        // note G
        LED_MATRIX[1][3] = (down) ? 1 : 0;
        break;

      case 9:
        // note G#
        LED_MATRIX[1][4] = (down) ? 1 : 0;
        break;

      case 10:
        // note A
        LED_MATRIX[2][0] = (down) ? 1 : 0;
        break;

      case 11:
        // note A#
        LED_MATRIX[2][1] = (down) ? 1 : 0;
        break;

      default:
        // not a note -- do nothing
        break;
    }
  }
  else {
    // lower octaves
    switch (note % 12) {
      case 0:
        // note C
        LED_MATRIX[2][3] = (down) ? 1 : 0;
        break;

      case 1:
        // note C#
        LED_MATRIX[2][4] = (down) ? 1 : 0;
        break;

      case 2:
        // note D
        LED_MATRIX[3][0] = (down) ? 1 : 0;
        break;

      case 3:
        // note D#
        LED_MATRIX[3][1] = (down) ? 1 : 0;
        break;

      case 4:
        // note E
        LED_MATRIX[3][2] = (down) ? 1 : 0;
        break;

      case 5:
        // note E#
        LED_MATRIX[3][3] = (down) ? 1 : 0;
        break;

      case 6:
        // note F
        LED_MATRIX[3][4] = (down) ? 1 : 0;
        break;

      case 7:
        // note F#
        LED_MATRIX[4][0] = (down) ? 1 : 0;
        break;

      case 8:
        // note G
        LED_MATRIX[4][1] = (down) ? 1 : 0;
        break;

      case 9:
        // note G#
        LED_MATRIX[4][2] = (down) ? 1 : 0;
        break;

      case 10:
        // note A
        LED_MATRIX[4][3] = (down) ? 1 : 0;
        break;

      case 11:
        // note A#
        LED_MATRIX[4][4] = (down) ? 1 : 0;
        break;

      default:
        // not a note -- do nothing
        break;
    }
  }
}


void playMatrix() {
  /*
     Shift data onto hardware
     For each row of matrix:
     1. Determine opcode (8 bits, last 4 bits are MAX7219 opcode)
     2. Determine data (8 bits, cells of row to light up)
  */
  byte opcode = 0;
  byte data = 0;

  // determine OP_DIGITX (row to give GND to)
  for (int i = 0; i < 5; i++) {
    opcode = i + 1;

    // determine data value from LED_MATRIX cells for given row
    for (int j = 0; j < 8; j++) {
      data *= 2; // double result so far
      if (j < 5 and LED_MATRIX[i][j]) data++; // add 1 if needed
    }

    spiTransfer(opcode, data);
    delay(2); // Time to hold on row before continuing
  }
}



void spiTransfer(volatile byte opcode, volatile byte data) {
  /*
    Load row data and col data into 8-bit shift register
    On positive edge of CLK, copy row and col data across shift register
    On positive edge of CS, copy shift register data to LED driver
  */
  digitalWrite(SPI_CS, LOW);
  // put opcode byte into shift register
  shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, opcode);
  // put data byte into shift register
  shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, data);
  digitalWrite(SPI_CS, HIGH);
}
