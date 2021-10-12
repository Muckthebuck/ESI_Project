#include <LiquidCrystal.h>

#include <virtuabotixRTC.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// DS1302 controller
// CLK 13, DAT 12, Reset 11
virtuabotixRTC myRTC(13, 12, 11); 

#define BL_LCD 10
#define PIR_PIN 2
#define ACTBZ_PIN 3

///////////////// Varibles /////////////////////
const int am_interval = 1000;
const int LEFT_BTN = 0; 
const int RIGHT_BTN = 1; 
unsigned long am_prev_time = 0;
int pir_status = 0;
int reminder_timer = 0;
int reminder_count = 0;
int reminder_hour = 100; //Set to 100 so we can reassign on startup
int butt;
int i=0;
int study_status = 0; // 0 is not studying, 1 is studying
int study_countdown = 0;
int study_loading = 0;
int pomodoro_timer = 0;
int pomo_mins;
int pomo_secs;
int cancel_counter;
int cancel_status = 0;
int cancel_first_time = 0;
int button_num; // 1 for SEL, 2 for LEFT, 3 for UP, 4 for DOWN
                // 5 for RIGHT
bool left_button = 0;
bool right_button = 0;
bool set_timer = 0;
bool study_pomo = 0;
bool study_break = 0;
bool break_time = 0;
int end_of_time_count;

