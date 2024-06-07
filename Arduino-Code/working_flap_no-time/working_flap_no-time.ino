#include <AccelStepper.h>
const float fullRevolution = 4097.0;
const float SteppDegree = fullRevolution / 360;

const int hallPin_1 = 35;
const int hallPin_2 = 34;
const int hallPin_3 = 39;
const int hallPin_4 = 36;

bool magnetDetected = false;
int letters[11] = {6,7,8,9,10,0,1,2,3,4,5};
int letterIndex = 0;
float lastDegree = 0;
int lastNumber = 5;
float cumulativeDegree = 0;

// Pins IN1-IN3-IN2-IN4
AccelStepper stepper1(AccelStepper::HALF4WIRE, 23, 21, 22, 19);
AccelStepper stepper2(AccelStepper::HALF4WIRE, 18, 17, 5, 16);
AccelStepper stepper3(AccelStepper::HALF4WIRE, 4, 2, 0, 15);
AccelStepper stepper4(AccelStepper::HALF4WIRE, 27, 12, 14, 13);

float sensor_1, sensor_2, sensor_3, sensor_4;

void setup(void) {
  Serial.begin(115200);

  pinMode(hallPin_1, INPUT);
  /*
  pinMode(hallPin_2, INPUT);
  pinMode(hallPin_3, INPUT);
  pinMode(hallPin_4, INPUT);
  */

  setupMotors();
}

void loop(void) {
  if (!magnetDetected) {
    sensor_1 = digitalRead(hallPin_1);
    
    // If magnet detected
    if (sensor_1 == LOW) {
      magnetDetected = true;
      Serial.println("Magnet detected, position set to 0");

      stepper1.setCurrentPosition(0);
      adjustPosition(10);
      rotateToNumber(10);
    } else {
      // Keep the motor running
      stepper1.setSpeed(-1000);
      stepper1.runSpeed();
    }
  } else {
    rotateThroughAllFlaps();
    // readSerialInputToTurnFlaps();
  }
}

void setupMotors(void) {
  stepper1.setMaxSpeed(1000.0);    // set the maximum speed
  stepper1.setAcceleration(500);   // set acceleration
  stepper1.setSpeed(-800);          // set initial speed
  stepper1.setCurrentPosition(0);  // set position

  stepper2.setMaxSpeed(1000.0);    // set the maximum speed
  stepper2.setAcceleration(500);   // set acceleration
  stepper2.setSpeed(-800);          // set initial speed
  stepper2.setCurrentPosition(0);  // set position

  stepper3.setMaxSpeed(1000.0);    // set the maximum speed
  stepper3.setAcceleration(500);   // set acceleration
  stepper3.setSpeed(-800);          // set initial speed
  stepper3.setCurrentPosition(0);  // set position

  stepper4.setMaxSpeed(1000.0);    // set the maximum speed
  stepper4.setAcceleration(500);   // set acceleration
  stepper4.setSpeed(-800);          // set initial speed
  stepper4.setCurrentPosition(0);  // set position
}

float calcStep(float degree) {
  float direction = -1;  // -1 => clockwise, 1 => counterclockwise
  return degree * SteppDegree * direction;
}

void adjustPosition(int degreeToAdjustBy) {
  stepper1.moveTo(calcStep(degreeToAdjustBy));
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();
  }
  stepper1.setCurrentPosition(0);
}

void rotateThroughAllFlaps() {
  while(letterIndex < 11) {
    rotateToNumber(letterIndex);
    
    letterIndex++;
    delay(1000);
  }
  
  while(1) {} // Stop motor from moving.
}

void rotateToNumber(int number) {
  if(number < 11) {
    Serial.print("Moving to: ");
    Serial.println(number);

    int numberToTurnBy = number - lastNumber;
    if(numberToTurnBy < 0) {
      numberToTurnBy = 11 + numberToTurnBy;
    }

    float degree = lastDegree + (360.0 / 11 * numberToTurnBy);

    lastDegree = degree;
    lastNumber = number;

    stepper1.moveTo(calcStep(degree));
    while (stepper1.distanceToGo() != 0) {
      stepper1.run();
    }
  }
}

void readSerialInputToTurnFlaps() {
  Serial.println("Show numbers 0-9 by entering the number.");
  Serial.println("Show empty flap by entering a space");
  Serial.println("");

  while(1) {
      if (Serial.available() > 0) {
        int input = Serial.read();
        if(input >= 48 && input <= 57) {
          rotateToNumber(input-48);
        }
        else if(input == 32) {
          rotateToNumber(10);
        }
      }
    }
}

