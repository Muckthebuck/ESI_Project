

// Include the required Wire library for I2C
#include <Wire.h>
//SLAVE 
#define SLAVE 9

#define LCD 1


char* test = {"HELLO WORLD!"};

void setup() {
  // Start the I2C Bus as Master
  Wire.begin(); 
}

void loop() {
  Wire.beginTransmission(SLAVE); // transmit to device #9
  Wire.write("BAAL");              // sends x 
  Wire.endTransmission(SLAVE);    // stop transmitting
  delay(3000);
  Wire.beginTransmission(SLAVE); // transmit to device #9
  Wire.write("your MAMA");              // sends x 
  Wire.endTransmission(SLAVE);
  delay(10000);
}
