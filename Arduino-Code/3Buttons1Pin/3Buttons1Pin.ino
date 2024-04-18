int sensorPin = 32;   // select the input pin for the potentiometer
int sensorValue = 0;

int r_button_1 = 0;
int r_button_2 = 235;
int r_button_3 = 535;
int r_toleranz = 10;

int led_1 = 15;
int led_2 = 2;
int led_3 = 0;

void setup() {
  Serial.begin(9600);

  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(led_3, OUTPUT);
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);

  int pressedButton = buttonPressed();

  if(pressedButton == -1) {
    digitalWrite(led_1, LOW);
    digitalWrite(led_2, LOW);
    digitalWrite(led_3, LOW);
  }
  else {
    digitalWrite(pressedButton,HIGH);
  }

  delay(100);
}

int buttonPressed() {
  if(sensorValue <= r_button_1 + r_toleranz && sensorValue >= r_button_1 - r_toleranz) return led_1;
  if(sensorValue <= r_button_2 + r_toleranz && sensorValue >= r_button_2 - r_toleranz) return led_2;
  if(sensorValue <= r_button_3 + r_toleranz && sensorValue >= r_button_3 - r_toleranz) return led_3;
  return -1;
}
