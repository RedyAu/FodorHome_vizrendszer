void error(int type) {
  currentError = type;

  terminal.print("error ");
  terminal.print(type);
  terminal.print(";");

  Blynk.virtualWrite(V101, currentError);
  Blynk.virtualWrite(V103, (currentError != 0 && currentError < 999) ? 255 : 0);

  if (currentError > 999) return; //above 1000 are warnings, loop not locked

  currentJob = waterJob{StopNext};
  cooling = false;
  dumping = false;
  tapFlow = false;
  watering = false;
  wateringFinished = true;
  
  job();
  jobDo();

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
    if (debug) terminal.println("debug contiCheckBAD;");
    jobStop();
    error(120);
  }
}
