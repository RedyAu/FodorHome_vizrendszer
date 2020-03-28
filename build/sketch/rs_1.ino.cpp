#include <Arduino.h>
#line 1 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
#line 1 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
#include "RS485_protocol.h"

const int TEPin1 = 9;
const int TEPin2 = 8;
const int RXPin = 0;
const int BaudR = 9600;

#line 8 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void fWrite(const byte what);
#line 10 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
int fAvailable();
#line 12 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
int fRead();
#line 14 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void setup();
#line 28 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void loop();
#line 38 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void sEn();
#line 44 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void sDi();
#line 8 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
void fWrite(const byte what) { Serial.write(what); }

int fAvailable() { Serial.available(); }

int fRead() { Serial.read(); }

void setup()
{
    Serial.begin(9600);
    pinMode(TEPin1, OUTPUT);
    pinMode(TEPin2, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(RXPin, INPUT_PULLUP);
}

const unsigned char msgOnS[] = "<LELongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageDON>";
const unsigned char msgOffS[] = "<LELongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageLongMessageDOFF>";
const byte msgOn[] = {(byte)msgOnS};
const byte msgOff[] = {(byte)msgOffS};

void loop()
{
    sEn();
    sendMsg(fWrite, msgOn, sizeof(msgOn));
    delay(1);
    sDi();

    delay(500);
}

void sEn()
{
    digitalWrite(TEPin1, HIGH);
    digitalWrite(TEPin2, HIGH);
}

void sDi() 
{
    digitalWrite(TEPin1, LOW);
    digitalWrite(TEPin2, LOW);
}
