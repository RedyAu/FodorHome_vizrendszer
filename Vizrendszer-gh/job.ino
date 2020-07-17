/*Example
  if (
      stdJunct(
        isWaterWatering,
        (waterLevel > 0),
        [] () {std::cout << "Starting...";},
        [] () {std::cout << "Stopping...";}
        )
  ) std::cout << "Returning from code block...";
  else std::cout << "Continuing...";
 */
bool stdJunct(bool isRunning, bool shouldRun, void Start(), void Stop()) {
  switch (isRunning + shouldRun) {
    case 0:
      return Continue;
    case 1:
      if (shouldRun) {
        Start();
        return End;
      }
      else {
        Stop();
        return Continue;
      }
    case 2:
      return End;
  }
}
bool stdJunctAlwaysStart(bool isRunning, bool shouldRun, void Start(), void Stop()) {
  switch (isRunning + shouldRun) {
    case 0:
      return Continue;
    case 1:
      if (shouldRun) {
        Start();
        return End;
      }
      else {
        Stop();
        return Continue;
      }
    case 2:
      Start();
      return End;
  }
}

bool tapAndDump() {
  static bool isBufferEmptying;
  static bool isWateringEmptying;
  static bool isWellTapFlow;
  
  switch (dumping + tapFlow) {
    case 0:
      return Continue;
      break;
    case 2:
      error(110);
      break;
  }
  
  if (dumping) {
    if (stdJunctAlwaysStart(
      isBufferEmptying,
      (levelOf(Buffer) > 0),
      [](){
        currentJob = waterJob{NoStopNext, fromBuffer, toDump};
        isBufferEmptying = true;
        },
      [](){
        currentJob = waterJob{StopNext};
        isBufferEmptying = false;
        }
      )
    ) return End;
  }

  if (stdJunctAlwaysStart(
    isWateringEmptying,
    (levelOf(Watering) > 0),
    [](){
      currentJob = waterJob{NoStopNext, fromWatering, tapFlow ? toTap : toDump};
      isWateringEmptying = true;
      },
    [](){
      currentJob = waterJob{StopNext};
      isWateringEmptying = false;
      }
    )
  ) return End;

  if (dumping) {
    dumping = false;
    //todo dumping done
    return Continue;
  }

  if (stdJunctAlwaysStart(
    isWellTapFlow,
    tapFlow,
    [](){
      currentJob = waterJob{NoStopNext, fromWell, toTap};
      isWellTapFlow = true;
      },
    [](){
      currentJob = waterJob{StopNext};
      isWellTapFlow = false;
      }
    )
  ) return End;
  
  return Continue;
}


void beginWatering(unsigned long duration) {
  Serial.print("Beginning watering session for ");
  Serial.println(duration);
  //calculate one unit time from duration and set weights - then continue
}

bool water() {
  //if haven't beginWater yet since last done, return. (?)
  //if lastUpdate old, move startTime
  //start current section based on startTime (rollover!! difference)
  //if finished reached, running false
  static unsigned long wateringStart;
  unsigned long pastDuration; //name wip
  static unsigned long lastUpdate;
}

