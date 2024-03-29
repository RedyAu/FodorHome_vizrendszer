bool bufferUpper, bufferLower, waterUpper, waterLower, groundWater, bufferLowerChange, bufferUpperChange, waterLowerChange, waterUpperChange, prevTapSwitchSt;
unsigned long forSenseMillis, forBufferLowerMillis, forBufferUpperMillis, forWaterLowerMillis, forWaterUpperMillis;

void sense() { //Read data from temperature and humidity sensors and write it to variables so other functions can use them

  if (forSenseMillis < millis()) {
    forSenseMillis = millis() + 1500;

    static bool doTempReading;
    if (doTempReading) {
      bufferTemp = waterTemp.getTempCByIndex(0);
      doTempReading = false;
    } else {
      waterTemp.requestTemperatures();
      doTempReading = true;
    }

    switch (int(bufferTemp * 10)) {
      case -1270:
        //sensor disconnected todo
        error(1420);
        break;
      case 850:
        //sensor connected but reading failed
        error(1425);
        bufferTemp = -127.0;
        break;
    }
  }

  bool bufferLowerNow = digitalRead(bufferLvlLower);
  if (bufferLower != bufferLowerNow) { //If lower sensor senses a change, wait 5sec with the news to avoid switching back and forth
    if (!bufferLowerChange) {
      forBufferLowerMillis = millis() + 5000;
      bufferLowerChange = true;
    }
    else if (forBufferLowerMillis < millis()) {
      bufferLower = bufferLowerNow;
      bufferLowerChange = false;
    }
  }
  else bufferLowerChange = false;

  bool bufferUpperNow = digitalRead(bufferLvlUpper);
  if (bufferUpper != bufferUpperNow) { //If lower sensor senses a change, wait 5sec with the news to avoid switching back and forth
    if (!bufferUpperChange) {
      forBufferUpperMillis = millis() + 5000;
      bufferUpperChange = true;
    }
    else if (forBufferUpperMillis < millis()) {
      bufferUpper = bufferUpperNow;
      bufferUpperChange = false;
    }
  }
  else bufferUpperChange = false;

  bool waterLowerNow = digitalRead(waterLvlLower);
  if (waterLower != waterLowerNow) {
    if (!waterLowerChange) {
      forWaterLowerMillis = millis() + 5000;
      waterLowerChange = true;
    }
    else if (forWaterLowerMillis < millis()) {
      waterLower = waterLowerNow;
      waterLowerChange = false;
    }
  }

  bool waterUpperNow = digitalRead(waterLvlUpper);
  if (waterUpper != waterUpperNow) { //If lower sensor senses a change, wait 5sec with the news to avoid switching back and forth
    if (!waterUpperChange) {
      forWaterUpperMillis = millis() + 5000;
      waterUpperChange = true;
    }
    else if (forWaterUpperMillis < millis()) {
      waterUpper = waterUpperNow;
      waterUpperChange = false;
    }
  }
  else waterUpperChange = false;

  tapFlowControl();
}

int levelOf(int container) {
  //0: puffer 1: watering
  //return 0 if both sensors 0
  //return 1 if bottom sensor 1
  //return 2 if both sensors 1

  bool upper = container ? waterUpper : bufferUpper;
  bool lower = container ? waterLower : bufferLower;

  if (!lower && !upper) return 0;
  if (lower && !upper) return 1;
  if (lower && upper) return 2;

  if (!lower && upper) {
    if (container) error(1300);
    else {
      return 0;
      error(1310);
    }
  }
  return 0;
}
