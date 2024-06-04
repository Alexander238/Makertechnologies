#include <AccelStepper.h>
#include <map>
#include <WiFi.h>
#include "time.h"

const char* ssid     = "Alex-Netzwerk";
const char* password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

const char* berlinTimezone = "CET-1CEST,M3.5.0,M10.5.0/3";


const byte Halfstep = 8;
const float SteppDegree = 11.32;  // 11.32 because of halfstep

const int hallPin_1 = 36;
const int hallPin_2 = 39;
const int hallPin_3 = 34;
const int hallPin_4 = 35;

int counter = 0;

std::map<String, int> motorDegrees = {
    {"HoursLeft", 0},
    {"HoursRight", 0},
    {"MinutesLeft", 0},
    {"MinutesRight", 0}
};

int maxSpeed = 1000;
int acceleration = 500;
int speed = 800;
int startPosition = 0;

// Pins IN1-IN3-IN2-IN4
AccelStepper stepper1(Halfstep, 4, 2, 0, 15);
AccelStepper stepper2(Halfstep, 18, 17, 5, 16);
AccelStepper stepper3(Halfstep, 23, 21, 22, 19);
AccelStepper stepper4(Halfstep, 27, 12, 14, 13);

void setup(void) {
  setupMotors();
  Serial.begin(9600);

  pinMode(hallPin_1, INPUT);
  pinMode(hallPin_2, INPUT);
  pinMode(hallPin_3, INPUT);
  pinMode(hallPin_4, INPUT);


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
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void setupMotors(void) {
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

void loop(void) {
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

  delay(1000);
  printLocalTime();
}

void moveMotors(void) {
  stepper1.moveTo(calcStep(motorDegrees["HoursLeft"]));
  stepper1.run();

  stepper2.moveTo(calcStep(motorDegrees["HoursRight"]));
  stepper2.run();

  stepper3.moveTo(calcStep(motorDegrees["MinutesLeft"]));
  stepper3.run();

  stepper4.moveTo(calcStep(motorDegrees["MinutesRight"]));
  stepper4.run();
}

// Calculate how far a motor should spin.
int calcStep(int degree) {
  int direction = -1;  // -1 => clockwise, 1 => counterclockwise
  return degree * SteppDegree * direction;
}

void setTimezone(const char * timezone) {
  Serial.print("Setting Timezone to ");
  Serial.println(timezone);
  setenv("TZ", timezone, 1);
  tzset();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMinute[3];
  strftime(timeMinute,3, "%M", &timeinfo);

  Serial.println(timeHour);
  Serial.println(timeMinute);

  Serial.println();
}