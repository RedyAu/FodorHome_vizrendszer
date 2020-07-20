bool tapAndDump() {
  /* Combined function for getting water from tap and dumping from the tanks.
   * Dumping empties both tanks and than finishes.
   * Tap flow only empties watering tank, and than continues indefinetely from well.
   */
  switch (dumping + tapFlow) {
    case 0: //return if neither feature is activated
      return Continue;
      break;
    case 2: //if both are activated, drop dumping task and throw warning
      error(1100);
      dumping = false;
      break;
  }
  
  if (dumping) {
    if (levelOf(Buffer) > 0) {
      currentJob = waterJob{NoStopNext, fromBuffer, toDump};
      return End;
    }
    else currentJob = waterJob{StopNext};
  }

  if (levelOf(Watering) > 0) {
    currentJob = waterJob{NoStopNext, fromWatering, tapFlow ? toTap : toDump};
    return End;
  }
  else currentJob = waterJob{StopNext};
  
  if (dumping) { //if both containers emptied, dumping task is done.
    dumping = false;
    Serial.println("\nBoth containers empy, dumping task is done.");
    return Continue;
  }
  //TapFlow continues from well after watering container is emptied.

  if (tapFlow) {
    currentJob = waterJob{NoStopNext, fromWell, toTap};
    return End;
  } else {
    currentJob = waterJob{StopNext};
  }
  
  return Continue;
}

void job() { /////////////////////////////////////////////////////////////////
  //Tap/Dump
  if (tapAndDump()) return;
  
  //Cool
  if (cool()) return;
  
  //Water
  if (water()) return;

  //If end of tree reached, stop and reset.
  currentJob = waterJob{StopNext};
}

/*
 General structure
wholeThing() {
 if (doThis()) return;
 doAfter();
}

 bool doThis() {
  if (continueCondition) {
    doSomething();
    return continue;
  } else {
    doOtherStuff;
    return end;
  }
 }
*/
