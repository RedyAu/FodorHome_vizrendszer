
/* Written by Benedek Fodor in 2019-2020
   Versions:
   0.0 - Write out in plain text what the code should do
   0.1 - Main functions layed out to tabs
   0.2 - First succesful compile
   0.3 - Serial Communication Implemented
   0.4 - Sensors set up
   0.5 - Litter the whole thing with debug messages.
   0.5.1 - Littered to the brim with debug.
   0.5.2 - Litter removed
   0.6 - Individual valve commands
   0.7 - tapFlow now pulls from buffer too (1 buffer, 2 watering, 3 well)
   0.8 - New ultrasonic sensor now at the top of the buffer tank. tapFlowSwitch implemented. dumpToTap implemented.
   0.9 - dumpToTap now changeable while running with command. HouseKeep function implemented containing various useful code ran every loop. Initialisation wait implemented.
   0.10 - eeprom functionality implemented. Store if cooling and dumpToTap was active and store bufferTarget. bufferTreshold functionality removed. Cleaned up serial output.
   0.11 - Implemented capability to use a timer in place for the buffer tank's temperature sensor in case that breaks.
   0.11.2 - TapFlow only gets water from well (broken sensors during winter)
   0.12 - Print version on init. Implement 3-part switch sequence to prevent accidental presses of tapFlowSwitch.
   0.12.1 - Faster response when starting water pumping, bugfixes
   0.12.2 - New pin definitions for system with watering built in
   0.13 - Restored IRL sensors, updated pins, cleaned up code. New capacitive water level sensor on waterUpper. Removed determineUpper() along with old waterUpper and groundWater sensors.
   1.0 - Add watering sections to waterStart, rework it, make allStop part of it.
         New system for tasks - no stopping of the pump when changing solanoid.
         Whole task management rewritten, more functionality added.
         File structure reworked.
*/
#define softwareVersion "1.0"

//Constants
const bool bufferTempDoTimer = false;

const bool debug = true;

#define RelayOn LOW
#define RelayOff HIGH

const int tapFlowSequenceMinimumTimeMillis = 300; //Least amont of time to finish the 3-part switch sequence
const int tapFlowSequenceMaximumTimeMillis = 3000; //Most amont of time to finish the 3-part switch sequence
const int tapFlowSequenceFirstDoneByMillis = 1000; //Most amount of time to turn of switch after first turned on to start 3-part switch sequence
const unsigned long tapFlowShortDurationMillis = 30 * 1000; //Amount of time to do tapFlow when proper sequence is not initiated.

const int bufferLvlLower = 45; //Pin number of lower water sensor of buffer tank
const int bufferLvlUpper = 46; //Pin number of upper water sensor of buffer tank
const int waterLvlLower = 44; //Pin number of lower water sensor of watering tank
const int waterLvlUpper = 39; //Pin number of upper water sensor of watering tank
const int tapFlowSwitch = 47; //Pulled up switch next to the tap.
const int udvarDHTpin = 48; //WIP
const int oneWireBus = 41; //One sensor connected: Water temperature sensor of Buffer tank

const int fromWell = 26;
const int fromGarage = 27;

const int toTap = 30;
const int toDump = 31;
const int toGrey = 32;
const int toPink = 33;
const int toGreen = 34;
const int toBlue = 35;
const int toRed = 36;
const int mainPump = 28;

const int toBuffer = 24;
const int toWatering = 25;
const int fromBuffer = 22;
const int fromWatering = 23;
const int flowPump = 29;

//Custom wording for clarity
#define Buffer 0 //levelOf()
#define Watering 1

#define Continue false //program flow control of job()
#define End true

#define StopNext true //waterJob
#define NoStopNext false

struct waterJob {
  bool stop;
  int from;
  int to;
};
waterJob currentJob;


byte output[] = {22,23,24,25,30,31,32,33,34,35,36,37,26,27,28,29};
byte input[] = {39,41,44,45,46};
byte input_pullup[] = {47};

//Globals

bool cooling, tapFlow, dumping, fullEmpty, begun = true;

float bufferTemp, wateringTemp;
float udvarTemp, udvarHum;

float bufferTarget;

int currentError;

unsigned long seconds = 0, forSecond = 0;

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

//Initialize libraries

#include <Utilities.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

DHT udvarDHT(udvarDHTpin, DHT11);

OneWire oneWire(oneWireBus);
DallasTemperature waterTemp(&oneWire);

//🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴

void setup() {
  //Init debug serial
  Serial.begin(9600);
  if (debug) {
    Serial.println(softwareVersion);
    Serial.print("Initializing... ");
  }
  //Init comms serial
  //todo

  //Init sensors
  udvarDHT.begin();
  waterTemp.begin();

  //Init pins
  pinModeGroup(output, LEN(output), OUTPUT);
  digitalWriteGroup(output, LEN(output), HIGH);

  pinModeGroup(input, LEN(input), INPUT);

  pinModeGroup(input_pullup, LEN(input_pullup), INPUT_PULLUP);

  //Read data stored in eeprom
  EEPROM.get(10, cooling);
  EEPROM.get(11, bufferTarget);
  EEPROM.get(12, dumpToTap);

  //Wait for Initialization of Thermostat
  while (!begun) {
    serialRead();
    Serial.print("wait 0;");
    sense();
    delay(1000);
  }
}

//🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴

void loop() {
  serialRead();
  sense();
  job();//todo
  jobDo();
  serialSend();//todo
  
//  everyTick();
  
  /*everyTick();
  sense();
  waterTasker(); //Don't act in the first five seconds while sensors are initializing.
  water();
  waterDo();

  serialRead();
  sendData();*/
}
