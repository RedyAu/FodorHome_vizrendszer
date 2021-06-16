bool cool() {
  static unsigned long bufferEmptyingStartTime;
  static unsigned long bufferLastFilled = 0;
  static bool isBufferLastFilledActive = false;

  if ((cooling && isBufferLastFilledActive && !fullEmpty && levelOf(Buffer) == 1) || (cooling && levelOf(Buffer) == 2)) {
    if (digitalRead(flowPump) == RelayOff) {
      digitalWrite(flowPump, RelayOn);
      Blynk.virtualWrite(flowPump, 255);
      terminal.println("flowPump on");
    }
  }
  else if (digitalRead(flowPump) == RelayOn) {
    digitalWrite(flowPump, RelayOff);
    Blynk.virtualWrite(flowPump, 0);
    terminal.println("flowPump off");
  }

  if (!cooling) {
    isBufferLastFilledActive = false;
    return Continue;
  }

  if (!isBufferEmptying()) {
    //start filling buffer if not full if not currently emptying
    if (levelOf(Buffer) < 2) {
      currentJob = waterJob{NoStopNext, fromWell, toBuffer};
      return End;
    } else { //Buffer tank is filled
      currentJob = waterJob{StopNext};
      if (isBufferFilling()) {
        bufferLastFilled = millis();
        isBufferLastFilledActive = true;
        fullEmpty = false;
      }
    }

    if (bufferTemp > bufferTreshold) { //start buffer emptying if temperature exceeded
      if ((millis() - bufferLastFilled < bufferFilledTooSoonTreshold) && isBufferLastFilledActive) {
        fullEmpty = true;
        if (debug) terminal.println("fullEmpty true");
      }
      bufferEmptyingStartTime = millis();

      if (isCoolingWatering) { //if we want to do watering with the cooling water
        if (levelOf(Watering) == 2) { //If watering tank becomes full while emptying, finish to dump before starting to water.
          currentJob = waterJob{NoStopNext, fromBuffer, toDump};
        } else {
          currentJob = waterJob{NoStopNext, fromBuffer, toWatering};
        }
      } else {
        currentJob = waterJob{NoStopNext, fromBuffer, toDump};
      }

      return End;
    }

  } else {
    if (levelOf(Watering) == 2) currentJob = waterJob{NoStopNext, fromBuffer, toDump}; //if watering tank has become full, continue empty to dump
    if (levelOf(Watering) == 2 && isCoolingWatering) { //if watering tank full, start the öntözést az öntözőből
      if (!watering) {
        beginWatering(1200000, Cooling);//20 minutes (will pause when tank is empty)
      }
    }
    if (fullEmpty) { //stop if empty or timer ran out or watering tank is full
      if (levelOf(Buffer) == 0) {
        currentJob = waterJob{StopNext};
        return End;
      }
      else return End;
    } else {
      if (millis() - bufferEmptyingStartTime > bufferEmptyingDuration) {
        currentJob = waterJob{StopNext};
        return End;
      }
      else return End;
    }
  }
  
  if (isWateringEmptying()) { //if watering is already running, return to run water()
    if (levelOf(Watering) > 0) return Continue;
    else { //if emptied, stop
      currentJob = waterJob{StopNext};
    }
  }

  return Continue;
}
