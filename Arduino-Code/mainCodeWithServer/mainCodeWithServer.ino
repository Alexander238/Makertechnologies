#include <AccelStepper.h>
#include <map>
#include <WiFi.h>
#include "time.h"
#include <WebServer.h>

/*** TIME-VARIABLES ***/
const char *ssid = "ESP32-Access-Point";  // Access Point SSID
const char *password = "123456789";       // Access Point Password
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
char currentTimezone[100] = "CET-1CEST,M3.5.0,M10.5.0/3"; // Initial timezone

// Berlin timezone: "CET-1CEST,M3.5.0,M10.5.0/3"

/*** MOTOR-VARIABLES ***/
const float fullRevolution = 4097.0;
const float SteppDegree = fullRevolution / 360;
AccelStepper stepper1(AccelStepper::HALF4WIRE, 23, 21, 22, 19);
AccelStepper stepper2(AccelStepper::HALF4WIRE, 18, 17, 5, 16);
AccelStepper stepper3(AccelStepper::HALF4WIRE, 4, 2, 0, 15);
AccelStepper stepper4(AccelStepper::HALF4WIRE, 27, 12, 14, 13);

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

// Create an instance of the web server on port 80
WebServer server(80);

// HTML page with form to update timezone
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>FLAP-DISPLAY</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
    <h1>ESP32 Web Server</h1>
    <form action="/update">
        <label for="timezone">Gebe deinen Zeitzone ein:</label><br>
        <input type="text" id="timezone" name="timezone"><br>
        <input type="submit" value="Submit">
    </form>
    <p>Current Timezone: %TIMEZONE%</p>
</body>
</html>
)rawliteral";

// Replaces placeholder with the current timezone
String processor(const String& var) {
  if (var == "TIMEZONE") {
    return String(currentTimezone);
  }
  return String();
}

// Handle the root route
void handleRoot() {
  String htmlPage = HTML_PAGE;
  htmlPage.replace("%TIMEZONE%", currentTimezone);
  server.send(200, "text/html", htmlPage);
}

// Handle form submission to update timezone
void handleUpdateTimezon() {
  if (server.hasArg("timezone")) {
    String newTimezone = server.arg("timezone");
    newTimezone.toCharArray(currentTimezone, 50);
    setTimezone(currentTimezone);
    updateTime();
  }
  handleRoot();
}

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
  setupAccessPoint();
  
  // Define routes for the web server
  server.on("/", handleRoot);
  server.on("/update", handleUpdateTimezone);
  
  // Start the web server
  server.begin();
}

void loop(void) {
  buttonValue = analogRead(buttonPin);
  
  int pressedButton = buttonPressed();

  if (pressedButton == 1) {
    digitalWrite(led1, HIGH);
  } else if (pressedButton == 3) {
    digitalWrite(led1, HIGH);
  } else {
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
    if (millis() - timeLastChecked > timeInterval) {
      updateTime();
      timeLastChecked = millis();
    } else {
      rotateToNumber(&stepper4, "MinutesRight");
      rotateToNumber(&stepper3, "MinutesLeft");
      rotateToNumber(&stepper2, "HoursRight");
      rotateToNumber(&stepper1, "HoursLeft");
    }
  }
  
  
  // This is used to handle all client requests.
  server.handleClient();
}

void setupAccessPoint() {
  Serial.print("Setting up AP with SSID: ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
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
  if(buttonValue <= startButton + tolerance && button
