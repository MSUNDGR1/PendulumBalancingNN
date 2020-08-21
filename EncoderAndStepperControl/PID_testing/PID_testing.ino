#define encPinA 2
#define encPinB 3

#define stepPinEna 9
#define stepPinDir 8 //LOW is ccw/left, HIGH is cw/right
#define stepPinPul 7

#include <digitalWriteFast.h>
const int stepMin = -500;
const int stepMax = 500;

volatile bool pinAState = LOW;
volatile bool pinBState = LOW;
volatile int incAngPos = 0;

int linPos, currVel;
unsigned int stepTimeH;


double kp = 8;
double ki = 0.000;
double kd = 1;

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double output, setPoint;
double cumError, rateError;
bool active;
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(encPinA, INPUT);
  pinModeFast(encPinB, INPUT);
  pinModeFast(stepPinDir, OUTPUT);
  pinModeFast(stepPinPul, OUTPUT);
  pinMode(stepPinEna, OUTPUT);
  digitalWriteFast(stepPinDir, LOW);
  digitalWrite(stepPinEna, HIGH);
  pinAState = digitalRead(encPinA);
  linPos = 0;
  pinBState = digitalReadFast(encPinB);
  attachInterrupt(digitalPinToInterrupt(encPinA), APULSE, FALLING);
  lastError = 0;
  previousTime = 0;
  setPoint = 300;
  cumError = 0;
  currVel = 0;
  active = false;
  stepTimeH = 100000;
}

int stepCount = 0;

void loop() {
  //Serial.println(incAngPos);
  // put your main code here, to run repeatedly:
  if(!active && incAngPos > 295) active = true;
  if(active){
    if(stepCount % 30 == 0){
     output = computePID(incAngPos);
     
     currVel += (output / 50);
     if(!((abs(output) > 1000))){
       stepTimeH = (int)(100000 / abs(output));
     }
    }
   if(output < 0){
      digitalWriteFast(stepPinDir, LOW);
      
      linPos--;
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(stepTimeH);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(stepTimeH);
      
    }else if(output > 0){
      digitalWriteFast(stepPinDir, HIGH);
      
      linPos++;
      digitalWriteFast(stepPinPul, HIGH);
      delayMicroseconds(stepTimeH);
      digitalWriteFast(stepPinPul, LOW);
      delayMicroseconds(stepTimeH);
      
    }
    if(linPos > stepMax || linPos < stepMin){
      digitalWrite(stepPinEna, LOW);
    }
    stepCount++;
  }
}

double computePID(double inp){
  currentTime = millis();
  elapsedTime = currentTime - previousTime;

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
