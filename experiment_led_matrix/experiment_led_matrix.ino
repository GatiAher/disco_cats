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
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  Serial.begin(115200); // will change baud rate of MIDI traffic to 115200 (baud rate of ttymidi)

  digitalWrite(SPI_CS, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  //  Serial.print("h\n");
  spiTransfer(OP_DIGIT0, B00111000);
  spiTransfer(OP_DIGIT1, B01110000);
  spiTransfer(OP_DIGIT2, B11100000);
  spiTransfer(OP_DIGIT3, B01110000);
  spiTransfer(OP_DIGIT4, B00111000);
  delay(100);
  spiTransfer(OP_DIGIT0, B01010000);
  spiTransfer(OP_DIGIT1, B10101000);
  spiTransfer(OP_DIGIT2, B01010000);
  spiTransfer(OP_DIGIT3, B10101000);
  spiTransfer(OP_DIGIT4, B01010000);
  delay(100);
  spiTransfer(OP_DIGIT0, B11100000);
  spiTransfer(OP_DIGIT1, B01110000);
  spiTransfer(OP_DIGIT2, B00111000);
  spiTransfer(OP_DIGIT3, B01110000);
  spiTransfer(OP_DIGIT4, B11100000);
  delay(100);
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
