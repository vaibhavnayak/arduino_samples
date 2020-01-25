
#include <PID_v1.h>

#define PIN_INPUT 2
#define PIN_OUTPUT 3
int Kt = 10;
int err = 0 , old_err , errB = 5;
int hp = 200;
int lp = 200;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

const int Apwm = 11 ;  //initializing pin as pwm
const int Ain_1 = 4 ;
const int Ain_2 = 5 ;

const int Bpwm = 10 ;  //initializing pin as pwm
const int Bin_1 = 6 ;
const int Bin_2 = 7 ;

#define encoderPinA  2
#define encoderPinB  3
volatile long encoderPosRight = 0;
volatile long encoderPosLeft = 0;
long max_en = 0;

long newpositionRight;
long oldpositionRight = 0;
long newpositionLeft;
long oldpositionLeft = 0;
unsigned long newtime;
unsigned long oldtime = 0;
long velA, velB;

//For providing logic to L298 IC to choose the direction of the DC motor

void setup()
{
  pinMode(encoderPinA, INPUT);
  digitalWrite(encoderPinA, HIGH);       // turn on pullup resistor
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinB, HIGH);
  attachInterrupt(0, doEncoderA, RISING);  // encoDER ON PIN 2
  attachInterrupt(1, doEncoderB, RISING);
  Serial.begin (115200);
  Serial.println("start");

  //initialize the variables we're linked to
  //Input = analogRead(PIN_INPUT);
  Setpoint = 250;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  pinMode(Apwm, OUTPUT) ;  //we have to set PWM pin as output
  pinMode(Ain_1, OUTPUT) ; //Logic pins are also set as output
  pinMode(Ain_2, OUTPUT) ;
  pinMode(Bpwm, OUTPUT) ;  //we have to set PWM pin as output
  pinMode(Bin_1, OUTPUT) ; //Logic pins are also set as output
  pinMode(Bin_2, OUTPUT) ;
}

void loop()
{
  //For Clock wise motion , in_1 = High , in_2 = Low
  // Serial.print (analogRead(PIN_INPUT));
  newpositionRight = encoderPosRight;
  newpositionLeft = encoderPosLeft;
  newtime = millis();
  velA = (newpositionRight - oldpositionRight) * 10000 / (newtime - oldtime); // 1000(to conver milli sec to sec)*60(to convert sec to minutes)/6(total number of pulses per rotation)=10000 (pulses per milli sec to rpm)
  velB = (newpositionLeft - oldpositionLeft) * 10000 / (newtime - oldtime);
  Serial.print ("  ");
  Serial.print (encoderPosRight);
  Serial.print ("  ");
  Serial.print (encoderPosLeft);
  Serial.print ("  ");
  Serial.print (newpositionRight);
  Serial.print ("  ");
  Serial.print (newpositionLeft);
  Serial.print ("  speed = ");
  Serial.print (velA);
  Serial.print ("   ");
  Serial.println (velB);
  oldpositionRight = newpositionRight;
  oldpositionLeft = newpositionLeft;
  oldtime = newtime;
  delay(1000);

  max_en = max(encoderPosRight, encoderPosLeft);

  /*if (encoderPosRight > 300)
    {
    if (old_err < err)
      lp=lp+5;
    else if (old_err > err)
      lp=lp-5;
    old_err = err;
    }
  */

  if (max_en == encoderPosLeft)
  {
    Serial.println(" error A = " + (String)((encoderPosLeft - encoderPosRight) / Kt) );
    err = (encoderPosLeft - encoderPosRight) / Kt ;

    while (encoderPosRight <= max_en)
    {
      digitalWrite(Bin_1, LOW) ;
      digitalWrite(Bin_2, HIGH) ;
      analogWrite(Bpwm, errB + hp + (encoderPosLeft - encoderPosRight) / Kt ) ;
      digitalWrite(Ain_1, LOW) ;
      digitalWrite(Ain_2, HIGH) ;
      analogWrite(Apwm, lp - (encoderPosLeft - encoderPosRight) / Kt ) ;
    }
  }
  else if (max_en == encoderPosRight)
  {
    //For Anti Clock-wise motion - IN_1 = LOW , IN_2 = HIGH
    Serial.println(" error B = " + (String)((encoderPosRight - encoderPosLeft) / Kt) );
    err = (encoderPosRight - encoderPosLeft) / Kt ;

    while (encoderPosLeft <= max_en)
    {
      digitalWrite(Ain_1, LOW) ;
      digitalWrite(Ain_2, HIGH) ;
      analogWrite(Apwm, hp + (encoderPosRight - encoderPosLeft) / Kt) ;
      digitalWrite(Bin_1, LOW) ;
      digitalWrite(Bin_2, HIGH) ;
      analogWrite(Bpwm, errB + lp - (encoderPosRight - encoderPosLeft) / Kt) ;

    }
  }
  else
  {
    digitalWrite(Bin_1, LOW) ;
    digitalWrite(Bin_2, HIGH) ;
    analogWrite(Bpwm, errB + hp) ;
    digitalWrite(Ain_1, LOW) ;
    digitalWrite(Ain_2, HIGH) ;
    analogWrite(Apwm, hp) ;
  }

  /*Input = velA;
    myPID.Compute();
    = Output;
    analogWrite(PIN_OUTPUT, Bpwm);
  */
  /*Input = analogRead(PIN_INPUT);
    myPID.Compute();
    analogWrite(PIN_OUTPUT, Output);
  */

}


void doEncoderA()
{
  encoderPosRight++;
  //Serial.println (encoderPos);
}
void doEncoderB()
{
  encoderPosLeft++;
  //Serial.println (encoderPos);
}

