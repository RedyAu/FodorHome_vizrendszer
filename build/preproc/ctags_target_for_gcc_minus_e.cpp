# 1 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
# 1 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino"
# 2 "c:\\Users\\fodor\\OneDrive\\Dokumentumok\\_FodorHOME\\FodorHOME\\_Testing\\RS485_test\\client_1\\rs_1.ino" 2

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
    pinMode(TEPin1, 0x1);
    pinMode(TEPin2, 0x1);
    pinMode(13, 0x1);
    pinMode(RXPin, 0x2);
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
    digitalWrite(TEPin1, 0x1);
    digitalWrite(TEPin2, 0x1);
}

void sDi()
{
    digitalWrite(TEPin1, 0x0);
    digitalWrite(TEPin2, 0x0);
}
