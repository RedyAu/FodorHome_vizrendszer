bool cool() {
  static bool isBufferEmptying;
  static bool isWateringEmptying;
  static bool isBufferFilling;
  static bool isFlowPump;
  static unsigned long bufferEmptyingStartTime;
  static unsigned long bufferLastFilled = 0;
  
  auto bufferEmptyStop = [](){
    isBufferEmptying = false;
    fullEmpty = false;
    currentJob = waterJob{StopNext};
  };

  stdJunct(
    isFlowPump,
    (cooling && levelOf(Buffer) > 0),
    [](){
      isFlowPump = true;
      digitalWrite(flowPump, RelayOn);
      },
    [](){
      isFlowPump = false;
      digitalWrite(flowPump, RelayOff);
      }
  );

  if (!cooling) return Continue;
  
  if (!isBufferEmptying) {
    //start filling buffer if not full if not currently emptying 
    if (stdJunctAlwaysStart(
      isBufferFilling,
      (levelOf(Buffer) < 2),
      [](){
        currentJob = waterJob{NoStopNext, fromWell, toBuffer};
        isBufferFilling = true;
        },
      [](){
        currentJob = waterJob{StopNext};
        isBufferFilling = false;
        bufferLastFilled = millis();
        }
      )) return End;

    if (isWateringEmptying) { //if watering is already running, return to run water()
     if (levelOf(Watering) > 0) return Continue;
     else { //if emptied, stop
       isWateringEmptying = false;
       currentJob = waterJob{StopNext};
     }
    }

    if (bufferTemp > bufferTreshold) { //start buffer emptying if temperature exceeded
      isBufferEmptying = true;
      if (millis() - bufferLastFilled < bufferFilledTooSoonTreshold) {
        fullEmpty = true;
        if (debug) Serial.println("fullEmpty true");
      }
      bufferEmptyingStartTime = millis();
      currentJob = waterJob{NoStopNext, fromBuffer, toWatering};
      return End;
    }
    
  } else {
    if (levelOf(Watering) == 2) { //if watering tank full, start the öntözést az öntözőből 
      bufferEmptyStop();
      beginWatering(1500);//todo measure empty time
      isWateringEmptying = true;
      return Continue;
    }
    if (fullEmpty) { //stop if empty or timer ran out or watering tank is full
      if (levelOf(Buffer) == 0) bufferEmptyStop();
      else return End;
    } else {
      if (millis() - bufferEmptyingStartTime > bufferEmptyingDuration) bufferEmptyStop();
      else return End;
    }
  }

  return Continue;
}
