void blynkSync() {
  Serial.println("Sync...");
  const int relays[] = {fromWell, fromBuffer, fromWatering, fromGarage,
                toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed,
                mainPump};

  Serial.print(digitalRead(mainPump) == RelayOn);
  
  for (int i = 0; i < LEN(relays); i++) { //Send state of every relay for display in the app (i hope)
    Blynk.virtualWrite(i, ((digitalRead(relays[i]) == RelayOn) ? 255 : 0));
    Serial.print(", ");
  }
  Serial.println(" end");
  

  Blynk.virtualWrite(V40, levelOf(Buffer));
  Blynk.virtualWrite(V41, levelOf(Watering));

  Blynk.virtualWrite(V100, currentError);

  Blynk.virtualWrite(V43, bufferTemp);

  Blynk.virtualWrite(V0, isBufferEmptying());
  Blynk.virtualWrite(V1, isBufferFilling());

  long double wateringProgressRatio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
  long double wateringProgress = (long double)1024 * wateringProgressRatio;
  Blynk.virtualWrite(V2, (int)wateringProgress);
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncAll();
}
