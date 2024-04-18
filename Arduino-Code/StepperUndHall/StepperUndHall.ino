#include <AccelStepper.h>
const byte Fullstep = 4;
const byte Halfstep = 8;
const short fullRevolution = 2038;
const float SteppDegree = 11.32;  // Halfstep 11.32 - Fullstep 5.66

const int hallPin_1 = 36;
const int hallPin_2 = 39;
const int hallPin_3 = 34;
const int hallPin_4 = 35;

const int buttonPin_1 = 3;

const int ledPin_1 = 1;

int counter = 0;
// Pins IN1-IN3-IN2-IN4
AccelStepper stepper1(Halfstep, 4, 2, 0, 15);
AccelStepper stepper2(Halfstep, 18, 17, 5, 16);
AccelStepper stepper3(Halfstep, 23, 21, 22, 19);
AccelStepper stepper4(Halfstep, 27, 12, 14, 13);


void setup(void) {
  setupMotors();

  pinMode(ledPin_1, OUTPUT);

  pinMode(buttonPin_1, INPUT);

  pinMode(hallPin_1, INPUT);
  pinMode(hallPin_2, INPUT);
  pinMode(hallPin_3, INPUT);
  pinMode(hallPin_4, INPUT);
  Serial.begin(9600);

  Serial.print("Counter: ");
  Serial.println(counter);
}

void loop(void) {

  Serial.print("Button 1: ");
  Serial.println(buttonPin_1);

  digitalWrite(ledPin_1, HIGH);
  delay(500);
  digitalWrite(ledPin_1, LOW);
  delay(500);


  // moveMotors();

  float sensor_1 = digitalRead(hallPin_1);
  float sensor_2 = digitalRead(hallPin_2);
  float sensor_3 = digitalRead(hallPin_3);
  float sensor_4 = digitalRead(hallPin_4);
  
  /*
  if (sensor_1 == LOW) {
    // Der Counter-Wert bestimmt den Winkel den ein Motor annimmt.
    counter += 90;
    Serial.print("Counter: ");
    Serial.println(counter);
    delay(500);
  }
  */
}

void setupMotors(void) {
  stepper1.setMaxSpeed(1000.0);    // set the maximum speed
  stepper1.setAcceleration(500);   // set acceleration
  stepper1.setSpeed(800);          // set initial speed
  stepper1.setCurrentPosition(0);  // set position

  stepper2.setMaxSpeed(1000.0);    // set the maximum speed
  stepper2.setAcceleration(500);   // set acceleration
  stepper2.setSpeed(800);          // set initial speed
  stepper2.setCurrentPosition(0);  // set position

  stepper3.setMaxSpeed(1000.0);    // set the maximum speed
  stepper3.setAcceleration(500);   // set acceleration
  stepper3.setSpeed(800);          // set initial speed
  stepper3.setCurrentPosition(0);  // set position

  stepper4.setMaxSpeed(1000.0);    // set the maximum speed
  stepper4.setAcceleration(500);   // set acceleration
  stepper4.setSpeed(800);          // set initial speed
  stepper4.setCurrentPosition(0);  // set position
}

void moveMotors(void) {
  stepper1.moveTo(calcStep(-counter));
  stepper1.run();

  stepper2.moveTo(calcStep(counter));
  stepper2.run();

  stepper3.moveTo(calcStep(counter));
  stepper3.run();

  stepper4.moveTo(calcStep(-counter));
  stepper4.run();
}

int calcStep(int degree) {
  int direction = -1;  // -1 => clockwise, 1 => counterclockwise
  return degree * SteppDegree * direction;
}