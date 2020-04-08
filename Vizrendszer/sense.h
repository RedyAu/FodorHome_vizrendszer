bool bufferUpper, bufferLower, waterUpper, waterLower, groundWater, bufferLowerChange, bufferUpperChange, waterLowerChange, prevTapSwitchSt;
unsigned long forSenseMillis, forBufferLowerMillis, forBufferUpperMillis, forWaterLowerMillis;

void sense() { //Read data from temperature and humidity sensors and write it to variables so other functions can use them

  if (forSenseMillis < millis()) {
    forSenseMillis = millis() + 2000;

    //garAknTemp = garazsaknaDHT.readTemperature();
    //garAknHum = garazsaknaDHT.readHumidity();
    //if (garAknTemp == nan || garAknHum == nan) error(1400); //read warning

    //udvarTemp = udvarDHT.readTemperature();
    //udvarHum = udvarDHT.readHumidity();
    //if (udvarTemp == nan || udvarHum == nan) error(1410); //read warning
    /*
        waterTemp.requestTemperatures();

        bufferTemp = waterTemp.getTempCByIndex(0);
        if (bufferTemp < -10.0 || bufferTemp > 80.0) {
          error(true, 1420);
          bufferTemp = 0;
      }*/
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

  bool tapSwitchSt = digitalRead(tapFlowSwitch); //The switch being in the upper position means tapFlow
  if ((prevTapSwitchSt != tapSwitchSt) && (tapFlow != tapSwitchSt)) {
    prevTapSwitchSt = tapSwitchSt;
    allStop(true);
    tapFlow = tapSwitchSt;
    Serial.println("Tap Switch Changed");
  }

  determineUpper();

  if (groundWater) error(false, 1320);
}

unsigned long mTickMillis = 0;
int mTick = 0, dTick = 0, groundTick, waterTick;
float sumGndM, sumWaterM, prevGndM, prevWaterM, nowWaterM, nowGndM;

void determineUpper() { //A really complicated soulution to reading water sensors. I found that they don't have a specific value when in or out of water
  if (mTickMillis < millis()) {// beacuse water sticks to the panel and it takes time to dry. What I can sense is that the signal gets way noisier when
    mTickMillis = millis() + 10; // in water, so the following code senses noise, and runs it trough 3 layers of abstarction to be sure.

    mTick++;
    if (mTick > 9) {
      mTick = 0;
      dTick++;

      if (sumGndM > 100) groundTick++; //Second: If the difference is more than a certain value, add to an intermediary.
      sumGndM = 0;

      if (sumWaterM > 32) waterTick++;
      sumWaterM = 0;

      if (dTick > 5) {
        dTick = 0;

        //if (groundTick >= 3) groundWater = true; //If difference exceeds the treshold at least three times of five, give a decision.
        //else groundWater = false;
        groundTick = 0;

        if (waterTick >= 3) waterUpper = true;
        else waterUpper = false;
        waterTick = 0;
      }
    }
    prevGndM = nowGndM; //First: Read voltages from sensors and get the absolute difference compared to last measurement.
    nowGndM = analogRead(A14);
    prevWaterM = nowWaterM;
    nowWaterM = analogRead(A15);

    sumGndM += abs(prevGndM - nowGndM);
    sumWaterM += abs(prevWaterM - nowWaterM);
  }
}
