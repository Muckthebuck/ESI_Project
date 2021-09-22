#include <LiquidCrystal.h>

#include <virtuabotixRTC.h>

// CLK 3, DAT 2, Reset 1
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// DS1302 controller
virtuabotixRTC myRTC(13, 12, 11); 

#define BL_LCD 10
#define PIR_PIN 2

int pir_status = 0;
int reminder_timer = 0;
int reminder_count = 0;
int reminder_subcount = 0;
int reminder_hour = 100; //Set to 100 so we can reassign on startup
int butt;
int study_status = 0; // 0 is not studying, 1 is studying
int study_countdown = 0;
int study_loading = 0;
int pomodoro_timer = 0;
int pomo_mins;
int pomo_secs;

void setup() {
  // Serial and lcd screen initilisation 
  Serial.begin(9600);
  lcd.begin(16,2);
 
  pinMode(BL_LCD, OUTPUT);
  //pinMode(PIR_PIN, INPUT);
  // Sets time time in sec, min, hour, day of week, day, mon, year
  myRTC.setDS1302Time(30, 00, 9, 2, 20, 9, 2021); 
}

void loop() {
  lcd.clear();

  // Analog buttons 
  butt = analogRead(0);
  
  // Update the time on the variables
  myRTC.updateTime();

  // Motion sensor
  pir_status = digitalRead(PIR_PIN);

  // if motion detected // trouble shooting
  if (pir_status == HIGH && study_status == 0) {            
    Serial.println("Motion detected");
  } 
  
  // Time of day function - brightness control
  bl_lcd_control(myRTC.hours);
  
  // Serial print date elements / trouble shooting
  Serial.print("Current Date / Time: ");
  Serial.print(myRTC.dayofmonth); 
  Serial.print("/");
  Serial.print(myRTC.month);
  Serial.print("/");
  Serial.print(myRTC.year);
  Serial.print(" ");
  Serial.print(myRTC.hours);
  Serial.print(":");
  Serial.print(myRTC.minutes);
  Serial.print(":");
  Serial.println(myRTC.seconds);
  Serial.print("count:");
  Serial.print(reminder_count);
  Serial.print(",subcount:");
  Serial.print(reminder_subcount);
  Serial.print(",hour of reminder:");
  Serial.println(reminder_hour);

  // Say the day of the week
  lcd.setCursor(0,0);
  print_DoW(myRTC.dayofweek);

  // Print hh:mm
  lcd.setCursor(10, 0);
  print02(myRTC.hours);
  lcd.print(":");
  print02(myRTC.minutes); 

  // Reminders per hour is 8, need to create hour variable
  if (reminder_hour != myRTC.hours) {
    reminder_hour = myRTC.hours;
    reminder_count = 0;
  }

  // Ask if I'm ready to study yet
  if ((pir_status == HIGH || reminder_timer > 0) && reminder_count <= 7 && study_status == 0) {            
    lcd.setCursor(0,1);
    lcd.print("Study? L(Y):R(N)");
    if (reminder_timer == 0) {
      reminder_timer = 5;
    }
    reminder_timer = reminder_timer - 1;
    reminder_subcount = reminder_subcount + 1;

    if (reminder_subcount == 5) {
      reminder_count = reminder_count + 1;
      reminder_subcount = 0;
    }
  }

  if (study_status == 1) {
    if (pomodoro_timer == 0) {
      pomodoro_timer = 1500;
    }
    lcd.setCursor(0, 1);
    lcd.print("Timer:");
    lcd.setCursor(10, 1);
    pomo_mins = pomodoro_timer/60;
    pomo_secs = pomodoro_timer%60;
    lcd.print(pomo_mins);
    lcd.print(":");
    print02(pomo_secs);
    pomodoro_timer = pomodoro_timer - 1;
  }

  // Study time code
  // Right button pressed
  if (butt < 60) {
    
  } // Up button
  else if (butt < 200) {
     //lcd.setCursor(0,1);
     //lcd.print ("Up    ");
  } // Down button
  else if (butt < 400){
     //lcd.setCursor(0,1);
     //lcd.print ("Down  ");
  } // Left button for turning on the timer
  else if (butt < 600){
    int i=5;
    study_loading = 1;
    
    for (int j = 0; j < 5; j++) {
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print ("Starting 25 min");
     lcd.setCursor(0,1);
     lcd.print("timer in:");
     lcd.setCursor(10,1);
     Serial.print("Countdown: ");
     Serial.println(i);
     lcd.print(i);
     i = i - 1;
     delay(1000);
    }
    
    if (i == 0) {
      study_loading = 0;
      study_status = 1;
      Serial.println("In study mode now");
    }
    
  } // Select button
  else if (butt < 800){
     //lcd.setCursor(0,1);
     //lcd.print ("Select");
  }

   
  // Delay for a second before looping again
  delay(1000);
}

// Backlight controller
void bl_lcd_control (uint16_t val) {
  // Currently set to between 11pm and 7am we idle
  // Need to introduce motion sensor variable to turn brightness up
  if (val >= 23 || val <= 7) {
    analogWrite(BL_LCD, 20);
  } else {
    analogWrite(BL_LCD, 128);
  }
}

// Day of week printer in text
void print_DoW(uint16_t val) {
  if (val == 1) {
    lcd.print("Sunday");
  }
  if (val == 2) {
    lcd.print("Monday");
  }
  if (val == 3) {
    lcd.print("Tuesday");
  }
  if (val == 4) {
    lcd.print("Wednesday");
  }
  if (val == 5) {
    lcd.print("Thursday");
  }
  if (val == 6) {
    lcd.print("Friday");
  }
  if (val == 7) {
    lcd.print("Saturday");
  }
}

// Leading zero issue fix
void print02(uint16_t val)
{
    if (val < 10)   // <10 = 09, 08, 07, 06, 05, 04, 03, 02, 01, 00 
    {
       lcd.print("0");  
    }
    lcd.print(val);   
}
