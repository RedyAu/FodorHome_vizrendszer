void jobStop() {
  if (debug) Serial.println("debug jobStop;");

  digitalWrite(mainPump, RelayOff);
  Blynk.virtualWrite(mainPump, 0);
  
  digitalWrite(toDump, RelayOn); //To releive excess pressure

  delay(100);

  byte fromPins[] = {fromWell, fromBuffer, fromWatering, fromGarage};
  digitalWriteGroup(fromPins, LEN(fromPins), RelayOff);
  for (int i = 0; i < LEN(fromPins); i++) {
    Blynk.virtualWrite(fromPins[i], 0);
  }

  delay(500);

  byte toPins[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
  digitalWriteGroup(toPins, LEN(toPins), RelayOff);
  
  Blynk.virtualWrite(toDump, 0);
  Blynk.virtualWrite(toTap, 0);
  Blynk.virtualWrite(toBuffer, 0);
  Blynk.virtualWrite(toWatering, 0);
  Blynk.virtualWrite(10, 0);
  Blynk.virtualWrite(13, 0);
  Blynk.virtualWrite(16, 0);
  Blynk.virtualWrite(19, 0);
}

void jobDo() {
  static waterJob previousJob;
  if ((previousJob.stop == currentJob.stop) && (previousJob.from == currentJob.from) && (previousJob.to == currentJob.to)) return;
  previousJob = currentJob;

  if (currentJob.stop) {
    jobStop();
    return;
  }

  byte from[] = {fromWell, fromBuffer, fromWatering, fromGarage};
  digitalWriteGroup(from, LEN(from), RelayOff);
  switch (currentJob.from) {
    case fromWell://From Well
      digitalWrite(fromWell, RelayOn);
      Blynk.virtualWrite(fromWell, 255);
      Serial.print("fromS fromWell;");
      break;
    case fromBuffer://From Buffer
      digitalWrite(fromGarage, RelayOn);
      Blynk.virtualWrite(fromGarage, 255);
      digitalWrite(fromBuffer, RelayOn);
      Blynk.virtualWrite(fromBuffer, 255);
      Serial.print("fromS fromBuffer;");
      break;
    case fromWatering://From Watering
      digitalWrite(fromGarage, RelayOn);
      Blynk.virtualWrite(fromGarage, 255);
      digitalWrite(fromWatering, RelayOn);
      Blynk.virtualWrite(fromWatering, 255);
      Serial.print("fromS fromWatering;");
      break;
    default:
      error(100);
  }

  byte to[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
  digitalWriteGroup(to, LEN(to), RelayOff);
  switch (currentJob.to) {
    case toDump:
      digitalWrite(toDump, RelayOn);
      Blynk.virtualWrite(toDump, 255);
      Serial.print("toS toDump;");
      break;
    case toTap:
      digitalWrite(toTap, RelayOn);
      Blynk.virtualWrite(toTap, 255);
      Serial.print("toS toTap;");
      break;
    case toBuffer:
      digitalWrite(toBuffer, RelayOn);
      Blynk.virtualWrite(toBuffer, 255);
      Serial.print("toS toBuffer;");
      break;
    case toWatering:
      digitalWrite(toWatering, RelayOn);
      Blynk.virtualWrite(toWatering, 255);
      Serial.print("toS toWatering;");
      break;
    case toPink:
      digitalWrite(toPink, RelayOn);
      Blynk.virtualWrite(10, 255);
      Serial.print("to pink;");
      break;
    case toGreen:
      digitalWrite(toGreen, RelayOn);
      Blynk.virtualWrite(13, 255);
      Serial.print("to green;");
      break;
    case toBlue:
      digitalWrite(toBlue, RelayOn);
      Blynk.virtualWrite(16, 255);
      Serial.print("to blue;");
      break;
    case toRed:
      digitalWrite(toRed, RelayOn);
      Blynk.virtualWrite(19, 255);
      Serial.print("to red;");
      break;
    /*case toGrey://To Grey
      break;*/
    default:
      error(101);
  }

  digitalWrite(mainPump, RelayOn);
  Blynk.virtualWrite(mainPump, 255);
  Serial.print("mainPump 1;");
}
