
// I2C Slave

// Include the required Wire library for I2C
#include <Wire.h>
#include <LiquidCrystal.h>
//commands
#define LIGHTSOFF 60 
#define LIGHTSON  62
int lights=1;

//8by8 array
#define DATA 2
#define SHIFT 3
#define STORE 4

#define SMILE 43
#define SAD 44
#define BORED 45
#define QN 46

const int columns = 8;
void animation(int movie[][columns]);
void store();
const int frames = 4;
const int smile[4][8] = {{0,36,36,36,0,66,60,0},
                 {0, 0,36,0, 0, 0,24,0},
                 {0, 0,36,0, 0, 0,60,0},
                 {0,36,36,36,0,66,60,0}};
const int sad[4][8] = {{0, 0,36,0, 0, 0,24,0},
                      {0, 0,36,0, 0, 0,24,0},
                      {0, 0,36,0, 0, 0,60,0},
                      {0, 0,102,0, 0, 0,60,66}};
                               
const int bored[4][8] = {{0, 0,36,0, 0, 0,24,0},
                         {0, 0,102,0, 0, 0,24,0},
                         {0, 0,102,0, 0, 0,60,0},
                         {0, 0,102,0, 0, 0,126,0}}; 

const int question[4][8]={{28, 34,2,12, 8, 0,0,0},
                         {28, 34,2,12, 8, 0,8,0},
                         {28, 34,2,12, 8, 0,0,0},
                         {28, 34,2,12, 8, 0,8,0}};              

const int LED = 13;
String x = "";
char buff[16];
int event;
char data[32];
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
  //8by8 led matrix
   pinMode(DATA, OUTPUT);
   pinMode(SHIFT, OUTPUT);
   pinMode(STORE, OUTPUT);
  //lcd
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
 // Serial.println(x);
}

void loop() {
   //lcd.clear();
  if(event==1){
    if(x[0]==LIGHTSOFF){
      lights=0;
      turn_off();
    }else if(x[0]==LIGHTSON){ 
      lights=1;
      turn_on();
    }else if(x[0]==SMILE||x[0]==SAD||x[0]==BORED||x[0]==QN){
      // Serial.println(x);
      if(x[0] == SMILE){
        animation(smile);
      }else if(x[0] == SAD){
       animation(sad);
      }else if(x[0] == BORED){
       animation(bored);
      }else if(x[0]== QN){
       animation(question);
      } 
    }else{
      local_LCD_display(x);
    }
  }
   
//  if(lights){
//    animation(smile);
//   }
 // delay(1000);
 // lcd.clear(); 
}

void turn_on(){
  digitalWrite(lcd_on, HIGH);
}

void turn_off(){
  digitalWrite(lcd_on, LOW);
}

void local_LCD_display(String message){
   //lcd.clear(); 
   
   message.toCharArray(data, 33);
  // Serial.print(" inside lcd function ");
   //Serial.println(data);
   int i;
   Serial.println(data);
    for(i=0;i<16;i++){
      lcd.setCursor(i,0);
      lcd.write(data[i]);
    }   
    for(i=16;i<32;i++){
      lcd.setCursor(i-16,1);
      lcd.write(data[i]);
    }
}

void change_animation(String x){
  
}

void animation(const int movie[][columns]){
  for(int j=0;j<frames;j++){
    for(int k=0;k<1000;k++){
      for (int i=0; i<8; i++) {
       shiftOut(DATA, SHIFT, LSBFIRST, ~movie[j][i]);
       shiftOut(DATA, SHIFT, LSBFIRST, 128 >> i);
       store();
     }
    } 
  }
}

void store() {
  digitalWrite(STORE, HIGH);
  delayMicroseconds(10);
  digitalWrite(STORE, LOW);
  delayMicroseconds(10);
}
