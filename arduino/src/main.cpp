#include <Arduino.h>

const int motor1Pin1 = 22;
const int motor1Pin2 = 24;
const int motor1Enable = 40;
const int motor2Pin3 = 26;
const int motor2Pin4 = 28;
const int motor2Enable = 42;

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor1Enable, OUTPUT);
  pinMode(motor2Pin3, OUTPUT);
  pinMode(motor2Pin4, OUTPUT);
  pinMode(motor2Enable, OUTPUT);

  Serial.begin(9600);

}

void moveMotor(int motor1A, int motor1B, int motor2A, int motor2B, int duration) {
  digitalWrite(motor1Pin1, motor1A);
  digitalWrite(motor1Pin2, motor1B);
  digitalWrite(motor2Pin3, motor2A);
  digitalWrite(motor2Pin4, motor2B);
  analogWrite(motor1Enable, 255);
  analogWrite(motor2Enable, 255);
  if (duration > 0) {
    delay(duration);
    analogWrite(motor1Enable, 0);
    analogWrite(motor2Enable, 0);
  }
}

void loop() {
  moveMotor(HIGH, LOW, LOW, HIGH, 500); //F
  moveMotor(LOW, HIGH, HIGH, LOW, 500); //B
  moveMotor(LOW, LOW, HIGH, LOW, 400);  //R
  moveMotor(LOW, HIGH, LOW, LOW, 400);  //L
}