void job() { /////////////////////////////////////////////////////////////////
  //Tap/Dump
  if (tapAndDump()) return;
  
  //Cool
  if (cool()) return;
  
  //Water
  water();

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

/*
const unsigned long bufferEmptyingTime = 150000;
unsigned long forMillisEmptying = 0; //The target millis to stop at. Automatically calculated at start of emptying by adding prev constant to current millis()

extern int currentTask;
extern float bufferTemp;

bool emptyingToWatering = false;
bool emptyingToDump = false;

bool flowPumpSt = false;

void water() {
  if (isEmptying) { //If we should empty, first do it in watering tank, and if that's filled, continue to dump.
    if (waterLevel(1) < 2) {
      if (!emptyingToWatering) {

        if (debug) Serial.print("Watering tank.\n\n"); //End of another message.

        emptyingToDump = false;
        emptyingToWatering = true;
        waterStart(1, 3);
      }
    }
    else if (!emptyingToDump) {

      if (debug) Serial.print("Dump.\n\n"); //End of another message.

      emptyingToDump = true;
      emptyingToWatering = false;
      waterStart(1, 0);
    }
  }

  if (cooling && waterLevel(0) > 1) {
    if (!flowPumpSt) {

      if (debug) Serial.println("Circulation pump turned ON.");

      flowPumpSt = true;
      digitalWrite(flowPump, RelayOn);
    }
  }
  if (waterLevel(0) < 1 || !cooling) {
    if (flowPumpSt) {

      if (debug) Serial.println("Circulation pump turned OFF.");

      flowPumpSt = false;
      digitalWrite(flowPump, RelayOff);
    }
  }

  //If our new task is not as important as the currently running task, terminate it. If it's the same, let it run, because that's how we know when the task should do something else. On task change, first stop all.
  switch (currentTask) {
    case 1: //Cooling

      if (isFilling) { //If we're filling the buffer tank,
        if (waterLevel(0) == 2) {//and if it's filled

          if (debug) Serial.print("\n\nBuffer tank is full. Filling stopped.\n\n");

          jobStop(); //stop filling it
          isFilling = false;

          delay(200);
          return;
        }
        else return; //otherwise continue
      }

      if (isEmptying) { //If we're emptying from the buffer tank
        if (veryCool) { //If we're supposed to empty it all the way
          if (waterLevel(0) == 0) { //If it's empty

            if (debug) Serial.print("\n\nBuffer tank emptying stopped. Tank empty.\nveryCool turned off, using timer next time.\n\n");

            isEmptying = false; //Stop emptying
            jobStop();

            veryCool = false; //Next time use timer.

            if (bufferTempDoTimer) tempTimerSt = 1;
            return;
          }
          else return; //Otherwise continue
        }
        else { //Otherwise
          if (millis() > forMillisEmptying) { //If a specific time has passed

            if (debug) Serial.print("\n\nBuffer tank emptying stopped. Timer reached 0.\n\n");

            isEmptying = false;//Stop emptying
            jobStop();

            if (bufferTempDoTimer) tempTimerSt = 1;
            return;
          }
          else return;//Otherwise continue
        }
      }

      if (waterLevel(0) == 2) { //If the buffer tank is filled
        if (bufferTreshold < bufferTemp) { //If the buffer temp exeeds the allowed limit

          if (debug) Serial.print("\n\nTarget temperature exceeded. Starting emptying to ");

          emptyingToDump = false;
          emptyingToWatering = false;

          isEmptying = true; //empty some water
          if (!veryCool) forMillisEmptying = millis() + bufferEmptyingTime;
          return;
        }
        else return; //Otherwise continue
      }
      else if (!isFilling) { //Otherwise start filling

        if (debug) Serial.print("\n\nBuffer Tank not full, started filling.\n\n");

        isFilling = true;
        waterStart(0, 2);
        return;
      }

      break;

    //🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴

    case 2: //Tap
      if (waterLevel(1) > 0) { //If there is water in the buffer tank
        if (isWaterWatering) return; //If we're already watering, continue
        else { //Otherwise start watering

          if (debug) Serial.println("debug tapStartWater;");

          isBufferWatering = false;
          isWellWatering = false;
          isWaterWatering = true;
          waterStart(2, 1);
          return;
        }
      }
      else { //Otherwise
        if (isWellWatering) return; //If we're watering from the well, continue
        else { //otherwise start it

          if (debug) Serial.println("debug tapStartWell;");

          isBufferWatering = false;
          isWaterWatering = false;
          isWellWatering = true;
          waterStart(0, 1);
          return;
        }
      }
      break;

    //🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴

    case 3: //Dumping
      if (waterLevel(0) > 0) {
        if (isBufferEmptying) return;
        else {

          if (debug) Serial.println("debug dumpingBuffer;");

          waterStart(1, 0);
          isBufferEmptying = true;
          return;
        }
      }
      if (waterLevel(1) > 0) {
        if (isWaterEmptying) return;
        else {

          if (debug) Serial.println("debug dumpingWatering;");

          waterStart(2, 0);
          isBufferEmptying = false;
          isWaterEmptying = true;
          return;
        }
      }
      else {
        isWaterEmptying = false;
        dumping = false;
        Serial.println("dumpDone 0;");
        jobStop();
        return;
      }

      break;
  }
  
}*/
