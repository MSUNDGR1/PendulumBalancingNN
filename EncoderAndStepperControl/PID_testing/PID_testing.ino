#define encPinA 2
#define encPinB 3

#define stepPinEna 9
#define stepPinDir 8 //LOW is ccw/left, HIGH is cw/right
#define stepPinPul 7

#include <digitalWriteFast.h>
const int stepMin = -1000;
const int stepMax = 1000;

volatile bool pinAState = LOW;
volatile bool pinBState = LOW;
volatile int incAngPos = 0;

int linPos;
unsigned int stepTimeH;


double kp = 2
double ki = 5
double kd = 1

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double output, setPoint;
double cumError, rateError;

void setup() {
  // put your setup code here, to run once:
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
  lastError = 0;
  previousTime = 0;
  setPoint = 0;
  cumError = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  output = computePID(incAngPos);
  if(output < 0){
    digitalWriteFast(stepPinDir, LOW);
    int stepNum = abs(output);
    for(int i = 0; i < stepNum; i++){
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(600);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(600);
    }
  }else if(output > 0){
    digitalWriteFast(stepPinDir, HIGH);
    int stepNum = abs(output);
    for(int i = 0; i < stepNum; i++){
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(600);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(600);
    }
  }
}

double computePID(double inp){
  currentTime = millis();
  elapsedTime = (double)(currentTime - previousTime);

  error = setPoint - inp;
  cumError += error * elapsedTime;
  rateError = (error - lastError) / elapsedTime;

  double out = kp * error + ki * cumError + kd * rateError;
  lastError = error;
  previousTime = currentTime;
  return out;
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
