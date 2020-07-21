void blynkSync() {
  //Serial.println("Sync...");

  //Update state of relays (yes this was the best way to do it)
  Blynk.virtualWrite(fromGarage, (digitalRead(fromGarage) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromBuffer, (digitalRead(fromBuffer) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromWatering, (digitalRead(fromWatering) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromWell, (digitalRead(fromWell) == RelayOn) ? 255 : 0);

  Blynk.virtualWrite(toDump, (digitalRead(toDump) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toTap, (digitalRead(toTap) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toBuffer, (digitalRead(toBuffer) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toWatering, (digitalRead(toWatering) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(10, (digitalRead(toPink) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(13, (digitalRead(toGreen) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(16, (digitalRead(toBlue) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(19, (digitalRead(toRed) == RelayOn) ? 255 : 0);

  Blynk.virtualWrite(mainPump, (digitalRead(mainPump) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(flowPump, (digitalRead(flowPump) == RelayOn) ? 255 : 0);

  //Update water levels
  Blynk.virtualWrite(V40, levelOf(Buffer));
  Blynk.virtualWrite(V41, levelOf(Watering));

  //Update buffer temperature
  Blynk.virtualWrite(V43, bufferTemp);

  //Update cooling state
  Blynk.virtualWrite(V0, isBufferEmptying() ? 255 : 0);
  Blynk.virtualWrite(V1, isBufferFilling() ? 255 : 0);

  //Update state of functions and buttons
  Blynk.virtualWrite(V50, tapFlow);
  Blynk.virtualWrite(V51, dumping);
  Blynk.virtualWrite(V53, cooling);
  Blynk.virtualWrite(V55, fullEmpty);
  Blynk.virtualWrite(V56, watering);

  
  //Update watering stuff
  Blynk.virtualWrite(V59, skipNextWatering);
  Blynk.virtualWrite(V62, (int)(currentSession.duration / 60));
  
  //Update watering progress
  long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
  long double wateringProgress = (long double)1024 * wateringProgressRatio;
  Blynk.virtualWrite(V60, (int)wateringProgress);

  
  //Send error
  Blynk.virtualWrite(V101, currentError);
  Blynk.virtualWrite(V103, (currentError != 0 && currentError < 999) ? 255 : 0);

  static bool heartbeat;
  if (!heartbeat) {
    Blynk.virtualWrite(V104, 255);
    heartbeat = true;
  } else {
    Blynk.virtualWrite(V104, 0);
    heartbeat = false;
  }
}

bool isPinkActive, isGreenActive, isBlueActive, isRedActive;
int pinkWeight, greenWeight, blueWeight, redWeight;
void updateZones() {
  wateringZone newZones[] = {
    { isPinkActive, toPink, pinkWeight },
    { isGreenActive, toGreen, greenWeight },
    { isBlueActive, toBlue, blueWeight },
    { isRedActive, toRed, redWeight }
  };
  
  //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA I DONT WANT IT LIKE THIS BUT THERE IS NOT OTHER WAY WHYYY
  //Just spent >2h searching for a cleaner solution... not happy...
  for (int i = 0; i < 4; i++) {
    zones[i] = newZones[i];
  }
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
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
BLYNK_WRITE(V56) { //watering start button
  if (param.asInt()) {
    beginWatering(setWateringDuration * 60, Normal);
  }
}
BLYNK_WRITE(V57) { //watering duration (when next started)
  setWateringDuration = param.asInt();
}
BLYNK_WRITE(V58) { //daily watering start at
  dailyWateringAtSeconds = param[0].asLong();
}
BLYNK_WRITE(V61) { //done today led - hopefully works ( todo )
  doneToday = param.asInt();
}
BLYNK_WRITE(V102) { //error clear
  if (param.asInt()) error(0);
}
