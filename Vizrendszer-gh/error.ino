void error(int type) {
  currentError = type;

  Serial.print("error ");
  Serial.print(type);
  Serial.print(";");

  if (currentError > 999) return; //above 1000 are warnings

  
  while (currentError != 0) { //alternate loop while critical error
    sense();
    job();
    serialRead();
    Blynk.run();
  }
}

void continuityCheck() {
  if (digitalRead(fromGarage) && (digitalRead(fromBuffer) || digitalRead(fromWatering))) bool fromGarage = true;
  else bool fromGarage = false;

  if (
    (digitalRead(mainPump) == RelayOn //When the main pump is running, at least one input and one output valve should be open.
    &&
    ((digitalRead(fromWell) + fromGarage) == 0))
    ||
    (digitalRead(toBuffer) + digitalRead(toWatering) + digitalRead(toDump) + digitalRead(toTap) + digitalRead(toPink) + digitalRead(toGreen) + digitalRead(toBlue) + digitalRead(toRed) == 0))//todo watering sections
  {
    if (debug) Serial.println("debug contiCheckBAD;");
    jobStop();
    error(120);
  }
}
