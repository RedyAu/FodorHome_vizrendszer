bool jobRead(int to, int from) {
  int fromCount = 0;
  int toCount = 0;

  if (from == 0) {
    fromCount = 1;
  } else {
    int fromPins[] = {fromWell, fromBuffer, fromWatering, fromGarage};
    for (int i = 0; i < LEN(fromPins); i++) {
      bool pin = digitalRead(fromPins[i]);
      if (pin == RelayOn) {
        if (fromPins[i] == from) fromCount++;
        else fromCount --;
      }
    }
  }

  if (to == 0) {
    toCount = 1;
  } else {      
    int toPins[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
    for (int i = 0; i < LEN(toPins); i++) {
      bool pin = digitalRead(toPins[i]);
      if (pin == RelayOn) {
        if (toPins[i] == to) toCount++;
        else toCount --;
      }
    }
  }
  
  return (fromCount + toCount) == 2;
}

bool isBufferEmptying() {
  return jobRead(fromBuffer, toWatering);
}
bool isWateringEmptying() {
  return jobRead(fromWatering, 0);
}
bool isBufferFilling() {
  return jobRead(fromWell, toBuffer);
}
