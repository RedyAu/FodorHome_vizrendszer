#include "Arduino.h"

void setup() {
  Serial.begin(115200);
}

void loop() {
  readSerial();
  parseData();
}


const byte messageSize = 32;
byte receivedMessage[messageSize];
const char startMarker = '<';
const char endMarker = '>';
bool newData;
byte index = 0;
bool incomingData = false;

void readSerial() {
  while (Serial.available() || !newData) {
    byte r = Serial.read();
    if (incomingData) {
      if ((char)r != endMarker) {
        receivedMessage[index] = r;
        index++;
        if (index >= messageSize) index = messageSize - 1;
      } else {
        receivedMessage[index] = '\0';
        incomingData = false;
        index = 0;
        newData = true;
      }
    } else if ((char)r == startMarker) incomingData = true;
  }
}

void parseData() {
  int index = 0;
  Serial.print("Parsing: ");
  while (index < messageSize + 1);
  {
    Serial.print((char)receivedMessage[index]);
    index++;
  }
  Serial.println();
}