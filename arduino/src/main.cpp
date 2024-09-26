#include <SPI.h>
#include <MFRC522.h>
#include <cppQueue.h>
#include <LiquidCrystal_I2C.h>

//RFID sensor
#define SS_PIN 53
#define RST_PIN 5
#define MAX_QUEUE_SIZE 50

#define BLACK 1
#define WHITE 0

MFRC522 rfid(SS_PIN, RST_PIN);
cppQueue queue(sizeof(int), MAX_QUEUE_SIZE, FIFO, false);
LiquidCrystal_I2C lcd(0x27, 16, 4);

struct UID {
  byte uid[4];
  const char* tagName;
  const char* direction;
  int dirCode;
};

UID authorizedUIDs[] = {
  { { 0xB9, 0x55, 0xF8, 0x0D }, "Tag 1", "Initialize Queue", 0 },
  { { 0x07, 0xE5, 0x97, 0xC6 }, "Tag 2", "Start", 0 },
  { { 0xF3, 0xF5, 0x8B, 0x0D }, "Tag 3", "Forward", 1 },
  { { 0xD3, 0x3C, 0x8A, 0x1D }, "Tag 4", "Backward", 2 },
  { { 0xF3, 0xA1, 0x16, 0x12 }, "Tag 5", "Right", 3 },
  { { 0x03, 0x3B, 0x78, 0x36 }, "Tag 6", "Left", 4 },
  { { 0xF3, 0xFA, 0x1F, 0x35 }, "Tag 7", "Single Dequeue", 0 }
};

const int motor1Pin1 = 22;
const int motor1Pin2 = 24;
const int motor1Enable = 40;
const int motor2Pin3 = 26;
const int motor2Pin4 = 28;
const int motor2Enable = 42;
// const int buttonPin = 30;
const int ledRfid = 32;
const int modeSwitchPin = 30;

//Line sensor
const int IRL = 2;
const int IRR = 3;

//RFID stack related
int dirCount = 0;
int numDir = 0;
bool initQueue = false;
bool startDequeue = false;

//Default mode
bool rfidMode = true;

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
  // pinMode(buttonPin, INPUT);
  pinMode(modeSwitchPin, INPUT);
  pinMode(IRL, INPUT);
  pinMode(IRR, INPUT);
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  Serial.println("Robot Ready!");
  Serial.println("Default Mode: RFID");
  lcd.setCursor(0, 0);
  lcd.print("Robot Ready!");
  lcd.setCursor(0, 1);
  lcd.print("Default Mode: RFID");
  delay(2000);
  lcd.clear();
}

void printToLCD(int row, String message) {
  lcd.clear();
  lcd.setCursor(0, row);
  lcd.print(message);
}


bool isAuthorized(byte* uid) {
  for (int i = 0; i < sizeof(authorizedUIDs) / sizeof(UID); i++) {
    if (memcmp(uid, authorizedUIDs[i].uid, 4) == 0) {
      return true;
    }
  }
  return false;
}

