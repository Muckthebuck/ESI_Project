

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
  LCD_print("Hello","Mukul");
}

String format_string_for_print(String str){
  // Formats a string to be exactly 16 characters
  int str_length = str.length();
  if (str_length>16){
    return "String Too Long";
    // Invalid input
  }
  
  String temp_str= "";
  
  for (int i=0; i<str_length; i++){
    temp_str.concat(str[i]);
  }
  for (int i=str_length; i<16; i++){
    temp_str.concat(" ");
  }
  return temp_str;
}

void LCD_print(String top_message, String bottom_message){
  String top = format_string_for_print(top_message);
  String bottom = format_string_for_print(bottom_message);
  Wire.beginTransmission(SLAVE); // transmit to device #9
  for(int i=0; i<32; i++){
    Wire.write((top+bottom)[i]);
  }
  Wire.endTransmission(SLAVE);
}

