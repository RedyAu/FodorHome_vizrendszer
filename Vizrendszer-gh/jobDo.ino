void jobStop() {
  if (debug) Serial.println("debug jobStop;");

  digitalWrite(mainPump, RelayOff);
  
  //digitalWrite(toDump, RelayOn); //To releive excess pressure

  delay(100);

  byte fromPins[] = {fromWell, fromBuffer, fromWatering, fromGarage};
  digitalWriteGroup(fromPins, LEN(fromPins), RelayOff);

  delay(500);

  byte toPins[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
  digitalWriteGroup(toPins, LEN(toPins), RelayOff);
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
      Serial.print("fromS fromWell;");
      break;
    case fromBuffer://From Buffer
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromBuffer, RelayOn);
      Serial.print("fromS fromBuffer;");
      break;
    case fromWatering://From Watering
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromWatering, RelayOn);
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
      Serial.print("toS toDump;");
      break;
    case toTap:
      digitalWrite(toTap, RelayOn);
      Serial.print("toS toTap;");
      break;
    case toBuffer:
      digitalWrite(toBuffer, RelayOn);
      Serial.print("toS toBuffer;");
      break;
    case toWatering:
      digitalWrite(toWatering, RelayOn);
      Serial.print("toS toWatering;");
      break;
    case toPink:
      digitalWrite(toPink, RelayOn);
      Serial.print("to pink;");
      break;
    case toGreen:
      digitalWrite(toGreen, RelayOn);
      Serial.print("to green;");
      break;
    case toBlue:
      digitalWrite(toBlue, RelayOn);
      Serial.print("to blue;");
      break;
    case toRed:
      digitalWrite(toRed, RelayOn);
      Serial.print("to red;");
      break;
    /*case toGrey://To Grey
      break;*/
    default:
      error(101);
  }

  digitalWrite(mainPump, RelayOn);
  Serial.print("mainPump 1;");
}
