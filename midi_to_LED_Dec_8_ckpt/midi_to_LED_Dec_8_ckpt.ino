/*
   MIDI File to LED lightshow
   Gati Aher
   Dec 8, 2021
  Use the MAX7219 chip (it has a latch signal so there is no flicker and rows are all equally bright)
  Given MIDI stream, lights correspond to pitch of notes played by instrument on channel 1
  0. Initialize inputs and outputs, MAX7219 config, begin serial, and test that LEDs work
  1. Read and decode MIDI message with a state machine
  2. Update matrix state
  3. Shift data onto hardware MAX7219 chip, 16-bit shift register with SPI of 4 wires (GND, CLK, CS, DIN)
*/

///////////////
// MIDI VARS //
///////////////

byte incomingByte;
byte note;
byte velocity;
int noteDown = LOW;
int channel = 1; // MIDI channel to respond to (in this case channel 2) change this to play different channel

// MIDI decoder state machine
int state = 0; // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting

/////////////////////////
// LED + MAX CHIP VARS //
/////////////////////////

int LED_MATRIX[ 8 ][ 8 ] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 1
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 2
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 3
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 4
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 5
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 6
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 7
  {0, 0, 0, 0, 0, 0, 0, 0}  // Instrument 8
};


//SPI pins for MAX7219
//#define SPI_CLK 39 // CLOCK (on positive edge of CLK, copy data across shift register)  
//#define SPI_CS 41 // LOAD (on positive edge of CS, copy shift register data to LED driver)
//#define SPI_MOSI 43 // Master Output Seconday Input (data signal, MAX7219 is a secondary device)

#define SPI_CLK 10 // CLOCK (on positive edge of CLK, copy data across shift register)  
#define SPI_CS 11 // LOAD (on positive edge of CS, copy shift register data to LED driver)
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


////////////////
// MOTOR VARS //
////////////////

#include <AccelStepper.h>

//Array for IO pin number for each stepper motor
uint8_t MotorPins[][10] = {
  {11, 10, 9, 8}, {7, 6, 5, 4}, {14, 15, 16, 17}, {18, 19, 20, 21},
  {22, 24, 26, 28}, {23, 25, 27, 29}, {30, 32, 34, 36}, {31, 33, 35, 37},
  {42, 44, 46, 48}
};

// Define the AccelStepper interface type; 4 wire motor in half step mode:
#define MotorInterfaceType 4

const int nrOfMotors = 9;

AccelStepper *stepperPtrArray[nrOfMotors];

//each represents 8 motors. Counts the time since the last time the channel was played.
int count[nrOfMotors] = {100, 100, 100, 100, 100, 100, 100, 100};

int songEnd = 0;

