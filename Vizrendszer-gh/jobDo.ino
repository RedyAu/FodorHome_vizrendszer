void jobStop() {
  if (debug) Serial.println("debug jobStop;");

  digitalWrite(mainPump, RelayOff);
  digitalWrite(toDump, RelayOn); //To releive excess pressure

  delay(100);

  digitalWrite(fromWell, RelayOff);
  digitalWrite(fromGarage, RelayOff);
  digitalWrite(fromBuffer, RelayOff);
  digitalWrite(fromWatering, RelayOff);

  delay(500);

  digitalWrite(toBuffer, RelayOff);
  digitalWrite(toWatering, RelayOff);
  digitalWrite(toDump, RelayOff);
  digitalWrite(toTap, RelayOff);
}

void jobDo() {
  static waterJob previousJob;
  if (previousJob.stop == currentJob.stop && previousJob.from == currentJob.from && previousJob.to == currentJob.to) return;
  previousJob = currentJob;

  if (currentJob.stop) {
    jobStop();
    return;
  }

  switch (currentJob.from) {
    case fromWell://From Well
      digitalWrite(fromWell, RelayOn);
      Serial.print("fromS 0;");
      break;
    case fromBuffer://From Buffer
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromBuffer, RelayOn);
      Serial.print("fromS 1;");
      break;
    case fromWatering://From Watering
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromWatering, RelayOn);
      Serial.print("fromS 2;");
      break;
    default:
      error(100);
  }

  switch (currentJob.to) {
    case toDump:
      digitalWrite(toDump, RelayOn);
      Serial.print("toS 0;");
      break;
    case toTap:
      digitalWrite(toTap, RelayOn);
      Serial.print("toS 1;");
      break;
    case toBuffer:
      digitalWrite(toBuffer, RelayOn);
      Serial.print("toS 2;");
      break;
    case toWatering:
      digitalWrite(toWatering, RelayOn);
      Serial.print("toS 3;");
      break;
    case toPink:
      digitalWrite(toPink, RelayOn);
      Serial.print("to pink");
      break;
    case toGreen:
      digitalWrite(toGreen, RelayOn);
      Serial.print("to green");
      break;
    case toBlue:
      digitalWrite(toBlue, RelayOn);
      Serial.print("to blue");
      break;
    case toRed:
      digitalWrite(toRed, RelayOn);
      Serial.print("to red");
      break;
    /*case toGrey://To Grey
      break;*/
    default:
      error(101);
  }

  continuityCheck();

  digitalWrite(mainPump, RelayOn);
  Serial.print("mainPump 1;");
}
