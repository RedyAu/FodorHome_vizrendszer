bool bufferUpper, bufferLower, waterUpper, waterLower, groundWater, bufferLowerChange, bufferUpperChange, waterLowerChange, waterUpperChange, prevTapSwitchSt;
unsigned long forSenseMillis, forBufferLowerMillis, forBufferUpperMillis, forWaterLowerMillis, forWaterUpperMillis;

void sense() { //Read data from temperature and humidity sensors and write it to variables so other functions can use them

  if (forSenseMillis < millis()) {
    forSenseMillis = millis() + 2000;

    //udvarTemp = udvarDHT.readTemperature();
    //udvarHum = udvarDHT.readHumidity();
    //if (udvarTemp == nan || udvarHum == nan) error(1410); //read warning

    waterTemp.requestTemperatures();

    bufferTemp = waterTemp.getTempCByIndex(0);
    if (bufferTemp < -10.0 || bufferTemp > 80.0) {
      error(true, 1420);
      bufferTemp = 0;
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
