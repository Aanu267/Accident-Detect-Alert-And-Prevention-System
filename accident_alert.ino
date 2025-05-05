#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Pins
#define EYE_BLINK_PIN 2
#define ALCOHOL_SENSOR A0
#define BUZZER_PIN 9
#define MOTOR_IN1 7
#define MOTOR_IN2 8
#define MOTOR_EN 10

#define X_AXIS A1
#define Y_AXIS A2
#define Z_AXIS A3

unsigned long eyeClosedStartTime = 0;
const int alcoholThreshold = 400;
const int shockThreshold = 250;

// GPS
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // TX, RX

void setup() {
  pinMode(EYE_BLINK_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);

  Serial.begin(9600);
  gpsSerial.begin(9600);

  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 255);
}

void loop() {
  // Read GPS data
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  int eyeState = digitalRead(EYE_BLINK_PIN);
  int alcoholValue = analogRead(ALCOHOL_SENSOR);
  int xVal = analogRead(X_AXIS);
  int yVal = analogRead(Y_AXIS);
  int zVal = analogRead(Z_AXIS);

  Serial.print("Alcohol: "); Serial.print(alcoholValue);
  Serial.print(" | Eye: "); Serial.print(eyeState);
  Serial.print(" | X: "); Serial.print(xVal);
  Serial.print(" | Y: "); Serial.print(yVal);
  Serial.print(" | Z: "); Serial.println(zVal);

  // Display GPS location
  if (gps.location.isValid()) {
    Serial.print("GPS Location: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Waiting for GPS...");
  }

  if (alcoholValue > alcoholThreshold) {
    stopMotorAndBuzz("Alcohol Detected");
  } else if (eyeState == LOW) {
    if (eyeClosedStartTime == 0) {
      eyeClosedStartTime = millis();
    } else if (millis() - eyeClosedStartTime >= 3000) {
      stopMotorAndBuzz("Eyes Closed > 3 sec");
    }
  } else if (isAccident(xVal, yVal, zVal)) {
    stopMotorAndBuzz("Accident Detected");
    if (gps.location.isValid()) {
      Serial.print("Accident Location: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(", ");
      Serial.println(gps.location.lng(), 6);
    }
  } else {
    eyeClosedStartTime = 0;
    runMotor();
  }

  delay(100);
}

bool isAccident(int x, int y, int z) {
  if (abs(x - 512) > shockThreshold || abs(y - 512) > shockThreshold || abs(z - 512) > shockThreshold) {
    return true;
  }
  return false;
}

void stopMotorAndBuzz(String reason) {
  Serial.println(reason);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 0);
}

void runMotor() {
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 255);
}
