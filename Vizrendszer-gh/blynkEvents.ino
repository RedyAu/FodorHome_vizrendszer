void heartbeat(bool ignoreBlynk = false) {
  static unsigned char heartbeat = 0;
  heartbeat ^= 1;
  if (!ignoreBlynk) Blynk.virtualWrite(V104, heartbeat ? 255 : 0);
  if (Blynk.connected() || ignoreBlynk) {
    digitalWrite(watchdogPin, heartbeat);
    Serial.print(".");
  }
}

void blynkSync() {
  unsigned char _temp;

  //Update water levels
  static unsigned char _v40 = 0;
  if ((_temp = _v40) != (_v40 = levelOf(Buffer))) Blynk.virtualWrite(V40, _v40);  //changes - store | own var
  static unsigned char _v41 = 0;
  if ((_temp = _v41) != (_v41 = levelOf(Watering))) Blynk.virtualWrite(V41, _v41);  //changes - store | own var

  //Update buffer temperature

  static float _bufferTemp;
  if (_bufferTemp != bufferTemp) {
    Blynk.virtualWrite(V43, bufferTemp);  //changes - store | own var
    _bufferTemp = bufferTemp;
  }

  //Update state of functions and buttons
  static unsigned char _v50 = 0;
  if ((_temp = _v50) != (_v50 = tapFlow)) Blynk.virtualWrite(V50, tapFlow);
  static unsigned char _v51 = 0;
  if ((_temp = _v51) != (_v51 = dumping)) Blynk.virtualWrite(V51, dumping);
  static unsigned char _v53 = 0;
  if ((_temp = _v53) != (_v53 = cooling)) Blynk.virtualWrite(V53, cooling);
  static unsigned char _v55 = 0;
  if ((_temp = _v55) != (_v55 = fullEmpty)) Blynk.virtualWrite(V55, fullEmpty);
  static unsigned char _v56 = 0;
  if ((_temp = _v56) != (_v56 = watering)) Blynk.virtualWrite(V56, watering);
  static unsigned char _v64 = 0;
  if ((_temp = _v64) != (_v64 = isCoolingWatering)) Blynk.virtualWrite(V64, isCoolingWatering);

  //Update watering stuff
  static unsigned char _v59 = 0;
  if ((_temp = _v59) != (_v59 = skipNextWatering)) Blynk.virtualWrite(V59, skipNextWatering);
  static unsigned char _v61 = 0;
  if ((_temp = _v61) != (_v61 = doneToday)) Blynk.virtualWrite(V61, doneToday ? 255 : 0);

  if (watering) {
    long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
    long double wateringProgress = (long double)1024 * wateringProgressRatio;
    Blynk.virtualWrite(V60, (int)wateringProgress);                         //no store
    Blynk.virtualWrite(V62, (int)(currentSession.duration / 60));  //no store
  }

  static int _v65 = -1;
  if ((_temp = _v65) != (_v65 = wateringMinutesCompletedToday)) Blynk.virtualWrite(V65, wateringMinutesCompletedToday);

  heartbeat();
  terminal.flush();
}

void pushWateringTimes() {
  Serial.println("push");
  static unsigned long lastPushed = 0;
  if (!syncComplete) return;

  updateZones();
  int totalTime = setWateringDuration;

  Serial.println(setWateringDuration);
  char tempString[50] = { 0 };

  /*int whytho = (((float)pinkWeight / (float)sumWeights) * (float)totalTime) / 60;
  Serial.println(whytho);*/
  sprintf(tempString, "Pink: %dp", (int)(((float)pinkWeight / (float)sumWeights) * (float)totalTime) / 60);  //Pink
  Blynk.setProperty(V11, "offLabel", tempString);
  sprintf(tempString, "Zöld: %dp", (int)(((float)greenWeight / (float)sumWeights) * (float)totalTime) / 60);  //Green
  Blynk.setProperty(V14, "offLabel", tempString);
  sprintf(tempString, "Kék: %dp", (int)(((float)blueWeight / (float)sumWeights) * (float)totalTime) / 60);  //Blue
  Blynk.setProperty(V17, "offLabel", tempString);
  sprintf(tempString, "Piros: %dp", (int)(((float)redWeight / (float)sumWeights) * (float)totalTime) / 60);  //Red
  Blynk.setProperty(V20, "offLabel", tempString);
  //sprintf(tempString, "Szürke: %dp", (int)(((float)greyWeight / (float)sumWeights) * (float)totalTime) / 60); //Grey future
  //Blynk.setProperty(V11, "offLabel", tempString);*/
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
  watering = false;
  syncComplete = true;
}

