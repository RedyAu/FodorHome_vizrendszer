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
      digitalWrite(flowPump, relayON);
    }
  }
  if (waterLevel(0) < 1 || !cooling) {
    if (flowPumpSt) {

      if (debug) Serial.println("Circulation pump turned OFF.");

      flowPumpSt = false;
      digitalWrite(flowPump, relayOFF);
    }
  }

  //If our new task is not as important as the currently running task, terminate it. If it's the same, let it run, because that's how we know when the task should do something else. On task change, first stop all.
  switch (currentTask) {
    case 1: //Cooling

      if (isFilling) { //If we're filling the buffer tank,
        if (waterLevel(0) == 2) {//and if it's filled

          if (debug) Serial.print("\n\nBuffer tank is full. Filling stopped.\n\n");

          allStop(false); //stop filling it
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
            allStop(false);

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
            allStop(false);

            if (bufferTempDoTimer) tempTimerSt = 1;
            return;
          }
          else return;//Otherwise continue
        }
      }

      if (waterLevel(0) == 2) { //If the buffer tank is filled
        if (bufferTarget < bufferTemp) { //If the buffer temp exeeds the allowed limit

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
        allStop(false);
        return;
      }

      break;
  }
}