void setup() {
  // Serial and lcd screen initilisation 
  Serial.begin(9600);
  lcd.begin(16,2);

  // Pins for buttons that I wasn't able to implement - not sure why
  pinMode(LEFT_BTN, INPUT);
  pinMode(RIGHT_BTN, INPUT);

  // Backlight and PIR sensor pins
  pinMode(BL_LCD, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  
  // Sets time time in sec, min, hour, day of week, day, mon, year
  //myRTC.setDS1302Time(00, 12, 15, 1, 10, 10, 2021); 
}

void loop() {
  check_inputs();
  action_manager();
}

void check_inputs() {
  
  // Analog buttons 
  butt = analogRead(0);

  //int right_state = digitalRead(RIGHT_BTN);
  //int left_state = digitalRead(LEFT_BTN);

  // Sets the button_num from 1-5 (Uses lcd shield buttons)
  if (butt < 60) {
    right_button = 1;
  } // Up button
  else if (butt < 200) {
    button_num = 3;
  } // Down button
  else if (butt < 400){
    button_num = 4;
  } // Left button for turning on the timer
  else if (butt < 600){
    left_button = 1;
  } // Select button
  else if (butt < 800){
    button_num = 1;
  }
}

void action_manager() {
  unsigned long current_time = millis();
  if ((unsigned long)(current_time - am_prev_time) >= am_interval)
  {
      am_prev_time = current_time;
  } else {
    return;
  }
  
  lcd.clear();
  
  // Update the time on the variables
  myRTC.updateTime();

  // Motion sensor
  pir_status = digitalRead(PIR_PIN);

  // if motion detected // trouble shooting
  if (pir_status == HIGH && study_status == 0) {            
    Serial.println("Motion detected");
  } 
  
  // Time of day function - backlight brightness control
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
  Serial.print(reminder_timer);
  Serial.print(",hour of reminder:");
  Serial.println(reminder_hour);

  // If the left button has been pressed AND we are not studying 
  if (left_button && study_status==0) {
    // For post break period, to go back into study mode when lftbtn is pressed
    break_time = 0;
    // set timer to 5 seconds
    if (set_timer == 0) {
      i=5;
      set_timer = 1;
    }
    // Initialise study phase
    study_loading = 1;

    // If right button pressed, stop initialising
    if (right_button) {
      left_button = 0;
      right_button = 0;
      set_timer = 0;
    }

    // Countdown the initialisation timer
    if (i >= 0) {
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
    }

     // Timer finished -> study mode
    if (i == -1) {
      // Turn off left button press & put into study mode
      left_button = 0;
      study_loading = 0;
      study_status = 1;
      // 
      study_pomo = 1;
      set_timer = 0;
      Serial.println("In study mode now");
      lcd.clear();
      // Say the day of the week
      lcd.setCursor(0,0);
      print_DoW(myRTC.dayofweek);
    
      // Print hh:mm
      lcd.setCursor(10, 0);
      print02(myRTC.hours);
      lcd.print(":");
      print02(myRTC.minutes); 
    }
  } // Else if i haven't pressed the left button and 
  // it is not break time pomodoro timer isn't 0
  // check to see if i'm ready to study - PIR sensor activated
  else if (!(break_time == 1 && pomodoro_timer == 0)) {
    // Say the day of the week
    lcd.setCursor(0,0);
    print_DoW(myRTC.dayofweek);
  
    // Print hh:mm
    lcd.setCursor(10, 0);
    print02(myRTC.hours);
    lcd.print(":");
    print02(myRTC.minutes); 
  
    // Reminders per hour is 8. Reset the count for reminders if a new
    // hour 
    if (reminder_hour != myRTC.hours) {
      reminder_hour = myRTC.hours;
      reminder_count = 0;
    }
  
    // Ask if I'm ready to study yet
    // If sensor has activated or i'm in the 5 second reminder period
    // and additional conditions
    if ((pir_status == HIGH || reminder_timer > 0) && reminder_count <= 7 && study_status == 0 && break_time == 0) {            
      
      // Ask if I want to study (using the timer)
      lcd.setCursor(0,1);
      lcd.print("Study? L(Y):R(N)");
      
      // Set countdown timer to 5
      // Only gets sets to 0 when a new reminder commences 
      if (reminder_timer == 0) {
        reminder_timer = 5;
        reminder_count = reminder_count + 1;
      }
      
      // iterate
      reminder_timer = reminder_timer - 1;
    }
  }

  // after elapsing the initialisation timer, study status = 1
  if (study_status == 1) {

    // pomodoro_timer for the countdown, study_pomo for the
    // study state

    // If the timer is 0, and we are in study_pomo (to indicate
    // it is the first time we are starting this timer) 
    if (pomodoro_timer == 0 && study_pomo == 1) {
      pomodoro_timer = 15; //  change to 25 mins
      study_pomo = 0; // not the first time anymore
    } 
    // if we have elapsed 25 mins
    else if (pomodoro_timer == 0) {
      // Go into break mode 
      break_time = 1;
      // First time indicator function like study_pomo
      study_break = 1;
      // Not studying anymore
      study_status = 0;
    } 
    // Active buzzer on for one second to signal end of timer
    if (pomodoro_timer == 1){
      tone(ACTBZ_PIN, 60,1000);
    }
    
    // update timer
    pomo_mins = pomodoro_timer/60;
    pomo_secs = pomodoro_timer%60;
    
    // Print on the screen how long we have
    lcd.setCursor(0, 1);
    lcd.print("Timer:");
    lcd.setCursor(10, 1);
    print02(pomo_mins); // with leading 0's
    lcd.print(":");
    print02(pomo_secs);

    //iterate 
    if (pomodoro_timer > 0) {
      pomodoro_timer = pomodoro_timer - 1;
    }
  } 

  if (break_time == 1) {
    if (pomodoro_timer == 0 && study_break == 1) {
      pomodoro_timer = 5; //change to 5 mins
      study_break = 0; // not first time anymore
      end_of_time_count = 12;
    } 
    // If we have run out of study break time
    else if (pomodoro_timer == 0) {
      lcd.setCursor(0,0);
      lcd.print("Restart timer?");
      lcd.setCursor(7,1);
      lcd.print("L(Y):R(N)");

      // Right button to go to home screen 
      if (right_button) {
        break_time = 0;
        right_button = 0;
      }

      // auto cancel/restart function goes here
      if (end_of_time_count == 0) {
        end_of_time_count = 0;
        break_time = 0;
      }
      // Auto stops the counter after specified period
      end_of_time_count = end_of_time_count - 1;
    }

    // Iterate and display on screen basically
    if (pomodoro_timer > 0) {
      Serial.print(pomodoro_timer);
      lcd.setCursor(0, 1);
      lcd.print("Break:");
      lcd.setCursor(10, 1);
      pomo_mins = pomodoro_timer/60;
      pomo_secs = pomodoro_timer%60;
      print02(pomo_mins);
      lcd.print(":");
      print02(pomo_secs);
      pomodoro_timer = pomodoro_timer - 1;  
    }  
  }

  // If we are cancelling mid study/break then activate the cancel_status and first time
  // variables - for initialising the timer
  if (right_button && (study_status || break_time) && !cancel_status) {
    cancel_status = 1;
    cancel_first_time = 1;
    right_button = 0;
  }

  // If we are cancelling the timer
  if (cancel_status) {
    // If we have at least 5 seconds or more left on our timer 
    if (pomodoro_timer >= 5) {
      lcd.clear();

      // If it is our first time, then set timer to 5
      if (cancel_counter == 0 && cancel_first_time) {
        cancel_counter = 5;
        cancel_first_time = 0;
      }

      // Print on LCD
      lcd.setCursor(0,0);
      lcd.print("End timer?: R(Y)");
      lcd.setCursor(0,1);
      lcd.print(cancel_counter);
      lcd.print(" s to confirm");
      cancel_counter = cancel_counter - 1;

      // If I press R btn in 5 seconds, cancel the whole timer
      if (right_button && cancel_status) {
        right_button = 0;
        study_status = 0;
        pomodoro_timer = 0;
        cancel_counter = 0;
      }

      // If the timer ends, go back to whatever mode it was in
      if (cancel_counter == -1) {
        cancel_counter=0;
        cancel_status = 0;
      }
    }
  }
}

////////////// Backlight controller //////////////
void bl_lcd_control (uint16_t val) {
  // Currently set to between 11pm and 7am we idle
  // Need to introduce motion sensor variable to turn brightness up
  if (val >= 23 || val <= 7) {
    analogWrite(BL_LCD, 20);
  } else {
    analogWrite(BL_LCD, 128);
  }
}

//////////// Day of week printer in text ///////////////
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

/////////////// Leading zero issue fix //////////////////
void print02(uint16_t val)
{
    if (val < 10)   // <10 = 09, 08, 07, 06, 05, 04, 03, 02, 01, 00 
    {
       lcd.print("0");  
    }
    lcd.print(val);   
}
