/*
   MIDI File to LED lightshow
   Gati Aher
   Oct 26, 2021
*/

byte incomingByte;
byte note;
byte velocity;
int noteDown = LOW;
int state = 0; // state machine variable 0 = command waiting : 1 = note waiting : 2 = velocity waiting
int channel = 1; // MIDI channel to respond to (in this case channel 2) chnage this to change the channel number

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
            playNote(note, incomingByte, noteDown); // fire LED
          }
          state = 0;  // reset state machine to start
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
