#include <AccelStepper.h>
#include <map>
#include <WiFi.h>
#include "time.h"

/*** TIME-VARIABLES ***/
const char *ssid = "WLAN-Kabel Gast";  // FH-Kiel-IoT-NAT
const char *password = "sandman1998";  // !FH-NAT-001!
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
const char *berlinTimezone = "CET-1CEST,M3.5.0,M10.5.0/3";

/*** MOTOR-VARIABLES ***/
const float fullRevolution = 4097.0;
const float SteppDegree = fullRevolution / 360;
AccelStepper stepper1(AccelStepper::HALF4WIRE, 23, 21, 22, 19);  // Pins IN1-IN3-IN2-IN4
AccelStepper stepper2(AccelStepper::HALF4WIRE, 18, 17, 5, 16);   // Pins IN1-IN3-IN2-IN4
AccelStepper stepper3(AccelStepper::HALF4WIRE, 4, 2, 0, 15);     // Pins IN1-IN3-IN2-IN4
AccelStepper stepper4(AccelStepper::HALF4WIRE, 27, 12, 14, 13);  // Pins IN1-IN3-IN2-IN4

/*** SENSOR-VARIABLES ***/
float sensor_1, sensor_2, sensor_3, sensor_4;
bool sensor_1_magnet, sensor_2_magnet, sensor_3_magnet, sensor_4_magnet;
const int hallPin_1 = 35;
const int hallPin_2 = 34;
const int hallPin_3 = 39;
const int hallPin_4 = 36;

/*** BUTTONS-VARIABLES ***/
int buttonPin = 32;
int buttonValue = 0;

int powerButton = 2850;
int modeButton = 720;
int startButton = 1860;
int tolerance = 100;

/*** LED ***/
int led1 = 33;

/*** GENERAL-VARIABLES ***/
int maxSpeed = 1000;
int acceleration = 500;
int speed = -400;
int startPosition = 0;
std::map<String, int> motorNumbers = {
  { "HoursLeft", 0 },
  { "HoursRight", 0 },
  { "MinutesLeft", 0 },
  { "MinutesRight", 0 }
};

int letters[11] = { 6, 7, 8, 9, 10, 0, 1, 2, 3, 4, 5 };
int letterIndex = 0;
std::map<String, float> lastDegree = {
  { "HoursLeft", 0 },
  { "HoursRight", 0 },
  { "MinutesLeft", 0 },
  { "MinutesRight", 0 }
};
std::map<String, int> lastNumber = {
  { "HoursLeft", 5 },
  { "HoursRight", 5 },
  { "MinutesLeft", 5 },
  { "MinutesRight", 5 }
};
float cumulativeDegree = 0;
long timeLastChecked = 0;
int timeInterval = 5000;
AccelStepper *reversedSteppers[] = { &stepper2 };

// TODO or not: Overflow weil zu hoch gezählt -> vlt. reset einmal am Tag
// Float Overflow nach 1.960716 x 10^31 Jahre, Int Overflow nach 123.75 Jahre

// TODO: Stoppuhr, durch Modi auswählbar

void setup(void) {
  Serial.begin(115200);

  pinMode(hallPin_1, INPUT);
  pinMode(hallPin_2, INPUT);
  pinMode(hallPin_3, INPUT);
  pinMode(hallPin_4, INPUT);

  pinMode(led1, OUTPUT);

  setupMotors();
  setupWifi();
}

void loop(void) {
  buttonValue = analogRead(buttonPin);
  
  int pressedButton = buttonPressed();

  if (pressedButton == 1) {
    digitalWrite(led1, HIGH);
  } else if (pressedButton == 3) {
    digitalWrite(led1, HIGH);
  }
  else {
    digitalWrite(led1, LOW);
  }

  if (!sensor_1_magnet) {
    sensor_1 = digitalRead(hallPin_1);
    calibrateMotor(&stepper1, &sensor_1, &sensor_1_magnet);
  }
  else if (!sensor_2_magnet) {
    sensor_2 = digitalRead(hallPin_2);
    calibrateMotor(&stepper2, &sensor_2, &sensor_2_magnet);
  }
  else if(!sensor_3_magnet) {
    sensor_3 = digitalRead(hallPin_3);
    calibrateMotor(&stepper3, &sensor_3, &sensor_3_magnet);
  }
  else if(!sensor_4_magnet) {
    sensor_4 = digitalRead(hallPin_4);
    calibrateMotor(&stepper4, &sensor_4, &sensor_4_magnet);
  } 
  else {
    /* Alternative Funktionen */
    // rotateThroughAllFlaps();
    // readSerialInputToTurnFlaps();

    if (millis() - timeLastChecked > timeInterval) {
      updateTime();
      timeLastChecked = millis();
    } else {
      //rotatingFlaps(&stepper1, -speed);
      rotateToNumber(&stepper4, "MinutesRight");
      rotateToNumber(&stepper3, "MinutesLeft");
      rotateToNumber(&stepper2, "HoursRight");
      rotateToNumber(&stepper1, "HoursLeft");

    }
  }
}

void setupWifi() {
  /*          TIME SERVER         */
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setTimezone(berlinTimezone);
  updateTime();
}

