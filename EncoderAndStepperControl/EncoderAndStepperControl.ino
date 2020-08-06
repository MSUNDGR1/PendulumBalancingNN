//Steps per revolution = 800
//CW moves left
//CCW moves right
#include <digitalWriteFast.h>
//#include <AceRoutine.h>
//using namespace ace_routine;


#define encPinA 2
#define encPinB 3

#define stepPinEna 9
#define stepPinDir 8
#define stepPinPul 7

const int stepMin = -3200;
const int stepMax = 3200;

//encoder variables
volatile bool pinAState = LOW;
volatile bool pinBState = LOW;
volatile int incAngPos = 0;
int lastAngPos = 0;

/* r = reset, return to home pos wait and set angular position to 0
 * s = disable motor
 * n = reenable motor and set linPos = 0
 * o = send output data
 * c = stepper command change in speed (p for +, n for -)
 */

int linPos = 0;
int currVel = 0;
int lastVel = 0;
unsigned int stepTimeH; // calculated in microseconds from currVel, half of each step time
int dir = 0; //-1 is CCW, 1 is CW, 0 is stationary


char input1;
char input2;
char dataBuff[3];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(encPinA, INPUT);
  pinModeFast(encPinB, INPUT);
  pinModeFast(stepPinDir, OUTPUT);
  pinModeFast(stepPinPul, OUTPUT);
  pinMode(stepPinEna, OUTPUT);
  digitalWriteFast(stepPinDir, LOW);
  digitalWrite(stepPinEna, HIGH);
  pinAState = digitalRead(encPinA);
  dir = 1;
  linPos = 0;
  stepTimeH = 1000;
  pinBState = digitalReadFast(encPinB);
  attachInterrupt(digitalPinToInterrupt(encPinA), APULSE, FALLING);
  Serial.println("meme");
}

unsigned int stepTimeCalc(int stepsPerSecond){
  static unsigned int retVal = (unsigned int)(500000 / stepsPerSecond);
  return retVal; 
}

void powerCommand(){
  int interm;
  char2 = Serial.read();
  while(Serial.available() < 3);
  for(int i = 0; i < 3; i++){
    charBuff[i] = Serial.read();
  }
  if(char2 == 'p'){
    interm = lastVel;
    lastVel = currVel;
    currVel += atoi(dataBuff);
  }else if(char2 == 'n'){
    interm = lastVel;
    lastVel = currVel;
    currVel -= atoi(dataBuff);
  }
  if(lastVel <= 0 && currVel > 0){
    digitalWriteFast(stepPinDir, HIGH);
    dir = 1;
  }
  if(lastVel >= 0 && currVel < 0){
    digitalWriteFast(stepPinDir, LOW);
    dir = -1;
  }
  stepTimeH = stepTimeCalc(currVel);
}


void loop() {
   if(Serial.available() > 0){
      char1 = Serial.read();
      switch (char1){
        case ('c'):
          powerCommand();
      }
   }
   bool checkLeft = linPos >stepMin || dir > 0;
   bool checkRight = linPos <stepMax || dir < 0;
   if((dir != 0) && (checkLeft) && (checkRight)){
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(stepTimeH);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(stepTimeH);
      linPos += dir;
   }
   lastAngPos = incAngPos;
}





//Encoder pulse position ISR
void APULSE(){
  pinBState = digitalReadFast(encPinB);//125 NS read, neglible effect on overall timing
  if(pinBState == LOW){
    incAngPos++;
  }else{
    incAngPos--;
  }
}
