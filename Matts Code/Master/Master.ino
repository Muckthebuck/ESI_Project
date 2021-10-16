#include <virtuabotixRTC.h>
#include <Wire.h>
#include <Stepper.h>
#include <LiquidCrystal.h>


/////////////////////////////////////////// DEBUG VARIABLES ///////////////////////////////
#define LCD_MAIN_ARDUINO false // Is LCD shield connected to the arduino or not?
#define LCD_SHIELD_BUTTONS false // are you using the buttons on the LCD? Doesn't allow for config mode


/////////////////////////////////////////// SETUP //////////////////////////////////////

// LCD
//const int rs = 8, en =9, d4 = 10, d5=11, d6=12, d7=13, lcd_on=7;
//LiquidCrystal lcd(rs,en,d4,d5,d6,d7);
//

// DS1302 controller Pins
// CLK 5, DAT 4, Reset 6
virtuabotixRTC myRTC(5, 4, 6); 

// PIR Sensor Pin
#define PIR_PIN 3

// Active Buzzer Pin
#define ACTIVE_BUZZ_PIN 2 

// Button Pins
#define LEFT_BUTTON_PIN A1
#define RIGHT_BUTTON_PIN A2
#define THIRD_BUTTON_PIN A3


// Arduino Communication
#define SLAVE 9

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
 
//Stepper steppermotor(STEPS_PER_REV, 8, 10, 9, 11);

///////////////////////////////////////// VARIABLES //////////////////////////////////////////

// State Variables
#define NO_MOVEMENT 1
#define READY_FOR_STUDY 2
#define STANDBY 3
#define IN_STUDY 4
#define IN_BREAK 5
int state = 0;

// Cancel Variables
bool cancel_message = 0;
int cancel_timer = 5;

// LCD Overwrite 
bool LCD_overwrite;
String LCD_overwrite_message_top;
String LCD_overwrite_message_bottom;

// Timing Variables for sudo-async functions
unsigned long current_time;
const int am_interval = 1000;
unsigned long am_prev_time = 0;

// Timing Variables for timer
int current_timer_length = 0; // Seconds
int current_timer_time;
unsigned long timer_start_time;
int study_time_duration = 90; // Seconds
int break_time_duration = 75; // Seconds

// Timing Variables for config mode
 

// Sensor/button trigger status
bool left_btn = 0;
bool right_btn = 0;
bool third_btn = 0;
bool PIR = 0;
char data[32];

// Messages
String random_message[] = {"Hello world", "maybe study?", "No youtube :("};
int random_message_loop_count = 0;



void setup() {
  Serial.begin(9600);
  // Begin Arduino Communication
  Wire.begin();
  myRTC.setDS1302Time(00,23,11,4,10,13,2021);
  pinMode(PIR_PIN, INPUT);

  
//  pinMode(ACTIVE_BUZZ_PIN, OUTPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT);
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(THIRD_BUTTON_PIN, INPUT);
//  lcd.begin(16, 2);                          // put your LCD parameters here
  state = NO_MOVEMENT;
  reset_inputs();
}

void loop() {
  check_inputs();
  action_manager();
}

void check_inputs() {
  if (LCD_SHIELD_BUTTONS==true){
    int butt = analogRead(A0);
    if (butt == 0) {
      right_btn = 1;
    } // Up button
    else if (butt < 200) {
      //button_num = 3;
    } // Down button
    else if (butt < 400){
      //button_num = 4;
    } // Left button for turning on the timer
    else if (butt == 478){
      left_btn = 1;
    } // Select button
    else if (butt < 800){
      //button_num = 1;
    }
  } else {
    if (digitalRead(LEFT_BUTTON_PIN)){
      left_btn = 1;
    }
    if (digitalRead(RIGHT_BUTTON_PIN)){
      right_btn = 1;
    }
    if (digitalRead(THIRD_BUTTON_PIN)){
      third_btn = 1;
    }
  }


  
  if (digitalRead(PIR_PIN)){
    PIR = 1;
  }
}

void action_manager(){
  // https://docs.google.com/document/d/1u9RpGDv0Du3XHBDmag-IDaQyKIT7uMIzJ4LOqx1lm7U/edit#heading=h.sil5lysrvarm
  // Limits how quickly this function happens.
  current_time = millis();
  if ((unsigned long)(current_time - am_prev_time) >= am_interval){
      am_prev_time = current_time;
  } else {
    return;
  }

  Serial.println(digitalRead(PIR_PIN));
  // Update the time on the variables
  myRTC.updateTime(); 
  //serial_print_time();

  if(cancel_message==1){
    cancel_timer--;
    if (right_btn==1){
      cancel_cancel_message();
      go_to_standby();
    } else if (cancel_timer<0){
      cancel_cancel_message();
    } else {
      LCD_overwrite_message_bottom = ((String)cancel_timer + " s to confirm");
      LCD_print("","");
    }
  }

  // Firstly, if we have no movement. Check if there has been movement and display "Ready to study?"
  if(detected_movement()==1 && state==NO_MOVEMENT){
    ready_to_study();
  }

  // Ready to study
  if(state==READY_FOR_STUDY){
    if(left_btn){
      start_timer();
    } else if (right_btn){
      go_to_standby();
    }
  }

  if(state==STANDBY){
    if (third_btn){
      ready_to_study();
    } else {
      LCD_print(get_day_and_time(),random_string());
    }
  }
  
  if(state==IN_STUDY){
    update_current_timer_time();
    if (cancel_message==0&&(left_btn||right_btn)){
      go_to_cancel_message();
    }
    if (current_timer_time >= 0){
      LCD_print(("Timer: " + seconds_to_mmss(current_timer_time)) ,"In timer");
    } else {
      study_timer_finished();
    }
  }

  if(state==IN_BREAK){
    update_current_timer_time();
    if (cancel_message==0&&(left_btn||right_btn)){
      go_to_cancel_message();
    }
    if (current_timer_time >= 0){
      LCD_print(("Break: " + seconds_to_mmss(current_timer_time)) ,"In timer");
    } else {
      break_timer_finished();
    }
  }

  
  
  //LCD_print(get_day_and_time(),"boop a ti boop");

  
}