void setupMotors() {
  stepper1.setMaxSpeed(maxSpeed);
  stepper1.setAcceleration(acceleration);
  stepper1.setSpeed(speed);
  stepper1.setCurrentPosition(startPosition);

  stepper2.setMaxSpeed(maxSpeed);
  stepper2.setAcceleration(acceleration);
  stepper2.setSpeed(speed);
  stepper2.setCurrentPosition(startPosition);

  stepper3.setMaxSpeed(maxSpeed);
  stepper3.setAcceleration(acceleration);
  stepper3.setSpeed(speed);
  stepper3.setCurrentPosition(startPosition);

  stepper4.setMaxSpeed(maxSpeed);
  stepper4.setAcceleration(acceleration);
  stepper4.setSpeed(speed);
  stepper4.setCurrentPosition(startPosition);
}

int buttonPressed() {
  if(buttonValue <= powerButton + tolerance && buttonValue >= powerButton - tolerance) return 1;
  if(buttonValue <= modeButton + tolerance && buttonValue >= modeButton - tolerance) return 2;
  if(buttonValue <= startButton + tolerance && buttonValue >= startButton - tolerance) return 3;
  return -1;
}

void calibrateMotor(AccelStepper *stepper, float *sensor, bool *sensor_magnet) {
  if (*sensor == LOW) {
    *sensor_magnet = true;
    Serial.println("Magnet detected, position set to 0");
    stepper->setCurrentPosition(0);

    if (isStepperInList(stepper)) {
      adjustPosition(stepper, -33);  // turn backwards, if stepper==stepper4
    } else if (stepper == &stepper4) {
      adjustPosition(stepper, 30);
    } else {
      adjustPosition(stepper, 15);
    } 
  } else {
    // Keep the motor running
    if (isStepperInList(stepper)) {
      stepper->setSpeed(-speed * 4);  // turn backwards, if stepper==stepper4
    } else {
      stepper->setSpeed(speed * 4);
    }

    stepper->runSpeed();
  }
}

float calcStep(float degree) {
  float direction = -1;  // -1 => clockwise, 1 => counterclockwise
  return degree * SteppDegree * direction;
}

void adjustPosition(AccelStepper *stepper, int degreeToAdjustBy) {
  stepper->moveTo(calcStep(degreeToAdjustBy));
  while (stepper->distanceToGo() != 0) {
    stepper->run();
  }
  stepper->setCurrentPosition(0);
}

bool isStepperInList(AccelStepper *stepper) {
  for (int i = 0; i < sizeof(reversedSteppers) / sizeof(reversedSteppers[0]); i++) {
    if (stepper == reversedSteppers[i]) {
      return true;
    }
  }
  return false;
}


/*** Functions for time ***/

void setTimezone(const char *timezone) {
  Serial.print("Setting Timezone to ");
  Serial.println(timezone);
  setenv("TZ", timezone, 1);
  tzset();
}

void updateTime() {
  struct tm timeinfo;
  char timeHour[3], timeMinute[3];

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println("Updating time");

  strftime(timeHour, 3, "%H", &timeinfo);
  strftime(timeMinute, 3, "%M", &timeinfo);

  motorNumbers["HoursLeft"] = timeHour[0] - '0';
  motorNumbers["HoursRight"] = timeHour[1] - '0';
  motorNumbers["MinutesLeft"] = timeMinute[0] - '0';
  motorNumbers["MinutesRight"] = timeMinute[1] - '0';
}

/*** Functions to turn flaps ***/

void rotateToNumber(AccelStepper *stepper, String position) {
  int number = motorNumbers[position];
  if (number < 11) {
    Serial.print("Moving to: ");
    Serial.println(number);

    int numberToTurnBy = number - lastNumber[position];
    if (numberToTurnBy < 0) {
      numberToTurnBy = 11 + numberToTurnBy;
    }

    float degree = lastDegree[position] + (360.0 / 11 * numberToTurnBy);
    lastDegree[position] = degree;
    lastNumber[position] = number;

    if (isStepperInList(stepper)) {
      stepper->moveTo(-calcStep(degree));  // turn backwards, if stepper==stepper4
    } else {
      stepper->moveTo(calcStep(degree));
    }

    while (stepper->distanceToGo() != 0) {
      stepper->run();
    }
  }
}

void rotatingFlaps(AccelStepper *stepper, int speed) {
  if (isStepperInList(stepper)) {
      stepper->setSpeed(speed);  // turn backwards, if stepper==stepper4
    } else {
      stepper->setSpeed(-speed);
    }

    stepper->runSpeed();
}

/*
void rotateThroughAllFlaps() {
  while(letterIndex < 11) {
    rotateToNumber(&stepper1, letterIndex);
    
    letterIndex++;
    delay(1000);
  }
  
  // while(1) {} // Stop motor from moving.
}

void readSerialInputToTurnFlaps() {
  if (Serial.available() > 0) {
    int input = Serial.read();
    if(input >= 48 && input <= 57) {
      rotateToNumber(&stepper1, input-48);
    }
    else if(input == 32) {
      rotateToNumber(&stepper1, 10);
    }
  }
}
*/
