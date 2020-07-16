int invalidTask = 0;

int currentTask;

void waterTasker() {
  //if cooling is true and cooling task is not set, set it. (separate integer, not function call data)

  //if we should do multiple things at once, stop everything and throw and error. (if it was he case for three times)
  if ((cooling + dumping + tapFlow) > 1) {
    invalidTask++;

    if (invalidTask > 2) {
      allStop(false);
      error(true,111);
    }

    return;
  }

  invalidTask = 0;

  if (cooling && currentTask != 1) {

    if (debug) Serial.println("debug waterTaskCooling;");

    if (currentTask != 0) allStop(false);
    currentTask = 1;
  }
  else if (dumping && currentTask != 3) {

    if (debug) Serial.println("debug waterTaskDump;");

    if (currentTask != 0) allStop(false);
    currentTask = 3;
  }
  
  else if (tapFlow && currentTask != 2) {

    if (debug) Serial.println("debug waterTaskTap;");

    if (currentTask != 0) allStop(false);
    currentTask = 2;
  }
  else if ((cooling + dumping + tapFlow) == 0) {
    if (currentTask != 0) {

      if (debug) Serial.println("debug waterTaskClear;");

      currentTask = 0;
      allStop(true);
    }
  }


}
