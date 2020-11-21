/* Written by Benedek Fodor in 2019-2020
   For controlling the heater system in the chamber; as part of the KÚTKLÍMA/FodorHome System

   ERRORS:
   0: DHT error
   1: Temperature error
   2: Humidity error

   Versions:
   0.0: First test version
   0.1: Implementing EEPROM
   0.2: Show version on startup. Only toggle one pin (changed relay module).
   1.0: Broken button: implemented automatic temperature set mode with timeout.
   1.1: Sleep functionality
   1.2: Display heater on percentage during last hour, new icons on screen
*/

#define RelayON LOW
#define RelayOFF HIGH

const String softwareVersion = "v1.2";

#include <EEPROM.h>

const int setTempA = 30;

#include <Encoder.h>
Encoder setTempEnc(3, 2);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#include <DHT.h>
const int dhtPin = 8;
DHT dht(dhtPin, DHT11);

unsigned long lastSet;
int setTimeoutStep = 0;

const int oLED = 13;
const int oRelay = 5;

int UImode; //1: Normal screen; 2: Set temperature; 3: Display error code
int setTemp = 0; //Stores temperature 10X - So 12.3C is 123.

int error = 0; //0 means no error. When non-0, most functions halt, and error is displayed.

unsigned long lastOnCheck;
const unsigned long onCheckFreq = 60000;

unsigned long lastDhtRead = 0;
const int dhtReadFreq = 3000;

unsigned long lastUI;
const int UIfreq = 200;

const unsigned long sleepTimeout = 600000; //10 min
bool isSleep = false;

int nowTemp;
int nowHum;

bool heat = false;

void setup() {
  Serial.begin(9600);
  Serial.println(softwareVersion);

  lcd.init();
  dht.begin();

  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Inditas... ");
  lcd.print(softwareVersion);
  lcd.setCursor(0, 1);
  lcd.print("FodorHOME>>Kamra");
  createChars();

  pinMode(oLED, OUTPUT);
  pinMode(oRelay, OUTPUT);
  digitalWrite(oRelay, HIGH);

  EEPROM.get(setTempA, setTemp);
  setTempEnc.write(setTemp * 4);
  if (setTemp == 0) UImode = 2;
  else UImode = 1;

  delay(2000);
  lcd.clear();
}

void loop() {
  if (millis() - lastSet > sleepTimeout) {
    isSleep = true;
    lcd.clear();
    lcd.noBacklight();
  }

  doSetTimer();
  // Encoder
  long NsetTemp = setTempEnc.read() / 4;
  if (NsetTemp != setTemp) {
    lastSet = millis();

    if (isSleep) {
      lcd.backlight();
      isSleep = false;
      doUI;
      delay(1500);
      setTempEnc.write(setTemp * 4);
      return;
    }
    setTemp = NsetTemp;


    error = 0;
    UImode = 2;
    if (UImode != 2) lcd.clear();

    doUI();
    return;//Update screen fasterrr
  }
  // \Encoder

  //DHT
  if (UImode == 1 && millis() - lastDhtRead > dhtReadFreq) {
    lastDhtRead = millis();

    float T = dht.readTemperature();
    float H = dht.readHumidity();
    if (isnan(T)) error = 1;
    if (isnan(H)) error = 2;
    if (isnan(T) && isnan(H)) error = 0;
    T *= 10;
    nowTemp = T;
    H *= 10;
    nowHum = H;

    doHeat();
  }

  if (millis() - lastOnCheck > onCheckFreq) {
    lastOnCheck = millis();
    updateOnPercentage();
  }

  doOutput();

  doUI();
}

static int tempTreshold = 5; //In C, X10 (so 5 = 0.5C)

void doHeat() {
  if (nowTemp < setTemp) heat = true;
  else if (nowTemp > setTemp + tempTreshold) heat = false;
}

void doOutput() {
  if (heat) {
    digitalWrite(oLED, HIGH);
    digitalWrite(oRelay, RelayON);
  }
  else {
    digitalWrite(oLED, LOW);
    digitalWrite(oRelay, RelayOFF);
  }
}

void doUI() {
  if (isSleep) return;

  if (millis() - lastUI > UIfreq) {
    lastUI = millis();
  } else return;

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
      lcd.write(0);
      lcd.print(printTemp, 1);
      lcd.print("C  ");
      lcd.setCursor(9, 0);
      lcd.write(1);
      lcd.print(printSetTemp, 1);
      lcd.print("C  ");

      lcd.setCursor(1, 1);
      lcd.write(2);
      lcd.print(printHum, 0);
      lcd.print("%  ");

      lcd.write(3);
      lcd.print(getOnPercentage());
      lcd.print("% ");

      lcd.setCursor(13, 1);
      lcd.write(4);
      heat
      ? lcd.print("BE")
      : lcd.print("KI");
      break;

    case 2: //set
      lcd.setCursor(8, 0);
      lcd.print(">");
      lcd.print(printSetTemp, 1);
      lcd.print("C<  ");

      lcd.setCursor(0, 1);
      switch (setTimeoutStep) {
        case 0:
          lcd.print("                ");
          break;
        case 1:
          lcd.print("           #    ");
          break;
        case 2:
          lcd.print("          ###   ");
          break;
        case 3:
          lcd.print("       #########");
          break;
        case 4:
          lcd.print("################");
          break;
      }
      break;

    case 3: //error
      lcd.setCursor(0, 0);
      lcd.print("HIBA!  Kod:     ");
      lcd.setCursor(11, 0);
      lcd.print(error);

      lcd.setCursor(0, 1);
      lcd.print("     RENDBEN: <>");
      break;

  }
}

void doSetTimer() {
  if (error || (UImode != 2)) return;

  unsigned long lastSetAgo = millis() - lastSet;

  if (lastSetAgo < 10) setTimeoutStep = -1; //don't overwrite the bottom row every tick
  else if (lastSetAgo < 1000) setTimeoutStep = 0;
  else if (lastSetAgo < 1500) setTimeoutStep = 1;
  else if (lastSetAgo < 2000) setTimeoutStep = 2;
  else if (lastSetAgo < 2500) setTimeoutStep = 3;
  else if (lastSetAgo < 3000) setTimeoutStep = 4;
  else {
    EEPROM.put(setTempA, setTemp);

    UImode = 1;
    lcd.clear();
    doUI();
  }
}
