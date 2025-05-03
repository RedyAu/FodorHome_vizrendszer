int tapFlowSequenceSuccesful = 0;
int tapFlowSequenceDuring = 0;
unsigned long tapFlowPressSequenceStarted;
bool tapFlowShort;

void tapFlowControl() {
  bool tapSwitch = digitalRead(tapFlowSwitch);

  static bool tapSwitchState;
  if (tapSwitch != tapSwitchState) {
    delay(50); //primitive debounce
    tapSwitchState = tapSwitch;
    
    directTap = tapSwitch;       
  }
}
