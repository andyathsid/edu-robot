#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <cppQueue.h>

#define SS_PIN 53
#define RST_PIN 5
#define MAX_QUEUE_SIZE 50

MFRC522 rfid(SS_PIN, RST_PIN);
cppQueue queue(sizeof(int), 10, FIFO, false);

struct UID {
  byte uid[4];
  const char* tagName;
  const char* direction;
  int dirCode;
};

UID authorizedUIDs[] = {
  {{0xF3, 0xF5, 0x8B, 0x0D}, "Tag 1", "Forward", 1},
  {{0xD3, 0x3C, 0x8A, 0x1D}, "Tag 2", "Backward", 2},
  {{0xF3, 0xA1, 0x16, 0x12}, "Tag 3", "Right", 3},
  {{0x03, 0x3B, 0x78, 0x36}, "Tag 4", "Left", 4}
};

const int motor1Pin1 = 22;
const int motor1Pin2 = 24;
const int motor1Enable = 40; 
const int motor2Pin3 = 26;
const int motor2Pin4 = 28;
const int motor2Enable = 42; 
const int buttonPin = 30;
const int ledRfid = 32;

int dirArray[MAX_QUEUE_SIZE];
int dirCount = 0;
int numDir = 0;

void setup() {
  SPI.begin();      
  rfid.PCD_Init();  
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor1Enable, OUTPUT); 
  pinMode(motor2Pin3, OUTPUT);
  pinMode(motor2Pin4, OUTPUT);
  pinMode(motor2Enable, OUTPUT); 
  pinMode(ledRfid, OUTPUT);
  Serial.begin(9600);
  Serial.println("Tap RFID/NFC Tag on reader");
}

bool isAuthorized(byte* uid) {
  for (int i = 0; i < sizeof(authorizedUIDs) / sizeof(UID); i++) {
    if (memcmp(uid, authorizedUIDs[i].uid, 4) == 0) {
      return true;
    }
  }
  return false;
}

void handleAuthorizedTag(int tagIndex) {
  digitalWrite(ledRfid, HIGH);
  Serial.println("Authorized " + String(authorizedUIDs[tagIndex].tagName));
  Serial.println("Add " + String(authorizedUIDs[tagIndex].direction));
  numDir = authorizedUIDs[tagIndex].dirCode;
  delay(1000);
  queue.push(&numDir);
  Serial.println("Done");
  digitalWrite(ledRfid, LOW);
}

void moveMotor(int motor1A, int motor1B, int motor2A, int motor2B, int duration) {
  digitalWrite(motor1Pin1, motor1A);
  digitalWrite(motor1Pin2, motor1B);
  digitalWrite(motor2Pin3, motor2A);
  digitalWrite(motor2Pin4, motor2B);
  analogWrite(motor1Enable, 255);
  analogWrite(motor2Enable, 255);
  delay(duration);
  analogWrite(motor1Enable, 0);
  analogWrite(motor2Enable, 0);
}

void handleMovement(int direction) {
  switch (direction) {
    case 1:
      Serial.println("Moving Forward...");
      moveMotor(LOW, HIGH, HIGH, LOW, 500);
      break;
    case 2:
      Serial.println("Moving Backward...");
      moveMotor(HIGH, LOW, LOW, HIGH, 500);
      break;
    case 3:
      Serial.println("Moving Right...");
      moveMotor(LOW, HIGH, LOW, LOW, 400);
      break;
    case 4:
      Serial.println("Moving Left...");
      moveMotor(LOW, LOW, HIGH, LOW, 400);
      break;
    default:
      break;
  }
  Serial.println("Done");
  dirCount++;
  delay(1000);
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    byte* uid = rfid.uid.uidByte;
    if (isAuthorized(uid)) {
      for (int i = 0; i < sizeof(authorizedUIDs) / sizeof(UID); i++) {
        if (memcmp(uid, authorizedUIDs[i].uid, 4) == 0) {
          handleAuthorizedTag(i);
          break;
        }
      }
    } else {
      Serial.print("Unauthorized Tag with UID:");
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledRfid, HIGH);  
        delay(500);                   
        digitalWrite(ledRfid, LOW);   
        delay(500);                   
      } 
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();
    }
    rfid.PICC_HaltA();       // halt PICC
    rfid.PCD_StopCrypto1();  // stop encryption on PCD
  }

  int buttonVal = digitalRead(buttonPin);
  // Serial.println(buttonVal);
  if (buttonVal == HIGH) {
    delay(800);
    digitalWrite(ledRfid, HIGH); 
    delay(200);
    digitalWrite(ledRfid, LOW);    
    delay(800);
    digitalWrite(ledRfid, HIGH); 
    delay(200);
    digitalWrite(ledRfid, LOW);    
    delay(800);
    digitalWrite(ledRfid, HIGH); 
    delay(200);
    digitalWrite(ledRfid, LOW);   
    while (!queue.isEmpty() && dirCount < MAX_QUEUE_SIZE) {
      int queueInt;
      queue.pop(&queueInt);
      handleMovement(queueInt);
    }
  }
}
