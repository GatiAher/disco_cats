/*
   MIDI File to LED lightshow
   Disco Cats
   Nov 17, 2021

  Given MIDI stream, lights correspond to pitch of notes played by instrument on channel 1
*/

/*
   Outer Loop
   Read from serial, decode command-note-velocity, update LED driver matrix
*/

#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60); //Bottom
Adafruit_MotorShield AFMS2 = Adafruit_MotorShield(0x61); //Top

// Select the ports for both motors (M1, M2, M3, M4)
Adafruit_DCMotor *motor1 = AFMS.getMotor(1);
Adafruit_DCMotor *motor2 = AFMS.getMotor(2);
Adafruit_DCMotor *motor3 = AFMS.getMotor(3);
Adafruit_DCMotor *motor4 = AFMS.getMotor(4);

Adafruit_DCMotor *motor5 = AFMS2.getMotor(1);
Adafruit_DCMotor *motor6 = AFMS2.getMotor(2);
Adafruit_DCMotor *motor7 = AFMS2.getMotor(3);
Adafruit_DCMotor *motor8 = AFMS2.getMotor(4);


const int nrOfMotors = 8;


Adafruit_DCMotor *motors[nrOfMotors] = {
  motor1, motor2, motor3, motor4, motor5, motor6, motor7, motor8,
};


int count[nrOfMotors]={100, 100, 100, 100, 100, 100, 100, 100}; //each represents 8 cats. Counts the time since the last time the channel was played. 

byte incomingByte;
byte note;
byte velocity;
int noteDown = LOW;
int state = 0; // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting
//int channel = 1; // MIDI channel to respond to (in this case channel 2) change this to play different channel
int channel; 

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


  AFMS.begin();
  AFMS2.begin();

  // test Motors
  motor1 ->setSpeed(40);
  motor1 ->run(FORWARD);
  motor4 ->setSpeed(40);
  motor4 ->run(FORWARD);
  
  // test LEDs work

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      LED_MATRIX[i][j] = 1;
    }
  }
  
  playMatrix();
  delay(1000);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
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
        //if (incomingByte == (144 | channel)) {
        if ((incomingByte & 0xf0) == 0x90){
          channel = incomingByte & 0x0f;
          noteDown = HIGH;
          state = 1;
        }
        // look for as status-byte, our channel, note off
        //if (incomingByte == (128 | channel)) {
        if ((incomingByte & 0xf0) == 0x80){ 
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
          updateMatrix(channel, note, velocity, noteDown);
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


void updateMatrix(int channel, byte note, byte velocity, int down) {
    int instrument = channel % 8;
    switch (note % 12) {
      case 0:
        // note C
      case 1:
        // note C#
        for(int i=0; i<8; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 2:
        // note D
      case 3:
        // note D#
        for(int i=0; i<2; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 4:
        // note E
        for(int i=0; i<3; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 5:
        // note F
      case 6:
        // note F#
        for(int i=0; i<4; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 7:
        // note G
      case 8:
        // note G#
        for(int i=0; i<5; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 9:
        // note A
      case 10:
        // note A#
        for(int i=0; i<6; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      case 11:
        // note B
        for(int i=0; i<7; i++){
          LED_MATRIX[instrument][i] = (down) ? 1 : 0;
        }
        break;

      default:
        // not a note -- do nothing
        break;
    }
  
}



void moveCat(int channel, byte velocity, int down){
  int cat = channel % 8; // Cat is from 0 to 7
  if(count[cat] > 0){
    count[cat] == 0; 
  }
  for (int i = 0; i < 8; i++){
    count[i]+=1;
    if(count[i] < 30){
      motors[i] ->setSpeed(40);
      motors[i] ->run(FORWARD);
    }
  }
}


void playMatrix() {
  for (int i = 0; i < 8; i++) {
    // give GND only to row i
    row_data[i] = 0;
    for (int j = 0; j < 8; j++) {
      // give VDD to all ON cells in row i
      col_data[j] = LED_MATRIX[i][j];
    }
    loadDataIntoShiftRegister();
    delay(2); // Time between switching between LEDs

    // reset
    row_data[i] = 1;
    for (int j = 0; j < 8; j++) {
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