void handleMovementTag(int tagIndex) {
  digitalWrite(ledRfid, HIGH);
  Serial.println("Authorized " + String(authorizedUIDs[tagIndex].tagName));
  Serial.println("Add " + String(authorizedUIDs[tagIndex].direction));
  printToLCD(2, "Authorized " + String(authorizedUIDs[tagIndex].tagName));
  printToLCD(3, "Add " + String(authorizedUIDs[tagIndex].direction));
  numDir = authorizedUIDs[tagIndex].dirCode;
  delay(1000);
  queue.push(&numDir);
  printToLCD(2, "Pushed to stack");
  digitalWrite(ledRfid, LOW);
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

void handleMovement(int direction) {
  switch (direction) {
    case 1:
      Serial.println("Moving Forward...");
      lcd.clear();
      // displayQueue();
      printToLCD(2, "Moving Forward...");
      moveMotor(HIGH, LOW, LOW, HIGH, 500);
      lcd.clear();
      break;
    case 2:
      Serial.println("Moving Backward...");
      lcd.clear();
      displayQueue();
      printToLCD(2, "Moving Backward...");
      moveMotor(LOW, HIGH, HIGH, LOW, 500);
      lcd.clear();
      break;
    case 3:
      Serial.println("Moving Right...");
      lcd.clear();
      displayQueue();
      printToLCD(2, "Moving Right...");
      moveMotor(LOW, LOW, HIGH, LOW, 400);
      moveMotor(HIGH, LOW, LOW, LOW, 300);
      lcd.clear();
      break;
    case 4:
      Serial.println("Moving Left...");
      lcd.clear();
      displayQueue();
      printToLCD(2, "Moving Left...");
      moveMotor(LOW, HIGH, LOW, LOW, 400);
      moveMotor(LOW, LOW, LOW, HIGH, 300);
      lcd.clear();
      break;
    default:
      break;
  }
  Serial.println("Done");
  dirCount++;
  delay(1000);
}

void objectDetection() {
  printToLCD(0, "Waiting for object...");
  if (Serial.available() > 0) {

    char command = Serial.read();

    switch (command) {
      case 'a':
        printToLCD(0, "Object a detected!");
        delay(1000);
        printToLCD(0, "Moving Forward...");  
        lcd.clear();
        moveMotor(HIGH, LOW, LOW, HIGH, 500);
        break;
      case 'b':
        printToLCD(0, "Object b detected!");
        delay(1000);
        printToLCD(0, "Moving Backward...");
        lcd.clear();
        moveMotor(LOW, HIGH, HIGH, LOW, 500);  
        break;
      case 'c':
        printToLCD(0, "Object c detected!");
        delay(1000);
        printToLCD(0, "Moving Right...");
        lcd.clear();
        moveMotor(LOW, LOW, HIGH, LOW, 400);
        moveMotor(HIGH, LOW, LOW, LOW, 300);
        break;
      case 'd':
        printToLCD(0, "Object d detected!");
        delay(1000);
        printToLCD(0, "Moving Left...");
        lcd.clear();
        moveMotor(LOW, HIGH, LOW, LOW, 400);
        moveMotor(LOW, LOW, LOW, HIGH, 300);
        break;
      default:
        break;
    }
  }
}

//NOTE: THIS AINT GONNA WORK
//TO-DO: IMPLEMENT THE OPERATION FROM THE LIBRARY ITSELF OR CHANGE LIBRARY AND REFACTOR THE WHOLE CODE
/*void undoQueue() {
  if (!queue.isEmpty()) {
    int queueSize = queue.getCount();
    int lastItem;
    queue.peekIdx(&lastItem, queueSize - 1);  
    queue.pop(&lastItem);                     
    Serial.println("Undo done: " + String(getDirectionLetter(lastItem)));
    printToLCD(2, "Undo done: " + String(getDirectionLetter(lastItem)));
  } else {
    Serial.println("Queue is empty");
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledRfid, HIGH);
      delay(200);
      digitalWrite(ledRfid, LOW);
      delay(200);
    }
  }
}
*/

char getDirectionLetter(int dirCode) {
  switch (dirCode) {
    case 1: return 'F';  // Forward
    case 2: return 'B';  // Backward
    case 3: return 'R';  // Right
    case 4: return 'L';  // Left
    default: return ' ';
  }
}


void displayQueue() {
  int queueInt;
  int queueSize = queue.getCount();
  bool rowFull = false;

  if (!queue.isEmpty()) {
    lcd.setCursor(0, 0);
    lcd.print("Queue: ");


    for (int i = 0; i < min(queueSize, 24); i++) {
      queue.peekIdx(&queueInt, i);
      lcd.print(getDirectionLetter(queueInt));
      if (i < min(queueSize, 5) - 1) {
        lcd.print(",");
      }
    }


    if (queueSize > 25) {
      lcd.setCursor(14, 1);
      lcd.print("++");
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Queue is empty!");
  }
}

void loop() {

  if (digitalRead(modeSwitchPin) == HIGH) {
    delay(50);  // Debounce
    if (digitalRead(modeSwitchPin) == HIGH) {
      rfidMode = !rfidMode;
      Serial.println(rfidMode ? "Switched to RFID mode" : "Switched to Object Detection mode");
      printToLCD(2, "Mode:");
      if (rfidMode) {
        displayQueue();
        lcd.setCursor(0, 3);
        lcd.print("RFID");
      }
      else {
        lcd.setCursor(0, 3);
        lcd.print("Object Detection");
        delay(2000);
      }
      while (digitalRead(modeSwitchPin) == HIGH);
    }
  }

  if (rfidMode) {
    // RFID Mode
    displayQueue();
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      byte* uid = rfid.uid.uidByte;
      if (isAuthorized(uid)) {
        for (int i = 0; i < sizeof(authorizedUIDs) / sizeof(UID); i++) {
          if (memcmp(uid, authorizedUIDs[i].uid, 4) == 0) {
            if (strcmp(authorizedUIDs[i].tagName, "Tag 1") == 0) {
              initQueue = true;
              while (!queue.isEmpty()) {
                int dummy;
                queue.pop(&dummy);
              }
              Serial.println("Queue initialized and cleared");
              lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("Queue initialized");
            } else if (strcmp(authorizedUIDs[i].tagName, "Tag 2") == 0) {
              if (queue.isEmpty()) {
                Serial.println("Queue is empty. Please add movements before starting.");
                for (int j = 0; j < 5; j++) {
                  digitalWrite(ledRfid, HIGH);
                  delay(100);
                  digitalWrite(ledRfid, LOW);
                  delay(100);
                }
              } else {
                startDequeue = true;
                lcd.clear();
                lcd.setCursor(0, 2);
                lcd.print("Starting dequeue");
                Serial.println("Starting dequeue process for all movements");
              }
            } else if (strcmp(authorizedUIDs[i].tagName, "Tag 7") == 0) {
              Serial.println("Undoing queue");
              // undoQueue();
            } else if (initQueue) {
              handleMovementTag(i);
            } else {
              Serial.println("Initialize queue with the appropriate tag first!");
              lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("Not initialized!");
            }
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
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }

    if (startDequeue == true) {
      delay(800);
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledRfid, HIGH);
        delay(200);
        digitalWrite(ledRfid, LOW);
        delay(200);
      }
      if (!queue.isEmpty()) {
        while (!queue.isEmpty() && dirCount < MAX_QUEUE_SIZE) {
          int queueInt;
          queue.pop(&queueInt);
          handleMovement(queueInt);
        }
      } else {
        Serial.println("Queue is empty. No movements to perform.");
      }
      startDequeue = false;
      initQueue = false;
      dirCount = 0;
    }
  } else {
    // Line Follower Mode
    objectDetection();
  }
}