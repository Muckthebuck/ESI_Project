#define DATA 2
#define SHIFT 3
#define STORE 4

void store();
int pic[] = {0,0,0,0,0,0,0,0};

void setup() {
  pinMode(DATA, OUTPUT);
  pinMode(SHIFT, OUTPUT);
  pinMode(STORE, OUTPUT);
  
//  shiftOut(DATA, SHIFT, MSBFIRST, 0);
//  shiftOut(DATA, SHIFT, MSBFIRST, 255);
//  store();
}

void loop() {
   shiftOut(DATA, SHIFT, LSBFIRST, 0);
   shiftOut(DATA, SHIFT, LSBFIRST, 255);
   store();

//   for (int i=0; i<8; i++) {
//    shiftOut(DATA, SHIFT, LSBFIRST, ~pic[i]);
//    shiftOut(DATA, SHIFT, LSBFIRST, 128 >> i);
//    store();
//  }
}

void store() {
  digitalWrite(STORE, HIGH);
  delayMicroseconds(10);
  digitalWrite(STORE, LOW);
  delayMicroseconds(10);
}
