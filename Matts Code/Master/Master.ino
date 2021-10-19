#include <virtuabotixRTC.h>
#include <Wire.h>
#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Buzzer.h>

/////////////////////////////////////////// SETUP //////////////////////////////////////

//// LCD
//command
#define LIGHTSOFF 60 
#define LIGHTSON 62

#define SMILE 43
#define SAD 44
#define BORED 45
#define QN 46

// DS1302 controller Pins
// CLK 5, DAT 4, Reset 6
virtuabotixRTC myRTC(5, 4, 6); 

// Photo resistor pin
#define PHOTO_PIN A0
#define PHOTO_THRESHOLD 150

// PIR Sensor Pin
#define PIR_PIN 3

// Active Buzzer Pin
#define ACTIVE_BUZZ_PIN 2
Buzzer buzzer(ACTIVE_BUZZ_PIN);

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
#define RESTART 6
int state = 0;
int animation_state=BORED;
// Ready_to_study Variables
bool RTS_message = 0;
int RTS_timer = 5;
unsigned long RTS_previous_time = 0;
unsigned long RTS_wait_time = 60000; // In Milliseconds
int RTS_reminder_hour;
int RTS_reminder_count = 8;

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

// Restart Timer Variables
int restart_timer_count = 12;

// Timing Variables for timer
int current_timer_length = 0; // Seconds
int current_timer_time;
unsigned long timer_start_time;
int study_time_duration = 10; // Seconds
int break_time_duration = 5; // Seconds

String prev_message="";
// Animation variables
 int previous_animation;
int current_animation;

// Sensor/button trigger status
bool left_btn = 0;
bool right_btn = 0;
bool third_btn = 0;
bool photo_input = 0;
bool enough_light = 0;
bool PIR = 0;
char data[32];
int light_state = 1;
// Messages
String random_message[] = {"Hello world", "maybe study?", "No youtube :("};
int random_message_loop_count = 0;

//functions

int RTS_has_time_passed(unsigned long current_time);
int is_enough_light();
void cancel_cancel_message();
void cancel_RTS_message();
void go_to_cancel_message();
void go_to_ReadyToStudy_message();
int RTS_reminder_remaining();
void go_to_standby();
void break_timer_finished();
void study_timer_finished();
void update_current_timer_time();
void start_timer();
void ready_to_study();
void reset_inputs();
int detected_movement();
void LCD_print(String top_message, String bottom_message);
String format_string_for_print(String str);
void local_LCD_display(String message);
String get_day_and_time();
String leading_zero(uint8_t val);
void buzzer_tone();
void happy_jingle();
void bl_lcd_control(uint16_t val);
String random_string();
String seconds_to_mmss(int seconds);
void serial_print_time();
void action_manager();
void toggle_lights(int state);

void setup() {
  Serial.begin(9600);
  // Begin Arduino Communication
  Wire.begin();
  myRTC.setDS1302Time(00,15,12,6,10,1,2014);
  pinMode(PIR_PIN, INPUT);
  pinMode(PHOTO_PIN, INPUT);
  pinMode(ACTIVE_BUZZ_PIN, OUTPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT);
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(THIRD_BUTTON_PIN, INPUT);
  state = STANDBY;
  reset_inputs();
  RTS_reminder_hour = myRTC.hours;
}

void loop() {
  check_inputs();
  action_manager();
}

