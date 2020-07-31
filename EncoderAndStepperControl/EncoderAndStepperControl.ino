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

char recCom; //First Character off Serial input
/* r = reset, return to home pos wait and set angular position to 0
 * s = disable motor
 * n = reenable motor and set linPos = 0
 * o = send output data
 * c = stepper command change in speed
 */
int serialByteInput;
int serialIn;
int serialOutAngNow;
int serialOutAngThen;
int serialOutLinPos;
int serialOutLinSpeed;

int linPos = 0;
int currVel = 0;
int lastVel = 0;
unsigned int stepTimeH; // calculated in microseconds from currVel, half of each step time
int dir = 0; //-1 is CCW, 1 is CW, 0 is stationary

byte inBuff[20];
byte outBuff[16];

/*COROUTINE(step){
  COROUTINE_LOOP() {
   if((dir != 0) && (linPos > stepMin) && (linPos < stepMax)){
    digitalWriteFast(stepPinPul, HIGH);
    COROUTINE_DELAY_MICROS(stepTimeH);
    digitalWriteFast(stepPinPul, LOW);
    COROUTINE_DELAY_MICROS(stepTimeH);
    linPos += dir;
   }
  }
}

COROUTINE(dynam){
  COROUTINE_LOOP(){
    if(incAngPos ==0){
     dir = 0;
    }
    if(lastAngPos != incAngPos){
     if((lastAngPos >= 0) && (incAngPos < 0)){
       Serial.println("CCW");
       digitalWriteFast(stepPinDir, LOW);
       dir = -1;
     }else if((lastAngPos <=0) && (incAngPos > 0)){
       Serial.println("CW");
       digitalWriteFast(stepPinDir, HIGH);
       dir = 1;
    }
    stepTimeH = (unsigned int) (500000 / incAngPos);
   }
   lastAngPos = incAngPos;
  }
}*/

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


/*Input Message is 5 bytes
 * Initial char is command
 * next 4 bytes are integer corresponding to command
 * 
 * Output message is 16 bytes from 4 ints
 * 1. Current angular position from encoder
 * 2. Previous timestep angular position from encoder
 * 3. Linear position of carriage 
 * 4. Linear velocity of carriage
 */
void loop() {
  
  /*step.runCoroutine();
  dynam.runCoroutine();*/
  /*serialByteInput = Serial.available();
  if(serialByteInput > 0){
    for(int i = 0; i < serialByteInput; i++){
      inBuff[i] = Serial.read();
    }
    
  }*/
  if(incAngPos ==0){
     dir = 0;
    }
    if(lastAngPos != incAngPos){
      Serial.println(incAngPos);
     if((lastAngPos >= 0) && (incAngPos < 0)){
       Serial.println("CCW");
       digitalWriteFast(stepPinDir, LOW);
       dir = -1;
     }else if((lastAngPos <=0) && (incAngPos > 0)){
       Serial.println("CW");
       digitalWriteFast(stepPinDir, HIGH);
       dir = 1;
    }
    stepTimeH = (unsigned int) (500000 / abs(3 *incAngPos));
   }
   lastAngPos = incAngPos;
   bool checkLeft = linPos >stepMin || dir > 0;
   bool checkRight = linPos <stepMax || dir < 0;
  if((dir != 0) && (checkLeft) && (checkRight)){
    digitalWriteFast(stepPinPul, HIGH);
    //COROUTINE_DELAY_MICROS(stepTimeH);
    delayMicroseconds(stepTimeH);
    digitalWriteFast(stepPinPul, LOW);
    //COROUTINE_DELAY_MICROS(stepTimeH);
    delayMicroseconds(stepTimeH);
    linPos += dir;
   }
}

unsigned int stepTimeCalc(int stepsPerSecond){
  static unsigned int retVal = (unsigned int)(500000 / stepsPerSecond);
  return retVal; 
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