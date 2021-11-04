/*
   MIDI File to LED lightshow
   Gati Aher
   Oct 26, 2021

  Given MIDI stream, lights correspond to pitch of notes played by instrument on channel 1
*/

/*
   Outer Loop
   Read from serial, decode command-note-velocity, update LED driver matrix
*/

byte incomingByte;
byte note;
byte velocity;
int noteDown = LOW;
int state = 0; // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting
int channel = 1; // MIDI channel to respond to (in this case channel 2) change this to play different channel

int LED_MATRIX[ 5 ][ 5 ] = {
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
};

/*
   Inner Loop
   Calculate row and columns from LED driver matrix
   Shift register through columns
*/

#define clk_pin 6 // clock  
#define col_data_pin 5 // data
#define row_data_pin 4 // data

// if there is a voltage difference (column 1, row 0) then LED lights up
int row_data[8] = {1, 1, 1, 1, 1, 1, 1, 1}; //reversed order (supplies GND to full row)
int col_data[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //reversed order (supplies VDD to cells of row)

/*
   State Machine
*/

// setup: declaring iputs and outputs and begin serial
void setup() {
  pinMode(clk_pin, OUTPUT);
  pinMode(row_data_pin, OUTPUT);
  pinMode(col_data_pin, OUTPUT);

  state = 0;
  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (default baud rate of ttymidi)

  // test LEDs work

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      LED_MATRIX[i][j] = 1;
    }
  }

  playMatrix();
  delay(1000);

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      LED_MATRIX[i][j] = 0;
    }
  }
  playMatrix();
}


// loop: wait for serial data, and interpret the message
void loop() {
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
        state = 0;  // reset state machine
        playMatrix();
        break;

      default:
        state = 0; // reset state machine
        break;
    }
  }
}


void updateMatrix(byte note, byte velocity, int down) {
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
  for (int i = 0; i < 5; i++) {
    // give GND only to row i
    row_data[i] = 0;
    for (int j = 0; j < 5; j++) {
      // give VDD to all ON cells in row i
      col_data[j] = LED_MATRIX[i][j];
    }
    loadDataIntoShiftRegister();
    delay(2); // Time between switching between LEDs

    // reset
    row_data[i] = 1;
    for (int j = 0; j < 5; j++) {
      col_data[j] = 0;
    }

  }
}


void loadDataIntoShiftRegister() {
  /*
     Load row data and col data into 8-bit shift register
  */
  // TODO: latch signal so LED isn't on before shift register reaches final position
  for (int k = 0; k < 8; k++) {
    // write each value in array
    digitalWrite(row_data_pin, row_data[k]);
    digitalWrite(col_data_pin, col_data[k]);
    // pulse clock to copy value to next location (shift register)
    byte z = digitalRead(clk_pin);
    digitalWrite(clk_pin, !z);
    digitalWrite(clk_pin, z);
  }
}
