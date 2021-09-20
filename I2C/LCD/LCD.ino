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

#include <LiquidCrystal.h>

//initialise library
const int rs = 8, en =9, d4 = 10, d5=11, d6=12, d7=13, lcd_on=7;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

void setup() {
  // put your setup code here, to run once:
  pinMode(lcd_on, OUTPUT);
  lcd.begin(16,2);
  turn_on();
  lcd.print("hello, world");
}

void loop() {
  // put your main code here, to run repeatedly:
  turn_on();
  lcd.setCursor(0,1);
  lcd.print(millis()/1000);
  lcd.setCursor(0,0);
  delay(2000);
  lcd.print("Display OFF");
  turn_off();
  delay(1000);
}

void turn_on(){
  digitalWrite(lcd_on, HIGH);
}
void turn_off(){
  digitalWrite(lcd_on,LOW);
}
