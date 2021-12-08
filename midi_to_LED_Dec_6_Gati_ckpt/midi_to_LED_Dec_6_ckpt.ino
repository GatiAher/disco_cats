/*
   MIDI File to LED lightshow
   Gati Aher
   Nov 11, 2021

  Use the RBG LEDs
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
//int channel; // Any channel


int LED_MATRIX_R[ 8 ][ 8 ] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 1
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 2
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 3
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 4
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 5
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 6
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 7
  {0, 0, 0, 0, 0, 0, 0, 0}  // Instrument 8
};


int LED_MATRIX_G[ 8 ][ 8 ] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 1
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 2
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 3
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 4
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 5
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 6
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 7
  {0, 0, 0, 0, 0, 0, 0, 0}  // Instrument 8
};


int LED_MATRIX_B[ 8 ][ 8 ] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 1
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 2
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 3
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 4
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 5
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 6
  {0, 0, 0, 0, 0, 0, 0, 0}, // Instrument 7
  {0, 0, 0, 0, 0, 0, 0, 0}  // Instrument 8
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
  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (baud rate of ttymidi)
  Serial.println("Start SetUp");

  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
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
  spiTransfer(OP_INTENSITY, 15);
  /* Clear the display */
  for (int i = 0; i < 8; i++) {
    spiTransfer(i + 1, 0);
    delay(100);
  }

  //
  //  spiTransfer(2, 170);
  //  delay(500);
  //  spiTransfer(2, 85);
  //  delay(500);
  //  spiTransfer(2, 170);
  //  delay(500);
  //  //  spiTransfer(1, 0);
  //  //  delay(500);
  //  delay(10000);


  Serial.println("End SetUp");
}


void loop() {
  spiTransfer(1, 0);
  delay(500);
  spiTransfer(2, 0);
  delay(500);
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