void check_inputs() {
    if (digitalRead(LEFT_BUTTON_PIN)){
      left_btn = 1;
    }
    if (digitalRead(RIGHT_BUTTON_PIN)){
      right_btn = 1;
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

  // Check Photoresistor, if high, turn on screen, otherwise exit action manager.
  if(is_enough_light()==0){ 
    toggle_lights(LOW);
    reset_inputs();
    return;
  } else {
   toggle_lights(1);
  }

  // Update time on RTC
  myRTC.updateTime(); 

  // Cancel message state logic
  if(cancel_message==1){
    cancel_timer--;
    if (right_btn==1){
      cancel_cancel_message();
      go_to_standby();
    } else if (cancel_timer<0){
     // change_animation(previous_animation); 
      cancel_cancel_message();
    } else {
      LCD_overwrite=1;
      LCD_overwrite_message_bottom = ((String)cancel_timer + " s to confirm");
      LCD_print("","");
    }
  }

  // Ready to study state logic
  if(RTS_message==1){
    RTS_timer--;
    if (left_btn==1){
      cancel_RTS_message();
      start_timer();
    } else if (right_btn){
      cancel_RTS_message();
      go_to_standby();
    } else if (RTS_timer<0){
      cancel_RTS_message();      
    } else {
      LCD_overwrite = 1;
      animation_state = QN;
      LCD_overwrite_message_top = get_day_and_time();
      LCD_overwrite_message_bottom = ("Study? L(Y):R(N)");
      LCD_print("","");
    }
  }

  // Standby State logic
  if(state==STANDBY){
    animation_state = BORED;
    if (detected_movement()&&RTS_has_time_passed(current_time)){
      // Restrict PIR message to 8 per hour
      if (RTS_reminder_remaining() > 0){
        // continue
         go_to_ReadyToStudy_message();
      }
    }
    if (left_btn||right_btn){
      go_to_ReadyToStudy_message();
    } else {
      LCD_print(get_day_and_time(),random_string());
    }
  }

  // In timer state logic
  if(state==IN_STUDY){
    animation_state = SMILE;
    update_current_timer_time();
    if (cancel_message==0&&(left_btn||right_btn)){
      go_to_cancel_message();
    }
    if (current_timer_time >= 0){
      LCD_print(get_day_and_time(),("Timer: " + seconds_to_mmss(current_timer_time)));
    } else {
      study_timer_finished();
    }
  }

  // In break state logic
  if(state==IN_BREAK){
    update_current_timer_time();
    if (cancel_message==0&&(left_btn||right_btn)){
      go_to_cancel_message();
    }
    if (current_timer_time >= 0){
      LCD_print(get_day_and_time(), ("Break: " + seconds_to_mmss(current_timer_time)) );
    } else {
      break_timer_finished();
    }
  }

  // Restart state logic
  if(state==RESTART){
    restart_timer_count--;
    if (left_btn){
      reset_inputs();
      restart_timer_count = 12;
      start_timer();
    } else if (restart_timer_count==0 || right_btn){
      reset_inputs();
      restart_timer_count = 12;
      go_to_standby();
    }
  }
  
}

////////////////////// FUNCTIONS ///////////////////////////

// Check if enough time has passed between PIR activated Ready to study messages
int RTS_has_time_passed(unsigned long current_time){
  // Check if we are within first 60 seconds <- this is an edge case
  if (RTS_previous_time == 0){
    RTS_previous_time = current_time;
    return 1;
  }
  
  if ((unsigned long)(current_time - RTS_previous_time) >= RTS_wait_time){
    RTS_previous_time = current_time;
    return 1;
  } else {
    return 0;
  }
}

// Check if the Photo resistor is above the threshold
int is_enough_light(){
  if (analogRead(PHOTO_PIN)<PHOTO_THRESHOLD){
    return 0;
  } else {
    return 1;
  }
}

// Exit cancel message
void cancel_cancel_message(){
  reset_inputs();
  cancel_message = 0;
  LCD_overwrite = 0;
  cancel_timer = 5;
}

// Exit Ready to study message
void cancel_RTS_message(){
  animation_state = QN;
  reset_inputs();
  RTS_message = 0;
  LCD_overwrite = 0;
  RTS_timer = 5;
}

// Display cancel message
void go_to_cancel_message(){
     animation_state = QN;
  // change_animation(QN);
  reset_inputs();
  cancel_message = 1;
  LCD_overwrite = 1;
  LCD_overwrite_message_top = "End timer?: R(Y)";
  LCD_overwrite_message_bottom = ((String)cancel_timer + " s to confirm");
  LCD_print("","");  
}

// Display Ready to study message
void go_to_ReadyToStudy_message(){
   animation_state = QN;
 // change_animation(QN);
  reset_inputs();
  RTS_message = 1;
  LCD_overwrite = 1;
  LCD_overwrite_message_top = get_day_and_time();
//  animation_state = QN;
  LCD_overwrite_message_bottom = ("Study? L(Y):R(N)");
  LCD_print("","");
}

// Return how many reminders are remaining. Limited to 8 per hour from the PIR
// This function does not get used when RTS message is activated from hitting left or right
int RTS_reminder_remaining(){
  if (RTS_reminder_hour != myRTC.hours){
    RTS_reminder_hour = myRTC.hours;
    RTS_reminder_count = 8;
  } else if (RTS_reminder_count == 0){
  } else {
    RTS_reminder_count--;
  }
  return RTS_reminder_count;
}

// Switch to standby state
void go_to_standby(){
  //animation_state = BORED;
  state=STANDBY;
  reset_inputs();
  animation_state = BORED;
 // change_animation(BORED);
}

// Break Timer has finished, transition to RESTART state
void break_timer_finished(){
  reset_inputs();
  buzzer_tone();
  LCD_print("Restart timer?","L(Y):R(N)");
  state = RESTART;
}

// Study Timer has finished, transistion to BREAK state
void study_timer_finished(){
  animation_state = QN;
  state = IN_BREAK;
  timer_start_time = millis();
  current_timer_length = break_time_duration;
  current_timer_time = break_time_duration+1;
  happy_jingle();
  LCD_print(get_day_and_time(), ("Break: " + seconds_to_mmss(current_timer_time)) );
}

// Updates timer count. I guess we can get more accurate by adding checking against the RTC
void update_current_timer_time(){
  //int time_elapsed = (current_time - timer_start_time)/1000; // time elapsed in seconds
  //current_timer_time = current_timer_length - time_elapsed;
  current_timer_time--;
}

// Start the timer!
void start_timer(){
  animation_state = SMILE;
  state = IN_STUDY;
  reset_inputs();
  timer_start_time = millis();
  current_timer_length = study_time_duration;
  current_timer_time = study_time_duration+1;
  LCD_print(get_day_and_time(),("Timer: " + seconds_to_mmss(current_timer_time)));
}

// Resets all the inputs so we can get some fresh data
void reset_inputs(){
  PIR=0;
  left_btn=0;
  right_btn=0;
  third_btn=0;
}

// How we detect movement
int detected_movement(){
  if(PIR==1){
    return 1;
  } else {
    return 0;
  }
}

// Send message to other arduino to change the state of the backlight
void toggle_lights(int next_state){
  Wire.beginTransmission(SLAVE);
  if(light_state!=next_state){
    light_state=next_state;
    if(next_state ==1){
      Wire.write((char)LIGHTSON);
    }else{
      Wire.write((char)LIGHTSOFF);
    }
     Wire.endTransmission(SLAVE);
  }
}

// Send a message to other arduino to change animation
void change_animation(int animation){
  previous_animation = current_animation;
  current_animation = animation;
  Wire.beginTransmission(SLAVE);
  Wire.write(animation);
  Wire.endTransmission(SLAVE);
}

// Send a string to the other arduino to Display a message
void LCD_print(String top_message, String bottom_message){
  String top = format_string_for_print(top_message);
  String bottom = format_string_for_print(bottom_message);
  String full_message;
  if (LCD_overwrite){
    full_message = format_string_for_print(LCD_overwrite_message_top)+format_string_for_print(LCD_overwrite_message_bottom);
  } else {
    full_message = top+bottom;
  }
  full_message[31]= animation_state;
  if(prev_message!=full_message){
    
    prev_message = full_message;
       Wire.beginTransmission(SLAVE); // transmit to device #9
    for(int i=0; i<32; i++){
      Wire.write(full_message[i]);
    }
    Wire.endTransmission(SLAVE);
  }
 
}

// Format a string to be exactly 16 characters by adding spaces
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

// Returns the day then time
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
  output_string = (day_of_week +" " +leading_zero(myRTC.hours) + ":" + leading_zero(myRTC.minutes));
  return output_string;
}

// Adds a leading zero to a single digit number
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

// Converts an amount of seconds to mm:ss
String seconds_to_mmss(int seconds){
  uint8_t timer_mins = seconds/60;
  uint8_t timer_secs = seconds%60;
  return (leading_zero(timer_mins)+":"+leading_zero(timer_secs));
}

// Returns a random string
String random_string(){
  return random_message[random(0, 3)];
}

void happy_jingle(){
  buzzer.begin(0);

  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(NOTE_DS5, 520);
  buzzer.sound(NOTE_GS5, 260);
  buzzer.sound(NOTE_F5, 520);
  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(NOTE_DS5, 520);
  buzzer.sound(NOTE_GS5, 800);
  buzzer.sound(NOTE_F5, 1000 );  
  
  buzzer.end(100);
}

void buzzer_tone(){
  buzzer.begin(0);
  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(0, 260);
  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(0, 260);
  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(0, 260);
  buzzer.sound(NOTE_D5, 260);
  buzzer.sound(0, 260);
  
  buzzer.end(100);
}
