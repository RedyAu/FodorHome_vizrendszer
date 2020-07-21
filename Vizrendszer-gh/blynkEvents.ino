void blynkSync() {
  //Serial.println("Sync...");

  Blynk.virtualWrite(V40, levelOf(Buffer));
  Blynk.virtualWrite(V41, levelOf(Watering));

  Blynk.virtualWrite(V43, bufferTemp);

  Blynk.virtualWrite(V0, isBufferEmptying() ? 255 : 0);
  Blynk.virtualWrite(V1, isBufferFilling() ? 255 : 0);

  Blynk.virtualWrite(V50, tapFlow);
  Blynk.virtualWrite(V51, dumping);
  Blynk.virtualWrite(V53, cooling);
  Blynk.virtualWrite(V55, fullEmpty);

  Blynk.virtualWrite(V101, currentError);
  Blynk.virtualWrite(V103, (currentError != 0 && currentError < 999) ? 255 : 0);

  long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
  long double wateringProgress = (long double)1024 * wateringProgressRatio;
  Blynk.virtualWrite(V2, (int)wateringProgress);
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
BLYNK_WRITE(V102) { //error clear
  if (param.asInt()) error(0);
}
