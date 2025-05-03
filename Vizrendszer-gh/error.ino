void error(int type) {
  currentError = type;

  terminal.print("error ");
  terminal.print(type);
  terminal.print(";");

  Blynk.virtualWrite(V101, currentError);
  Blynk.virtualWrite(V103, (currentError != 0 && currentError < 999) ? 255 : 0);

  if ((currentError > 999) || currentError == 0) return; //above 1000 are warnings, loop not locked

  currentJob = waterJob{StopNext};
  directTap = false;
  watering = false;
  wateringFinished = true;
  currentSession = emptySession;
  
  job();
  jobDo();

  while (currentError != 0) { //alternate loop while critical error
    sense();
    job();
    Blynk.run();
  }
}

void continuityCheck() {
  return;
}
