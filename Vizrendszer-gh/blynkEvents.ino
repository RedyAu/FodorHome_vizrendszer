void blynkSync() {
  Serial.println("Sync...");

  Blynk.virtualWrite(V40, levelOf(Buffer));
  Blynk.virtualWrite(V41, levelOf(Watering));

  Blynk.virtualWrite(V43, bufferTemp);

  Blynk.virtualWrite(V0, isBufferEmptying() ? 255 : 0);
  Blynk.virtualWrite(V1, isBufferFilling() ? 255 : 0);
  
  Blynk.virtualWrite(V55, fullEmpty);

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
BLYNK_WRITE(V53) {
  cooling = param.asInt();
}
BLYNK_WRITE(V54) {
  bufferTreshold = param.asDouble();
}
BLYNK_WRITE(V55) {
  fullEmpty = param.asInt();
}