void setup() {
  /*
     Initialize inputs and outputs, MAX7219 config, begin serial, and test that LEDs work
  */

  ////////////////
  // MIDI SETUP //
  ////////////////

  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (baud rate of ttymidi)
  state = 0;

  //////////////////////////
  // LED + MAX CHIP SETUP //
  //////////////////////////

  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);

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
  spiTransfer(OP_INTENSITY, 1);
  /* Clear the display */
  for (int i = 0; i < 8; i++) {
    spiTransfer(i + 1, 0);
  }

  // test
  playMatrix();
  delay(500);

  // test: turn all LEDs OFF
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      LED_MATRIX[i][j] = 0;
    }
  }
  playMatrix();

  //TODO: error when more than one LED on per row; perhaps because RSET shoudl be 120k Ohm but it 120 Ohm ?
  // test: turn all LEDs ON
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      LED_MATRIX[i][j] = 1;
      playMatrix();
      LED_MATRIX[i][j] = 0;
      delay(500);
    }
  }
  delay(100);

  // test: turn all LEDs OFF
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      LED_MATRIX[i][j] = 0;
    }
  }
  playMatrix();
  delay(100);

  /////////////////
  // MOTOR SETUP //
  /////////////////
  // connect and configure the stepper motors to their IO pins
  for (int x = 0; x < nrOfMotors; x++)  {
    stepperPtrArray[x] = new AccelStepper(4, MotorPins[x][0], MotorPins[x][1], MotorPins[x][2], MotorPins[x][3]);
    stepperPtrArray[x]->setMaxSpeed(1000);
    stepperPtrArray[x]->setSpeed(0);
  }
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

        //        look for as status - byte, our channel, note on
        //        if (incomingByte == (144 | channel)) {
        // check for NOTE_ON
        if ((incomingByte & 0xf0) == 0x90) {
          // take last 4 bits
          channel = incomingByte & 0x0f;
          noteDown = HIGH;
          state = 1;
        }

        //        look for as status - byte, our channel, note off
        //        if (incomingByte == (128 | channel)) {

        // check for NOTE_OFF
        if ((incomingByte & 0xf0) == 0x80) {
          // take last 4 bits
          channel = incomingByte & 0x0f;
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
          updateVisualizer(channel, note, velocity, noteDown);
        }
        playMatrix();
        state = 0;  // reset state machine
        break;

      default:
        state = 0; // reset state machine
        break;
    }
  }

  for (int i = 0; i < nrOfMotors; i++) {
    stepperPtrArray[i] -> runSpeed();
  }

  // check if song is over by checking if all LEDs are off
  // for a duration of time
  int is_all_off = 1;
  for (int i = 0; i < 8; i++) {
    if (is_all_off == 1) {
      for (int j = 0; j < 8; j++) {
        if (LED_MATRIX[i][j]) {
          is_all_off = 0;
          break;
        };
      }
    }
  }
  if (is_all_off == 1) {
    songEnd += 1;
  } else {
    songEnd = 0;
  }
  if (songEnd > 30) {
    for (int i = 0; i < nrOfMotors; i++) {
      stepperPtrArray[i] -> setSpeed(0);
    }
  }
}


void updateMotor(int instrument, byte velocity) {
  if (count[instrument] > 0) {
    count[instrument] = 0;
  }

  for (int i = 0; i < 8; i++) {
    count[i] = count[i] + 1;

    //if this channel (instrument) has been played
    if (count[i] < 30) {
      stepperPtrArray[i] -> setSpeed(600);
      if (i == 7) {
        stepperPtrArray[i + 1] -> setSpeed(-600);
      }
    }
    //if this channel (instrument) hasn't been played
    else {
      stepperPtrArray[i] -> setSpeed(0);
      if (i == 7) {
        stepperPtrArray[i + 1] -> setSpeed(0);
      }
    }
  }
}


void updateVisualizer(int channel, byte note, byte velocity, int down) {
  //instrument is from 0 to 7
  int instrument = channel % 8;

  updateMotor(instrument, velocity);

  switch (note % 12) {
    case 0:
    // note C
    case 1:
      // note C#
      for (int i = 0; i < 8; i++) {
        LED_MATRIX[instrument][7] = (down) ? 1 : 0;
      }
      break;

    case 2:
    // note D
    case 3:
      // note D#
      for (int i = 0; i < 2; i++) {
        LED_MATRIX[instrument][1] = (down) ? 1 : 0;
      }
      break;

    case 4:
      // note E
      for (int i = 0; i < 3; i++) {
        LED_MATRIX[instrument][2] = (down) ? 1 : 0;
      }
      break;

    case 5:
    // note F
    case 6:
      // note F#
      for (int i = 0; i < 4; i++) {
        LED_MATRIX[instrument][3] = (down) ? 1 : 0;
      }
      break;

    case 7:
    // note G
    case 8:
      // note G#
      for (int i = 0; i < 5; i++) {
        LED_MATRIX[instrument][4] = (down) ? 1 : 0;
      }
      break;

    case 9:
    // note A
    case 10:
      // note A#
      for (int i = 0; i < 6; i++) {
        LED_MATRIX[instrument][5] = (down) ? 1 : 0;
      }
      break;

    case 11:
      // note B
      for (int i = 0; i < 7; i++) {
        LED_MATRIX[instrument][6] = (down) ? 1 : 0;
      }
      break;

    default:
      // not a note -- do nothing
      break;
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
  for (int i = 0; i < 8; i++) {
    opcode = i + 1;

    // determine data value from LED_MATRIX cells for given row
    for (int j = 0; j < 8; j++) {
      data *= 2; // double result so far
      if (LED_MATRIX[i][j]) data++; // add 1 if needed
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
