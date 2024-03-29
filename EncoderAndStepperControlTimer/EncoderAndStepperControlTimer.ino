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

//Motor variables
int linPos = 0;
int currVel = 0; //steps per second
int lastVel = 0; //steps per second at last update call
int desVel; //desired steps per second at next update call
int minStepsPerSecond = 30;
float UpdateTime = 0.15;
unsigned int stepTimeH; // calculated in microseconds from currVel, half of each step time
int dir = 0; //-1 is CCW, 1 is CW, 0 is stationary
bool operating =  false;
volatile bool swap = LOW;

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
  //Timer pulse at 15625 hz
  cli();//stop interrupts
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536) Sets standard update rate to 2 hz
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
  
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
  if(operating == false){ 
    operating = true;
    digitalWrite(stepPinEna, HIGH);
  }
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
   if!(((dir != 0) && (checkLeft) && (checkRight) && operating)){
      /*digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(stepTimeH);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(stepTimeH);
      linPos += dir;*/
      digitalWrite(stepPinEna, LOW);
      operating = false;
   }
   
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt at Xhz drives stepper
   if((dir != 0) && operating){
      digitalWriteFast(stepPinPul, swap);
      swap = !swap;
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
