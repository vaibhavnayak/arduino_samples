
#include <PID_v1.h>

#define PIN_INPUT 2
#define PIN_OUTPUT 3
#define LENGTH 100      //in cm
#define BREADTH 100     //in cm
#define LATERAL 10      //disance covered by the bot on a right turn at the end of traversing a column

int Kt = 10;
int err = 0 , old_err , errB = 0;
int hp = 200;
int lp = 200;

int motor_distace = 0;

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
  int i = 0;

  for (; i <= LENGTH / LATERAL; i++) {

    motor_distace = 0;

    encoderPosRight = 0;
    encoderPosLeft = 0;

    while (motor_distace < BREADTH)
    {
      forward();
    }

    digitalWrite(Ain_1, HIGH) ;
    digitalWrite(Ain_2, HIGH) ;
    analogWrite(Apwm, hp) ;

    digitalWrite(Bin_1, HIGH) ;
    digitalWrite(Bin_2, HIGH) ;
    analogWrite(Bpwm, hp) ;
    delay(100);

    if ((i%2) == 0) {       //even

      rightTurn();
      // rightTurn();

      digitalWrite(Ain_1, HIGH) ;
      digitalWrite(Ain_2, HIGH) ;
      analogWrite(Apwm, hp) ;

      digitalWrite(Bin_1, HIGH) ;
      digitalWrite(Bin_2, HIGH) ;
      analogWrite(Bpwm, hp) ;
      delay(100);

    }

    else {              //odd

      leftTurn();
      //leftTurn();

      digitalWrite(Ain_1, HIGH) ;
      digitalWrite(Ain_2, HIGH) ;
      analogWrite(Apwm, hp) ;

      digitalWrite(Bin_1, HIGH) ;
      digitalWrite(Bin_2, HIGH) ;
      analogWrite(Bpwm, hp) ;
      delay(100);
    }
  }
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

int forward() {

  //TODO: Move the bot forward and compute distance travelled updating global variable "motor_distance"
  //For Clock wise motion , in_1 = High , in_2 = Low
  // Serial.print (analogRead(PIN_INPUT));
  //detachInterrupt(0);
  //detachInterrupt(1);
  max_en = max(encoderPosRight, encoderPosLeft);

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

  motor_distace = (encoderPosRight + encoderPosLeft) / 5;
  // attachInterrupt(0, doEncoderA, RISING);
  // attachInterrupt(1, doEncoderB, RISING);

}

void leftTurn() {

  //TODO: Make the bot take a left turn
  encoderPosRight = 0;
  encoderPosLeft = 0;
  while (encoderPosRight < 115)
  {
    digitalWrite(Ain_1, LOW) ;
    digitalWrite(Ain_2, HIGH) ;
    analogWrite(Apwm, hp) ;
  }
  digitalWrite(Ain_1, HIGH) ;
  digitalWrite(Ain_2, HIGH) ;
  analogWrite(Apwm, hp) ;
}

void rightTurn() {

  //TODO: Make the bot take a right turn
  encoderPosRight = 0;
  encoderPosLeft = 0;
  while (encoderPosLeft < 115)
  {
    digitalWrite(Bin_1, LOW) ;
    digitalWrite(Bin_2, HIGH) ;
    analogWrite(Bpwm, errB + hp) ;
  }
  digitalWrite(Bin_1, HIGH) ;
  digitalWrite(Bin_2, HIGH) ;
  analogWrite(Bpwm, errB + hp) ;
}

