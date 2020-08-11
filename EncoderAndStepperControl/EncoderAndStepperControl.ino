//Steps per revolution = 800
//CW moves left
//CCW moves right
#include <digitalWriteFast.h>


#define encPinA 2
#define encPinB 3

#define stepPinEna 9
#define stepPinDir 8
#define stepPinPul 7

const int stepMin = -1000;
const int stepMax = 1000;

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
bool operating =  false;

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
  digitalWrite(stepPinEna, LOW);
  pinAState = digitalRead(encPinA);
  dir = 0;
  linPos = 0;
  stepTimeH = 1000;
  pinBState = digitalReadFast(encPinB);
  attachInterrupt(digitalPinToInterrupt(encPinA), APULSE, FALLING);
  while(Serial.available() > 0){
    char in = Serial.read();
  }
}

unsigned int stepTimeCalc(int stepsPerSecond){
  static unsigned int retVal = (unsigned int)(500000 / abs(stepsPerSecond));
  return retVal; 
}

void powerUp(){
  digitalWriteFast(stepPinDir, HIGH);
  dir = 1;
  stepTimeH = stepTimeCalc(1500);
}

void powerCommand(){
  
  while(Serial.available() < 1);
  input2 = Serial.read();
  
  if(input2 == 'p'){
    operating = true;
    while(Serial.available() < 3);
    for(int i = 0; i < 3; i++){
      dataBuff[i] = Serial.read();
    }
    lastVel = currVel;
    currVel = atoi(dataBuff);
  }else if(input2 == 'n'){
    operating = true;
    while(Serial.available() < 3);
    for(int i = 0; i < 3; i++){
      dataBuff[i] = Serial.read();
    }
    lastVel = currVel;
    currVel = -1 *atoi(dataBuff);
  }else if(input2 == 'z'){
    operating = false;
    lastVel = currVel;
    currVel = 0;
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

void sendCommand(){
    Serial.println(incAngPos);
    Serial.println(lastAngPos);
    lastAngPos = incAngPos;
    Serial.println(linPos);
    Serial.println(currVel);
    lastAngPos = incAngPos;
}

void resetCommand(){
  if(linPos > 0){
      digitalWriteFast(stepPinDir, LOW);
      dir = -1;
      stepTimeH = stepTimeCalc(800);
      while(linPos != 0){
        digitalWriteFast(stepPinPul, HIGH);
        delayMicroseconds(stepTimeH);
        digitalWriteFast(stepPinPul, LOW);
        delayMicroseconds(stepTimeH);
        linPos += dir;
      }
    
  }else if(linPos < 0){
    digitalWriteFast(stepPinDir, HIGH);
      dir = 1;
      stepTimeH = stepTimeCalc(800);
      while(linPos != 0){
        digitalWriteFast(stepPinPul, HIGH);
        delayMicroseconds(stepTimeH);
        digitalWriteFast(stepPinPul, LOW);
        delayMicroseconds(stepTimeH);
        linPos += dir;
      }
  }
  delay(2000);
  incAngPos = 0;
  lastAngPos = 0;
  currVel = 0;
  lastVel = 0;
  dir = 0;
  operating = false;
}

void disableCommand(){
  digitalWrite(stepPinEna, LOW);
  operating = false;
}

void reenableCommand(){
  digitalWrite(stepPinEna, HIGH);
  linPos = 0;
  resetCommand();
}

void loop() {
   if(Serial.available() > 0){
      input1 = Serial.read();
      switch (input1){
        case ('c'):
          powerCommand();
          break;
         case ('r'):
          resetCommand();
          break;
         case ('n'):
          reenableCommand();
          break;
         case ('s'):
          disableCommand();
          break;
         case ('o'):
          sendCommand();
          break;
      }
   }
   bool checkLeft = linPos >stepMin || dir > 0;
   bool checkRight = linPos <stepMax || dir < 0;
   if((dir != 0) && (checkLeft) && (checkRight) && operating){
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(stepTimeH);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(stepTimeH);
      linPos += dir;
   }
   
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
