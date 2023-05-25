/*
Written by RedyAu in 2021
Watchdog for the Kútklíma project

Changelog:
  0.1 - Code outline
  1.0 - First deployed version
*/

#define SoftwareVersion "1.0pre"

//CONFIG

const unsigned long startupGracePeriodMs = 50000;
const unsigned long resetAfterInactiveForMs = 20000;

//CONSTANTS

const int watchPin = A0;
const int resetOutputPin = 12;
const int ledPin = 13;

//PROTOTYPES
void startupGrace();
void sendReset();

//FUNCTIONS

void setup() {
  Serial.begin(9600);
  Serial.print(SoftwareVersion);
  Serial.println("\nWatchdog");

  pinMode(watchPin, INPUT);
  pinMode(resetOutputPin, OUTPUT);
  digitalWrite(resetOutputPin, HIGH);
  digitalWrite(ledPin, HIGH);

  //Ethernet shield needs a reset for some reason on powerup.
  delay(150);
  sendReset();

  startupGrace();
}


unsigned long lastChangedMillis = millis() + resetAfterInactiveForMs;
unsigned char prevWatch = digitalRead(watchPin);

void loop() {
  //Register a correct watchPin change
  unsigned char nowWatch = digitalRead(watchPin);
  if (prevWatch != nowWatch) {
    //Changed state - system is working
    lastChangedMillis = millis();
    prevWatch = nowWatch;
    Serial.print(".");
  }

  if (millis() - lastChangedMillis > resetAfterInactiveForMs) {
    Serial.println("Input hasn't changed in specified time!");
    sendReset();
    startupGrace();
  } 
}

void sendReset() {
  Serial.println("Resetting...");
  digitalWrite(resetOutputPin, LOW);
  delay(500);
  digitalWrite(resetOutputPin, HIGH);
  lastChangedMillis = millis();
}

void startupGrace() {
  digitalWrite(ledPin, LOW);
  Serial.println("Grace period to allow main unit to connect.");
  delay(startupGracePeriodMs);
  Serial.println("Grace period over.");
  digitalWrite(ledPin, HIGH);
  lastChangedMillis = millis();
}