bool jobRead(int to, int from) {
  int fromCount = 0;
  int toCount = 0;

  if (from == 0) {
    fromCount = 1;
  } else {
    int from[] = {fromWell, fromBuffer, fromWatering, fromGarage};
    for (int i = 0; i < LEN(from); i++) {
      if (digitalRead(from[i] == RelayOn)) fromCount++;
    }
  }

  if (to == 0) {
    toCount = 1;
  } else {
    int to[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
    for (int i = 0; i < LEN(to); i++) {
      if (digitalRead(to[i] == RelayOn)) toCount++;
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