//-------------------------------------
//Watering Zone Properties
//Pink
BLYNK_WRITE(V11) {  //Active
  isPinkActive = param.asInt();
}
BLYNK_WRITE(V12) {  //Weight
  pinkWeight = param.asInt();
}
//Green
BLYNK_WRITE(V14) {  //Active
  isGreenActive = param.asInt();
}
BLYNK_WRITE(V15) {  //Weight
  greenWeight = param.asInt();
}
//Blue
BLYNK_WRITE(V17) {  //Active
  isBlueActive = param.asInt();
}
BLYNK_WRITE(V18) {  //Weight
  blueWeight = param.asInt();
}
//Red
BLYNK_WRITE(V20) {  //Active
  isRedActive = param.asInt();
}
BLYNK_WRITE(V21) {  //Weight
  redWeight = param.asInt();
}
/*Watering Zone Properties*/
//-------------------------------------

BLYNK_WRITE(V49) {
  if (param.asInt()) {
    currentJob = { NoStopNext, AllValves, AllValves };
  } else {
    currentJob = { StopNext };
  }
}

BLYNK_WRITE(V50) {  //TapFlowButton
  tapFlow = param.asInt();
}
BLYNK_WRITE(V51) {  //Dumping Button
  dumping = param.asInt();
}
BLYNK_WRITE(V52) {  //Stop Button
  if (param.asInt()) {
    currentJob = waterJob{ StopNext };
    currentSession = emptySession;
    cooling = false;
    dumping = false;
    tapFlow = false;
    watering = false;
    wateringFinished = true;
    EEPROM.update(13, true);
  }
}
BLYNK_WRITE(V53) {  //cooling button
  cooling = param.asInt();
}
BLYNK_WRITE(V54) {  //buffer treshold
  bufferTreshold = param.asDouble();
}
BLYNK_WRITE(V55) {  //full empty
  fullEmpty = param.asInt();
}
BLYNK_WRITE(V56) {  //watering start button
  if (param.asInt()) {
    if (restoreWateringSession()) return;
    beginWatering(setWateringDuration, Normal);
  } else {
    watering = false;
    currentSession = emptySession;
    wateringFinished = true;
    currentJob = waterJob{ StopNext };
    EEPROM.update(13, true);
  }
}
BLYNK_WRITE(V57) {  //watering duration (when next started)
  setWateringDuration = (unsigned long)param.asInt() * 60;
  pushWateringTimes();
}

BLYNK_WRITE(V58) {  //daily watering start at
  TimeInputParam t(param);
  dailyWateringAtSeconds = param[0].asLong();
}

BLYNK_WRITE(V59) {  //skip next daily watering session
  skipNextWatering = param.asInt();
}
BLYNK_WRITE(V61) {  //done today led
  doneToday = param.asInt();
}
BLYNK_WRITE(V64) {
  isCoolingWatering = param.asInt();
}
BLYNK_WRITE(V63) {  //isPeriodicWateringEnabled
  isPeriodicWateringEnabled = param.asInt();
}
BLYNK_WRITE(V65) {
  wateringMinutesCompletedToday = param.asInt();
}

BLYNK_WRITE(V67) { // refresh data
if (param.asInt()) {
  terminal.println("Last 7 day's watering minutes:");
    for (int i = 0; i < 7; i++) {
      terminal.println(EEPROM.read(50));
    }
    terminal.println("---");
  sumMinutesWateredOnDays();
  updateWeatherValues();
}
}

BLYNK_WRITE(V70) {
  mmToMinuteFactor = param.asInt();
}
BLYNK_WRITE(V71) {
  minimumStartableDuration = (unsigned long)param.asInt() * 60;
}

BLYNK_WRITE(V102) {  //error clear
  if (param.asInt()) error(0);
}
