int tapFlowSequenceSuccesful;
int tapFlowSequenceDuring;
unsigned long tapFlowPressSequenceStarted;
bool tapFlowShort;

void tapFlowControl() {
  bool tapSwitch = digitalRead(tapFlowSwitch);
  
  static bool tapSwitchState;
  if (tapSwitch != tapSwitchState) {
    delay(5); //primitive debounce
    tapSwitchState = tapSwitch;
    if (debug) Serial.print("Tap switch turned ");
    if (debug) Serial.println(tapSwitch ? "ON" : "OFF");
    
    if (tapSwitchState) tapSwitchOn();
    else tapSwitchOff();
  }

  static unsigned long tapFlowShortStart;
  unsigned long tapFlowFirstDuringDuration = millis() - tapFlowPressSequenceStarted;
  if (tapFlowSequenceDuring == 1 && (tapFlowFirstDuringDuration > tapFlowSequenceFirstDoneByMillis) && !tapFlow) {
    if (debug) Serial.println("Ran out of time, starting short tapFlow");
    
    tapFlowShortStart = millis();
    tapFlowShort = true;
    tapFlow = true;
    tapFlowSequenceReset();
  }

  unsigned long tapFlowShortDuration = millis() - tapFlowShortStart;
  if (tapFlowShort && tapFlowShortDuration > tapFlowShortDurationMillis && tapFlow) {
    if (debug) Serial.println("Short tapFlow over.");
    tapFlow = false;
    tapFlowShort = false;
    tapFlowSequenceReset();
  }

  unsigned long tapFlowSequenceDuration = millis() - tapFlowPressSequenceStarted;
  if (tapFlowSequenceDuration > tapFlowSequenceMaximumTimeMillis) {
    //if (debug) Serial.println("Switch sequence stopped abruptly, ran out of time. Resetting...");
    tapFlowSequenceReset();
  }
}

void tapSwitchOn() {
  if (tapFlow) return; //If tap is already running, disregard switch (maybe turned on from smartphone, etc)
  
  switch(tapFlowSequenceSuccesful) {
    case 0:
      if (debug) Serial.println("Listening for sequence...");
      tapFlowPressSequenceStarted = millis();
      tapFlowSequenceDuring = 1;
      tapFlowShort = false;
      break;
    case 1:
      if (debug) Serial.println("2nd during");
      tapFlowSequenceDuring = 2;
      break;
    case 2:
      if (debug) Serial.println("All three presses done.");
      unsigned long timePassed = millis() - tapFlowPressSequenceStarted;
      if (timePassed > tapFlowSequenceMinimumTimeMillis && timePassed < tapFlowSequenceMaximumTimeMillis) tapFlow = true;
      else tapFlowSequenceReset();
      break;
    default:
      error(true, 1500);
      tapFlowSequenceReset();
      break;
  }
}

void tapSwitchOff() {
  if (tapFlow) { //If tap is already running, just turn it off, don't do any of the switch sequence counting.
    tapFlow = false;
    return;
  }
  switch (tapFlowSequenceDuring) {
    case 1:
      if (debug) Serial.println("1st succesful");
      tapFlowSequenceSuccesful = 1;
      tapFlowSequenceDuring = 0;
      break;
    case 2:
      if (debug) Serial.println("2nd succesful");
      tapFlowSequenceSuccesful = 2;
      tapFlowSequenceDuring = 0;
      break;
    default:
      error(true, 1510);
      tapFlowSequenceReset();
      break;
  }
}

void tapFlowSequenceReset() {
  //if (debug) Serial.println("Resetting tapFlow sequence...");
  tapFlowSequenceDuring = 0;
  tapFlowSequenceSuccesful = 0;
  
}