void cancel_cancel_message(){
  reset_inputs();
  cancel_message = 0;
  LCD_overwrite = 0;
  cancel_timer = 5;
}

void go_to_cancel_message(){
  reset_inputs();
  cancel_message = 1;
  LCD_overwrite = 1;
  LCD_overwrite_message_top = "End timer?: R(Y)";
  LCD_overwrite_message_bottom = ((String)cancel_timer + " s to confirm");
  LCD_print("","");
}

void go_to_standby(){
  state=STANDBY;
  reset_inputs();
}

void break_timer_finished(){
  reset_inputs();
  LCD_print("Study? L(Y):R(N)"," ");
  state = READY_FOR_STUDY;
}

void study_timer_finished(){
  state = IN_BREAK;
  timer_start_time = millis();
  current_timer_length = break_time_duration;
  current_timer_time = break_time_duration+1;
  LCD_print(("Break: " + seconds_to_mmss(current_timer_time)) ,"In timer");
}

void update_current_timer_time(){
  //int time_elapsed = (current_time - timer_start_time)/1000; // time elapsed in seconds
  //current_timer_time = current_timer_length - time_elapsed;
  current_timer_time--;
}

void start_timer(){
  state = IN_STUDY;
  reset_inputs();
  timer_start_time = millis();
  current_timer_length = study_time_duration;
  current_timer_time = study_time_duration+1;
  LCD_print(("Timer: " + seconds_to_mmss(current_timer_time)) ,"In timer");
}

void ready_to_study(){
  state = READY_FOR_STUDY;
  LCD_print("Ready for study?","L: Yes R:No");
  reset_inputs();
}

void reset_inputs(){
  PIR=0;
  left_btn=0;
  right_btn=0;
  third_btn=0;
}

int detected_movement(){
  if(PIR==1){
    return 1;
  } else {
    return 0;
  }
}


void LCD_print(String top_message, String bottom_message){
  String top = format_string_for_print(top_message);
  String bottom = format_string_for_print(bottom_message);
  String full_message;

  if (LCD_overwrite){
    full_message = format_string_for_print(LCD_overwrite_message_top)+format_string_for_print(LCD_overwrite_message_bottom);
  } else {
    full_message = top+bottom;
  }
  if (LCD_MAIN_ARDUINO){
      local_LCD_display(full_message);
  } else {
    Wire.beginTransmission(SLAVE); // transmit to device #9
    for(int i=0; i<32; i++){
      Wire.write(full_message[i]);
    }
    Wire.endTransmission(SLAVE);
  }
  
}

String format_string_for_print(String str){
  // Formats a string to be exactly 16 characters
  int str_length = str.length();
  if (str_length>16){
    Serial.println("Problem with string");
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

void local_LCD_display(String message){
   //lcd.clear(); 
//   message.toCharArray(data, 33);
//   int i;
//   for(i=0;i<16;i++){
//     lcd.setCursor(i,0);
//     lcd.write(data[i]);
//   }   
//   for(i=16;i<32;i++){
//     lcd.setCursor(i-16,1);
//     lcd.write(data[i]);
//   }
   //Serial.println(message + "3");
}

String get_day_and_time(){
  int day_number = myRTC.dayofweek;
  String day_of_week;
  String output_string;
  if (day_number == 1) {
    day_of_week = "Sunday   ";
  } else if (day_number == 2){
    day_of_week = "Monday   ";
  } else if (day_number == 3){
    day_of_week = "Tuesday  ";
  } else if (day_number == 4){
    day_of_week = "Wednesday";
  } else if (day_number == 5){
    day_of_week = "Thursday ";
  } else if (day_number == 6){
    day_of_week = "Friday   ";
  } else if (day_number == 7){
    day_of_week = "Saturday ";
  } 
  output_string = (day_of_week + " " + leading_zero(myRTC.hours) + ":" + leading_zero(myRTC.minutes));
  return output_string;
}

String leading_zero(uint8_t val){
    if (val < 10)   // <10 = 09, 08, 07, 06, 05, 04, 03, 02, 01, 00 
    {
       return ("0" + (String)val);
    }
    return (String)val; 
}

void serial_print_time(){
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
}

String seconds_to_mmss(int seconds){
  uint8_t timer_mins = seconds/60;
  uint8_t timer_secs = seconds%60;
  return (leading_zero(timer_mins)+":"+leading_zero(timer_secs));
}

String random_string(){
  return random_message[random(0, 3)];
}
