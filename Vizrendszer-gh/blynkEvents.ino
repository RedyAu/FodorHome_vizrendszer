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

  //Update state of functions and buttons
  static unsigned char _v50 = 0;
  if ((_temp = _v50) != (_v50 = directTap)) Blynk.virtualWrite(V50, directTap);
  static unsigned char _v56 = 0;
  if ((_temp = _v56) != (_v56 = watering)) Blynk.virtualWrite(V56, watering);
  
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
  
  sprintf(tempString, "Szürke: %dp", (int)(((float)greyWeight / (float)sumWeights) * (float)totalTime) / 60); //Grey
  Blynk.setProperty(V23, "offLabel", tempString);

  sprintf(tempString, "Csap: %dp", (int)(((float)tapWeight / (float)sumWeights) * (float)totalTime) / 60); //Tap watering
  Blynk.setProperty(V31, "offLabel", tempString);
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
//Grey
BLYNK_WRITE(V23) {  //Active
  isGreyActive = param.asInt();
}
BLYNK_WRITE(V24) {  //Weight
  greyWeight = param.asInt();
}
//Tap watering
BLYNK_WRITE(V31) {  //Active
  isTapActive = param.asInt();
}
BLYNK_WRITE(V32) {  //Weight
  tapWeight = param.asInt();
}
/*Watering Zone Properties*/
//-------------------------------------

BLYNK_WRITE(V40) {
  directGrey = param.asInt();
}
BLYNK_WRITE(V41) {
  directPink = param.asInt();
}
BLYNK_WRITE(V42) {
  directGreen = param.asInt();
}
BLYNK_WRITE(V43) {
  directBlue = param.asInt();
}
BLYNK_WRITE(V44) {
  directRed = param.asInt();
}

//-------------------------------------

BLYNK_WRITE(V49) {
  if (param.asInt()) {
    currentJob = { NoStopNext, AllValves, AllValves };
  } else {
    currentJob = { StopNext };
  }
}

BLYNK_WRITE(V50) {  //TapFlowButton
  directTap = param.asInt();
}
BLYNK_WRITE(V52) {  //Stop Button
  if (param.asInt()) {
    currentJob = waterJob{ StopNext };
    currentSession = emptySession;
    directTap = false;
    watering = false;
    wateringFinished = true;
    EEPROM.update(13, true);
  }
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
BLYNK_WRITE(V63) {  //isPeriodicWateringEnabled
  isPeriodicWateringEnabled = param.asInt();
}
BLYNK_WRITE(V65) {
  wateringMinutesCompletedToday = param.asInt();
}

BLYNK_WRITE(V80) { // PAUSE
  pause = param.asInt();
}

BLYNK_WRITE(V67) { // simulate daily reset
  if (param.asInt()) {
    doDailyReset();
  }
}

BLYNK_WRITE(V68) {
  if (param.asInt()) {
    doScheduledStart(false);
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
