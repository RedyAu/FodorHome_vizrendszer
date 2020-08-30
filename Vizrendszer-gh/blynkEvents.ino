void blynkSync() {
  //terminal.println("Sync...");
  
  //Update water levels
  Blynk.virtualWrite(V40, levelOf(Buffer));
  Blynk.virtualWrite(V41, levelOf(Watering));

  //Update buffer temperature
  Blynk.virtualWrite(V43, bufferTemp);

  //Update state of functions and buttons
  Blynk.virtualWrite(V50, tapFlow);
  Blynk.virtualWrite(V51, dumping);
  Blynk.virtualWrite(V53, cooling);
  Blynk.virtualWrite(V55, fullEmpty);
  Blynk.virtualWrite(V56, watering);
  Blynk.virtualWrite(V64, isCoolingWatering);

  //Update watering stuff
  Blynk.virtualWrite(V59, skipNextWatering); 
  Blynk.virtualWrite(V56, watering);
  Blynk.virtualWrite(V61, doneToday ? 255 : 0);
  
  if (watering) {
    long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
    long double wateringProgress = (long double)1024 * wateringProgressRatio;
    Blynk.virtualWrite(V60, (int)wateringProgress);
    Blynk.virtualWrite(V62, (int)((currentSession.duration / 1000) / 60));
  }

  static bool heartbeat;
  if (!heartbeat) {
    Blynk.virtualWrite(V104, 255);
    heartbeat = true;
  } else {
    Blynk.virtualWrite(V104, 0);
    heartbeat = false;
  }

  terminal.flush();
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
  watering = false;
}

//-------------------------------------
//Watering Zone Properties
//Pink
BLYNK_WRITE(V11) { //Active
  isPinkActive = param.asInt();
}
BLYNK_WRITE(V12) { //Weight
  pinkWeight = param.asInt();
}
//Green
BLYNK_WRITE(V14) { //Active
  isGreenActive = param.asInt();
}
BLYNK_WRITE(V15) { //Weight
  greenWeight = param.asInt();
}
//Blue
BLYNK_WRITE(V17) { //Active
  isBlueActive = param.asInt();
}
BLYNK_WRITE(V18) { //Weight
  blueWeight = param.asInt();
}
//Red
BLYNK_WRITE(V20) { //Active
  isRedActive = param.asInt();
}
BLYNK_WRITE(V21) { //Weight
  redWeight = param.asInt();
}
/*Watering Zone Properties*/
//-------------------------------------

BLYNK_WRITE(V50) { //TapFlowButton
  tapFlow = param.asInt();
}
BLYNK_WRITE(V51) { //Dumping Button
  dumping = param.asInt();
}
BLYNK_WRITE(V52) { //Stop Button
  if (param.asInt()) {
    currentJob = waterJob{StopNext};
    currentSession = emptySession;
    cooling = false;
    dumping = false;
    tapFlow = false;
    watering = false;
    wateringFinished = true;
  }
}
BLYNK_WRITE(V53) { //cooling button
  cooling = param.asInt();
}
BLYNK_WRITE(V54) { //buffer treshold
  bufferTreshold = param.asDouble();
}
BLYNK_WRITE(V55) { //full empty
  fullEmpty = param.asInt();
}
bool initNoWatering = true;
BLYNK_WRITE(V56) { //watering start button
  if (param.asInt()) {
    if (initNoWatering) {
      initNoWatering = false;
      Blynk.virtualWrite(V56, 0);
    } else beginWatering(setWateringDuration, Normal);
  } else {
    watering = false;
    wateringFinished = true;
    currentJob = waterJob {StopNext};
  }
}
BLYNK_WRITE(V57) { //watering duration (when next started)
  setWateringDuration = (unsigned long)((unsigned long)param.asInt() * 60) * 1000;
}

bool isMonday, isTuesday, isWednesday, isThursday, isFriday, isSaturday, isSunday;
bool isThisWeekdaySelected(int thisWeekday) {
  switch (thisWeekday) { //im so terribly sorry
    case 1:
      return isMonday;
      break;
    case 2:
      return isTuesday;
      break;
    case 3:
      return isWednesday;
      break;
    case 4:
      return isThursday;
      break;
    case 5:
      return isFriday;
      break;
    case 6:
      return isSaturday;
      break;
    case 7:
      return isSunday;
      break;
    default:
      terminal.print("isThisWeekdaySelected invalid weekday");
      error(1020);
  }
}
BLYNK_WRITE(V58) { //daily watering start at
  TimeInputParam t(param);
  isMonday = t.isWeekdaySelected(1); //im sorry
  isTuesday = t.isWeekdaySelected(2);
  isWednesday = t.isWeekdaySelected(3);
  isThursday = t.isWeekdaySelected(4);
  isFriday = t.isWeekdaySelected(5);
  isSaturday = t.isWeekdaySelected(6);
  isSunday = t.isWeekdaySelected(7);
  
  dailyWateringAtSeconds = param[0].asLong();
  if (dailyWateringAtSeconds == 0) dailyWateringAtSeconds = 60; //scheduler can't handle midnight, so if that's set, set it to 00:01.
}

BLYNK_WRITE(V59) { //skip next daily watering session
  skipNextWatering = param.asInt();
}
BLYNK_WRITE(V61) { //done today led - hopefully works ( todo )
  doneToday = param.asInt();
}
BLYNK_WRITE(V64) {
  isCoolingWatering = param.asInt();
}
BLYNK_WRITE(V63) { //isPeriodicWateringEnabled
  isPeriodicWateringEnabled = param.asInt();
}
BLYNK_WRITE(V102) { //error clear
  if (param.asInt()) error(0);
}
