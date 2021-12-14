// Include the AccelStepper library:
#include <AccelStepper.h>

// Motor pin definitions:
//#define motor1Pin1  11      // IN1 on the ULN2003 driver
//#define motor1Pin2  10      // IN2 on the ULN2003 driver
//#define motor1Pin3  9     // IN3 on the ULN2003 driver
//#define motor1Pin4  8     // IN4 on the ULN2003 driver
//
//#define motor2Pin1  7      // IN1 on the ULN2003 driver
//#define motor2Pin2  6      // IN2 on the ULN2003 driver
//#define motor2Pin3  5     // IN3 on the ULN2003 driver
//#define motor2Pin4  4     // IN4 on the ULN2003 driver
//
//#define motor3Pin1  14      // IN1 on the ULN2003 driver
//#define motor3Pin2  15      // IN2 on the ULN2003 driver
//#define motor3Pin3  16     // IN3 on the ULN2003 driver
//#define motor3Pin4  17     // IN4 on the ULN2003 driver


uint8_t MotorPins[][10] = {
  {11, 10, 9, 8}, {7, 6, 5, 4}, {14, 15, 16, 17}, {18, 19, 20, 21},
  {22, 24, 26, 28}, {23, 25, 27, 29}, {30, 32, 34, 36}, {31, 33, 35, 37},
  {42, 44, 46, 48}
};



// Define the AccelStepper interface type; 4 wire motor in half step mode:
#define MotorInterfaceType 8


AccelStepper *stepperPtrArray[10];

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
//AccelStepper stepper1 = AccelStepper(MotorInterfaceType, motor1Pin1, motor1Pin3, motor1Pin2, motor1Pin4);
//AccelStepper stepper2 = AccelStepper(MotorInterfaceType, motor2Pin1, motor2Pin3, motor2Pin2, motor2Pin4);
//AccelStepper stepper3 = AccelStepper(MotorInterfaceType, motor3Pin1, motor3Pin3, motor3Pin2, motor3Pin4);



void setup() {
  Serial.begin(115200);
//
//  stepperPtrArray[0] = new AccelStepper(4, motor1Pin1, motor1Pin2, motor1Pin3, motor1Pin4);
//  stepperPtrArray[1] = new AccelStepper(4, motor2Pin1, motor2Pin2, motor2Pin3, motor2Pin4);
//  // Set the maximum steps per second:
//  stepperPtrArray[0] -> setMaxSpeed(1000);
//  stepperPtrArray[1] -> setMaxSpeed(1000);
//  stepper2.setMaxSpeed(1000);
//  stepper3.setMaxSpeed(1000);

  for (uint8_t x = 0; x < 9; x++)  {
    stepperPtrArray[x] = new AccelStepper(4, MotorPins[x][0], MotorPins[x][1], MotorPins[x][2], MotorPins[x][3]);
    stepperPtrArray[x]->setMaxSpeed(1000);
  }
  
//  stepperPtrArray[1] -> setSpeed(500);
//  stepperPtrArray[1] -> setCurrentPosition(0);;
//  
//  while(stepperPtrArray[1] -> currentPosition() < 50){
//    stepperPtrArray[1] -> runSpeed();
//  }
}


void loop() {
    
  // Set the speed of the motor in steps per second:
  for (uint8_t x = 0; x < 9; x++)  {
    stepperPtrArray[x] -> setSpeed(600);
  }
//  stepper2.setSpeed(500);
//  stepper3.setSpeed(500);
  
  // Step the motor with constant speed as set by setSpeed():
  for (uint8_t x = 0; x < 9; x++)  {
    stepperPtrArray[x] -> runSpeed();
    Serial.println(x);
    delay(0);
  }

//  stepper2.runSpeed();
//  stepper3.runSpeed();
}
