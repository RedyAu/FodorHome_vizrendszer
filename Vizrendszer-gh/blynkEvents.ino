void blynkSync() {
  unsigned char _temp;

  //Update water levels
  static unsigned char _v40 = 0;
  if ((_temp = _v40) != (_v40 = levelOf(Buffer))) Blynk.virtualWrite(V40, _v40); //changes - store | own var
  static unsigned char _v41 = 0;
  if ((_temp = _v41) != (_v41 = levelOf(Watering))) Blynk.virtualWrite(V41, _v41); //changes - store | own var

  //Update buffer temperature

  static float _bufferTemp;
  if (_bufferTemp != bufferTemp) {
    Blynk.virtualWrite(V43, bufferTemp); //changes - store | own var
    _bufferTemp = bufferTemp;
  }

  //Update state of functions and buttons
  static unsigned char _v50 = 0;
  if ((_temp = _v50) != (_v50 = tapFlow)) Blynk.virtualWrite(V50, tapFlow); //changes - store | bit0
  static unsigned char _v51 = 0;
  if ((_temp = _v51) != (_v51 = dumping)) Blynk.virtualWrite(V51, dumping); //changes - store | bit1
  static unsigned char _v53 = 0;
  if ((_temp = _v53) != (_v53 = cooling)) Blynk.virtualWrite(V53, cooling); //changes - store | bit2
  static unsigned char _v55 = 0;
  if ((_temp = _v55) != (_v55 = fullEmpty)) Blynk.virtualWrite(V55, fullEmpty); //changes - store | 3
  static unsigned char _v56 = 0;
  if ((_temp = _v56) != (_v56 = watering)) Blynk.virtualWrite(V56, watering); //changes - store | 4
  static unsigned char _v64 = 0;
  if ((_temp = _v64) != (_v64 = isCoolingWatering)) Blynk.virtualWrite(V64, isCoolingWatering); //does not change probably

  //Update watering stuff
  static unsigned char _v59 = 0;
  if ((_temp = _v59) != (_v59 = skipNextWatering))  Blynk.virtualWrite(V59, skipNextWatering); //changes - store | 5
  static unsigned char _v61 = 0;
  if ((_temp = _v61) != (_v61 = doneToday)) Blynk.virtualWrite(V61, doneToday ? 255 : 0); //changes | 7

  if (watering) {
    long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
    long double wateringProgress = (long double)1024 * wateringProgressRatio;
    Blynk.virtualWrite(V60, (int)wateringProgress); //no store
    Blynk.virtualWrite(V62, (int)((currentSession.duration / 1000) / 60)); //no store
  }

  static unsigned char heartbeat = 0;
  heartbeat ^= 1;
  Blynk.virtualWrite(V104, heartbeat ? 255 : 0);

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

BLYNK_WRITE(V49) {
  if (param.asInt()) {
    currentJob = {NoStopNext, AllValves, AllValves};
    terminal.print("Opening all valves");
  } else {
    currentJob = {StopNext};
    terminal.print("Closing all valves");
  }
}

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
      return isSunday;
      break;
    case 2:
      return isMonday;
      break;
    case 3:
      return isTuesday;
      break;
    case 4:
      return isWednesday;
      break;
    case 5:
      return isThursday;
      break;
    case 6:
      return isFriday;
      break;
    case 7:
      return isSaturday;
      break;
    default:
      terminal.print("isThisWeekdaySelected invalid weekday");
      error(1020);
  }
}
BLYNK_WRITE(V58) { //daily watering start at
  TimeInputParam t(param);

   //im sorry
  isMonday = t.isWeekdaySelected(1);
  isTuesday = t.isWeekdaySelected(2);
  isWednesday = t.isWeekdaySelected(3);
  isThursday = t.isWeekdaySelected(4);
  isFriday = t.isWeekdaySelected(5);
  isSaturday = t.isWeekdaySelected(6);
  isSunday = t.isWeekdaySelected(7);

  /*terminal.println(isThisWeekdaySelected(1) ? "Weekday 1 is selected" : "Weekday 1 is not selected");
  terminal.println(isThisWeekdaySelected(2) ? "Weekday 2 is selected" : "Weekday 2 is not selected");
  terminal.println(isThisWeekdaySelected(3) ? "Weekday 3 is selected" : "Weekday 3 is not selected");
  terminal.println(isThisWeekdaySelected(4) ? "Weekday 4 is selected" : "Weekday 4 is not selected");
  terminal.println(isThisWeekdaySelected(5) ? "Weekday 5 is selected" : "Weekday 5 is not selected");
  terminal.println(isThisWeekdaySelected(6) ? "Weekday 6 is selected" : "Weekday 6 is not selected");
  terminal.println(isThisWeekdaySelected(7) ? "Weekday 7 is selected" : "Weekday 7 is not selected");*/

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
