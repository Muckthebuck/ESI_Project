

// Include the required Wire library for I2C
#include <Wire.h>
#include <Stepper.h>
//SLAVE 
#define SLAVE 9

#define LCD 1

//stepper motor
// Number of steps per internal motor revolution 
const float STEPS_PER_REV = 32; 
 
//  Amount of Gear Reduction
const float GEAR_RED = 64;
 
// Number of steps per geared output rotation
const float STEPS_PER_OUT_REV = STEPS_PER_REV * GEAR_RED;
 
// Define Variables
 
// Number of Steps Required
int StepsRequired;
 
// Create Instance of Stepper Class
// Specify Pins used for motor coils
// The pins used are 8,9,10,11 
// Connected to ULN2003 Motor Driver In1, In2, In3, In4 
// Pins entered in sequence 1-3-2-4 for proper step sequencing
 
Stepper steppermotor(STEPS_PER_REV, 8, 10, 9, 11);

char* test = {"HELLO WORLD!"};

void setup() {
  // Start the I2C Bus as Master
  Wire.begin(); 
}

void loop() {
  Wire.beginTransmission(SLAVE); // transmit to device #9
  Wire.write("going clockwise");              // sends x 
  Wire.endTransmission(SLAVE); // stop transmitting
  
  // Rotate CW 1/2 turn slowly
  StepsRequired  =  STEPS_PER_OUT_REV / 2; 
  steppermotor.setSpeed(500);   
  steppermotor.step(StepsRequired);
  delay(10000);
  
  Wire.beginTransmission(SLAVE); // transmit to device #9
  Wire.write("going anticlockwise");              // sends x 
  Wire.endTransmission(SLAVE);
  // Rotate CCW 1/2 turn quickly
  StepsRequired  =  - STEPS_PER_OUT_REV / 2;   
  steppermotor.setSpeed(700);  
  steppermotor.step(StepsRequired);
  delay(2000);
}
