#include "RS485_protocol.h"

const int TEPin1 = 9;
const int TEPin2 = 8;
const int RXPin = 0;
const int BaudR = 9600;

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

const unsigned char msgOnS[] = "<LEDON>";
const unsigned char msgOffS[] = "<LEDOFF>";
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