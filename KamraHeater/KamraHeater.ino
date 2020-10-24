/* Written by Benedek Fodor
   For controlling the heater system in the chamber; as part of the KÚTKLÍMA/FodorHome System

   Versions:
   0.0: First operating version
   0.1: Implementing EEPROM
   0.2: Show version on startup. Only toggle one pin (changed relay module).
*/

const String softwareVersion = "0.2";

#include <EEPROM.h>
const int setTempA = 10;

#include <Encoder.h>
Encoder setTempEnc(2, 3);
const int middleSwitchPin = 4;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#include <DHT.h>
const int dhtPin = 5;
DHT dht(dhtPin, DHT11);

const int oLED = 13;
const int oRelay = 11;

int UImode; //1: Normal screen; 2: Set temperature; 3: Display error code
long setTemp = 0; //Stores temperature 10X - So 12.4C is 124.

void setup() {

  lcd.init();
  dht.begin();

  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Inditas... ");
  lcd.print(softwareVersion);
  lcd.setCursor(0, 1);
  lcd.print("FodorHOME>>Kamra");

  pinMode(middleSwitchPin, INPUT_PULLUP);
  pinMode(oLED, OUTPUT);
  pinMode(oRelay, OUTPUT);
  digitalWrite(oRelay, HIGH);

  EEPROM.get(setTempA, setTemp);
  if (setTemp == 0) UImode = 2;
  else UImode = 1;

  delay(2000);
  lcd.clear();
}

int error = 0; //0 means no error. When non-0, most functions halt, and error is displayed. (not yet implemented)

bool doneMiddle = false;
bool middleState = false; //True means pressed down
unsigned long forMiddleRead; //At this millis() time, we'll check button state again (crude debounce);
static int middleReadFreq = 15; //Read middle button with this ms interval

unsigned long forDhtRead;
static int dhtReadFreq = 3000;

unsigned long forUI;
static int UIfreq = 200;

int nowTemp;
int nowHum;

bool heat = false;

void loop() {

  // Encoder
  if (UImode == 2) {
    long NsetTemp = setTempEnc.read() / 4;
    if (NsetTemp != setTemp) {
      setTemp = NsetTemp;
      doUI();
    }
  }

  if (forMiddleRead < millis()) {
    forMiddleRead = millis() + middleReadFreq;

    bool NmiddleState = !digitalRead(middleSwitchPin);
    if (NmiddleState != middleState) {
      middleState = NmiddleState;
      if (middleState) doMiddle();
      else doneMiddle = false;
    }
  }
  // \Encoder

  //DHT
  if (UImode == 1 && forDhtRead < millis()) {
    forDhtRead = millis() + dhtReadFreq;

    float T = dht.readTemperature();
    float H = dht.readHumidity();
    if (isnan(T) || isnan(H)) error = 1;
    T *= 10;
    nowTemp = T;
    H *= 10;
    nowHum = H;

    doHeat();
  }

  //UI
  if (forUI < millis()) {
    forUI = millis() + UIfreq;

    doUI();
  }

  doOutput();
}

static int tempTreshold = 5; //In C, X10

void doHeat() {
  if (nowTemp < setTemp) heat = true;
  else if (nowTemp > setTemp + tempTreshold) heat = false;
}

void doOutput() {
  if (heat) {
    digitalWrite(oLED, HIGH);
    digitalWrite(oRelay, LOW); //Relays work backwards...
  }
  else {
    digitalWrite(oLED, LOW);
    digitalWrite(oRelay, HIGH); //Relays work backwards...
  }
}

void doUI() {

  if (error) UImode = 3;

  float printTemp = nowTemp;
  printTemp /= 10;
  float printSetTemp = setTemp;
  printSetTemp /= 10;
  float printHum = nowHum;
  printHum /= 10;

  switch (UImode) {
    case 1: //normal
      lcd.setCursor(1, 0);
      lcd.print(printTemp, 1);
      lcd.print("C  ");
      lcd.setCursor(7, 0);
      lcd.print("!");
      lcd.print(printSetTemp, 1);
      lcd.print("C  ");
      lcd.setCursor(15, 0);
      heat ? lcd.print("F") : lcd.print("_");

      lcd.setCursor(1, 1);
      lcd.print(printHum, 0);
      lcd.print(" %  ");
      lcd.setCursor(8, 1);
      lcd.print("BEALLIT>");
      break;

    case 2: //set
      lcd.setCursor(0, 0);
      lcd.print("       >");
      lcd.print(printSetTemp, 1);
      lcd.print("C<  ");

      lcd.setCursor(0, 1);
      lcd.print("           KESZ>");
      break;

    case 3: //error
      lcd.setCursor(0, 0);
      lcd.print("HIBA!  Kod:     ");
      lcd.setCursor(11, 0);
      lcd.print(error);

      lcd.setCursor(0, 1);
      lcd.print("        RENDBEN>");
      break;

  }
}

void doMiddle() {
  if (doneMiddle) return;
  else doneMiddle = true;

  if (!error) {
    if (UImode == 1) {
      setTempEnc.write(setTemp * 4);
      UImode = 2;
    }
    else if (UImode == 2) {
      EEPROM.put(setTempA, setTemp);
      UImode = 1;
    }
  }
  else {
    error = 0;
    UImode = 1;
  }

  lcd.clear();
  doUI();
}
