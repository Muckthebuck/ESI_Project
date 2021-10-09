// Chapter 7 - Communications
// I2C Slave
// By Cornel Amariei for Packt Publishing

// Include the required Wire library for I2C
#include <Wire.h>
#include <LiquidCrystal.h>

const int LED = 13;
String x = "";
char data[16];
char buff[16];
int event;
/*
 * pin layout
 * 7 - A, to turn on backlight
 * 8 - RS pin
 * 9 - Enable pin
 * 10 - D4
 * 11 - D5
 * 12 - D6
 * 13 - D7
 */

//initialise LCD library
const int rs = 8, en =9, d4 = 10, d5=11, d6=12, d7=13, lcd_on=7;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

byte Heart[8] = {
0b00000,
0b01010,
0b11111,
0b11111,
0b01110,
0b00100,
0b00000,
0b00000
};

void setup() {
  // Define the LED pin as Output
  Serial.begin(9600);
  pinMode(lcd_on, OUTPUT);
  lcd.begin(16,2);
  delay (500);
  lcd.createChar(0, Heart);
  lcd.clear();
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  event=0;
  Wire.onReceive(receiveEvent);
  turn_on();
}

void receiveEvent(int bytes) {
  x ="";
  while( Wire.available()){
    lcd.setCursor(0,0);
   // sprintf(buff,"recieved");
    x += (char)Wire.read(); // read from the I2C
  } 
  event=1;
  //Serial.print(event);
}

void loop() {
   lcd.clear();
   //Serial.println(x);
  if(event==1){
    local_LCD_display(x);
  }
  
  delay(1000);
  lcd.clear();
}

void turn_on(){
  digitalWrite(lcd_on, HIGH);
}

void local_LCD_display(String message){
   lcd.clear(); 
   char data[32];
   message.toCharArray(data, 33);
   int i;
   
    for(i=0;i<16;i++){
      lcd.setCursor(i,0);
      lcd.write(data[i]);
    }   
    for(i=16;i<32;i++){
      lcd.setCursor(i-16,1);
      lcd.write(data[i]);
    }
}
