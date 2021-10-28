/*
   MIDI File to LED lightshow
   Gati Aher
   Oct 26, 2021
*/

// possible commands
const byte NOTE_OFF = 128;
const byte NOTE_ON = 144;
const byte POLY_PRESSURE = 160;
const byte CONTROL_CHANGE = 176;
const byte PROGRAM_CHANGE = 192;
const byte AFTERTOUCH = 208;
const byte PITCH_BEND = 224;

// state variables
int state = 0; // state machine variable
// 0 = command channel waiting
// 1 = data waiting
// 2 = velocity waiting

byte incomingByte;

byte currentCommand;
byte currentChannel;
byte currentNote;
byte currentVelocity;
int noteDown = LOW;

// display
int LED1 = 12;
int LED2 = 11;
int LED3 = 10;
int LED4 = 9;
int LED5 = 8;
int LED6 = 7;
int LED7 = 6;
int LED8 = 5;

// setup: declaring iputs and outputs and begin serial
void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);
  state = 0;

  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (default baud rate of ttymidi)

  // test LEDs work
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH);
  digitalWrite(LED8, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  digitalWrite(LED6, LOW);
  digitalWrite(LED7, LOW);
  digitalWrite(LED8, LOW);
}

// loop: wait for serial data, and interpret the message
void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    switch (state) {

      case 0:
        // identify command (most significant 4 bits)
        currentCommand = (incomingByte & 240);
        // identify channel (least significant 4 bits)
        currentChannel = (incomingByte & 15);

        if (currentCommand == NOTE_ON) {
          state = 1;
          noteDown = HIGH;
        }

        if (currentCommand == NOTE_OFF) {
          state = 1;
          noteDown = LOW;
        }

        if (currentCommand == PROGRAM_CHANGE) {
          state = 1;
        }


      case 1:
        // validate: data byte should have MSB 0
        if (incomingByte >= 128) {
          state = 0;  // reset state machine as this should be a note number
          break;
        }

        if ((currentCommand == NOTE_ON) | (currentCommand == NOTE_OFF)) {
          note = incomingByte;
          state = 2;
          break;
        }

      case 2:
        // validate: data byte should have MSB 0
        if (incomingByte >= 128) {
          state = 0;  // reset state machine to start
          break;
        }

        playNote(note, incomingByte, noteDown); // fire LED

    }
  }
}


void playNote(byte note, byte velocity, int down) {

  // if velocity = 0 on a 'Note ON' command, treat it as a note off
  if ((down == HIGH) && (velocity == 0)) {
    down = LOW;
  }

  if (note == 65) {
    digitalWrite(LED1, down);
  }
  if (note == 66) {
    digitalWrite(LED2, down);
  }
  if (note == 67) {
    digitalWrite(LED3, down);
  }
  if (note == 68) {
    digitalWrite(LED4, down);
  }
  if (note == 69) {
    digitalWrite(LED5, down);
  }
  if (note == 70) {
    digitalWrite(LED6, down);
  }
  if (note == 71) {
    digitalWrite(LED7, down);
  }
  if (note == 72) {
    digitalWrite(LED8, down);
  }
}
