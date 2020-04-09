int tapFlowSequenceSuccesful;
int tapFlowSequenceDuring;
unsigned long tapFlowPressSequenceStarted;

void tapFlowControl() {
  bool tapSwitch = !digitalRead(tapFlowSwitch); //pullup means low = switch turned on (switch closed)
  static bool tapSwitchState;
  if (tapSwitch != tapSwitchState) {
    delay(5); //primitive debounce
    tapSwitchState = tapSwitch;
    if (tapSwitchState) tapSwitchOn();
    else tapSwitchOff();
  }

  static bool tapFlowShort;
  static unsigned long tapFlowShortStart;
  if (tapFlowSequenceDuring == 1 && (millis() - tapFlowPressSequenceStarted > tapFlowSequenceFirstDoneByMillis) && !tapFlow) {
    tapFlowShortStart = millis();
    tapFlowShort = true;
    tapFlow = true;
    tapFlowSequenceReset();
  }

  if (tapFlowShort && millis() - tapFlowShortStart > tapFlowShortDurationMillis && tapFlow) {
    tapFlow = false;
    tapFlowSequenceReset();
  }

  if (millis() - tapFlowPressSequenceStarted > tapFlowSequenceMaximumTimeMillis) tapFlowSequenceReset();
}

void tapSwitchOn() {
  if (tapFlow) return; //If tap is already running, disregard switch (maybe turned on from smartphone, etc)
  
  switch(tapFlowSequenceSuccesful) {
    case 0:
      tapFlowPressSequenceStarted = millis();
      tapFlowSequenceDuring = 1;
      break;
    case 1:
      tapFlowSequenceDuring = 2;
      break;
    case 2:
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
      tapFlowSequenceSuccesful = 1;
      tapFlowSequenceDuring = 0;
      break;
    case 2:
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
  tapFlowSequenceDuring = 0;
  tapFlowSequenceSuccesful = 0;
  
}
