/*
   Written by Benedek Fodor in 2019
   Versions: (first number: rolls with new feature | second number: rolls with programming session)
   0.0 - Write out in plain text what the code should do
   0.1 - Main functions layed out to tabs
   0.2 - First succesful compile
   0.3 - Serial Communication Implemented
   0.4 - Sensors set up
   0.5 - Litter the whole thing with debug messages.
   0.51 - Littered to the brim with debug.
   0.52 - Litter removed
   0.6 - Individual valve commands
   0.7 - tapFlow now pulls from buffer too (1 buffer, 2 watering, 3 well)
   0.8 - New ultrasonic sensor now at the top of the buffer tank. tapFlowSwitch implemented. dumpToTap implemented.
   0.9 - dumpToTap now changeable while running with command. HouseKeep function implemented containing various useful code ran every loop. Initialisation wait implemented.
   0.10 - eeprom functionality implemented. Store if cooling and dumpToTap was active and store bufferTarget. bufferTreshold functionality removed. Cleaned up serial output.
   0.11 - Implemented capability to use a timer in place for the buffer tank's temperature sensor in case that breaks.
   0.11.2 - TapFlow only gets water from well
   0.12 - Print version on init. Implement 3-part switch sequence to prevent accidental presses of tapFlowSwitch.

   1.0 - First operating version (future)
*/
const String softwareVersion = "0.12";
/*
#include "allStop.h"
#include "communicate.h"
#include "continuityCheck.h"
#include "error.h"
#include "sendData.h"
#include "sense.h"
#include "water.h"
#include "waterLevel.h"
#include "waterStart.h"
#include "waterTasker.h"
*/
//Constants
int waitToCool = 500;
int tempTimerSt = 1;
unsigned long tempTimerForSeconds;
const bool bufferTempDoTimer = true; ///////////////////////////////

const bool debug = true;

#define relayON LOW
#define relayOFF HIGH

const int tapFlowSequenceMinimumTimeMillis = 300; //Least amont of time to finish the 3-part switch sequence
const int tapFlowSequenceMaximumTimeMillis = 3000; //Most amont of time to finish the 3-part switch sequence
const int tapFlowSequenceFirstDoneByMillis = 1000; //Most amount of time to turn of switch after first turned on to start 3-part switch sequence
const unsigned long tapFlowShortDurationMillis = 30000; //Amount of time to do tapFlow when proper sequence is not initiated.

const int bufferLvlLower = 40; //Pin number of lower water sensor of buffer tank
const int bufferLvlUpper = 39; //Pin number of upper water sensor of buffer tank
const int waterLvlLower = 41; //Pin number of lower water sensor of watering tank
const int waterLvlUpper = A15; //Pin number of upper water sensor of watering tank
const int groundWaterLvl = A14; //Pin number of groundwater-sensor

const int fromWell = 33;
const int fromGarage = 34;
const int fromBuffer = 22;
const int fromWatering = 23;
const int toDumpO = 37; //original to dump
/* */ int toDump;
const int toBuffer = 24;
const int toWatering = 25;
const int toTap = 35;
const int mainPump = 28;
const int flowPump = 26;
const int tapFlowSwitch = 49;

byte output[] = {22, 23, 24, 25, 26, 28, 33, 34, 35, 37};
byte input[] = {39, 40, 41, A13, A15};
byte input_pullup[] = {49, 38};

//Globals
bool isFilling, isEmptying, isWaterWatering, isWellWatering, isBufferWatering, isBufferEmptying, isWaterEmptying;

bool cooling, tapFlow, dumping, dumpToTap, veryCool, begun = true;

float bufferTemp, wateringTemp;
float udvarTemp, udvarHum, garAknTemp, garAknHum;

float bufferTarget;

int currentError;

unsigned long seconds = 0, forSecond = 0;

//Initialize libraries

#include <Utilities.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define garazsaknaDHTpin 44
#define udvarDHTpin 48
DHT garazsaknaDHT(garazsaknaDHTpin, DHT11);
DHT udvarDHT(udvarDHTpin, DHT11);

#define oneWireBus 43
OneWire oneWire(oneWireBus);
DallasTemperature waterTemp(&oneWire);



void setup() {
  /*
    Serial - debug
    Serial1 - termosztát
    handshake
    settings receive
    start libraries if needed */

  Serial.begin(9600);
  if (debug) {
    Serial.println(softwareVersion);
    Serial.print("Initializing... ");
  }

  garazsaknaDHT.begin();
  udvarDHT.begin();
  waterTemp.begin();

  pinModeGroup(output, LEN(output), OUTPUT);
  digitalWriteGroup(output, LEN(output), HIGH);

  pinModeGroup(input, LEN(input), INPUT);

  pinModeGroup(input_pullup, LEN(input_pullup), INPUT_PULLUP);

  //Read data stored in eeprom
  EEPROM.get(10, cooling);
  EEPROM.get(11, bufferTarget);
  EEPROM.get(12, dumpToTap);

  while (!begun) {
    serialRead(); //Wait for Initialization of Thermostat inside
    Serial.print("wait 0;");
    sense();
    delay(1000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  houseKeep();
  sense();
  if (seconds > 5) waterTasker(); //Don't act in the first five seconds while sensors are initializing.
  water();

  serialRead();
  if (seconds > 5) sendData();
}

void houseKeep() { //Miscalennious things to do in each loop.  
  if (millis() > forSecond) {
    seconds++;
    forSecond = millis() + 1000; //Second clock for longer timings. No need to worry about value resetting, unsigned long can hold 160 years worth of seconds.
    everySecond();
  }

//Bad temp sensor
  if (!bufferTempDoTimer) return;
  else {
    switch(tempTimerSt) {
      case 0: //prompt cooling
        bufferTemp = 30.0;
        break;
      case 1: //water() refreshed buffer tank
        tempTimerForSeconds = seconds + waitToCool;
        tempTimerSt = 2;
        break;
      case 2: //waiting for timer
        bufferTemp = 5.00;
        if (seconds > tempTimerForSeconds) tempTimerSt = 0;
        break;       
    }
  }
  
}

void everySecond() { //Do stuff that doesn't need to be done every millisecond.
  if (seconds == 5 && debug) Serial.print("Done!\n\n"); //End of initializing message.

  if (dumpToTap) toDump = toTap; //Change dumping solanoid assignment.
  else toDump = toDumpO;

}
