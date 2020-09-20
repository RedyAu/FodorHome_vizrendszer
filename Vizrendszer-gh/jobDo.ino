void jobStop() {
  terminal.println("\njobDo: Stopping.\n");

  digitalWrite(mainPump, RelayOff);

  //digitalWrite(toDump, RelayOn); //To releive excess pressure

  delay(100);

  byte fromPins[] = {fromWell, fromBuffer, fromWatering, fromGarage};
  digitalWriteGroup(fromPins, LEN(fromPins), RelayOff);

  delay(500);

  byte toPins[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
  digitalWriteGroup(toPins, LEN(toPins), RelayOff);

  Blynk.virtualWrite(V60, 0); //Display 0 on watering progress bar after stopping
  Blynk.virtualWrite(V62, 0);

  blynkJobUpdate();
}

void blynkJobUpdate() {
  Blynk.virtualWrite(fromGarage, (digitalRead(fromGarage) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromBuffer, (digitalRead(fromBuffer) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromWatering, (digitalRead(fromWatering) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(fromWell, (digitalRead(fromWell) == RelayOn) ? 255 : 0);

  Blynk.virtualWrite(toDump, (digitalRead(toDump) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toTap, (digitalRead(toTap) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toBuffer, (digitalRead(toBuffer) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(toWatering, (digitalRead(toWatering) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(10, (digitalRead(toPink) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(13, (digitalRead(toGreen) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(16, (digitalRead(toBlue) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(19, (digitalRead(toRed) == RelayOn) ? 255 : 0);

  Blynk.virtualWrite(mainPump, (digitalRead(mainPump) == RelayOn) ? 255 : 0);

  Blynk.virtualWrite(V0, isBufferEmptying() ? 255 : 0);
  Blynk.virtualWrite(V1, isBufferFilling() ? 255 : 0);
}

void jobDo() {
  static waterJob previousJob;
  if ((previousJob.stop == currentJob.stop) && (previousJob.from == currentJob.from) && (previousJob.to == currentJob.to)) return;
  previousJob = currentJob;

  if (currentJob.stop) {
    jobStop();
    return;
  }

  terminal.print("jobDo: ");

  byte from[] = {fromWell, fromBuffer, fromWatering, fromGarage};
  digitalWriteGroup(from, LEN(from), RelayOff);
  switch (currentJob.from) {
    case fromWell://From Well
      digitalWrite(fromWell, RelayOn);
      terminal.print("well");
      break;
    case fromBuffer://From Buffer
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromBuffer, RelayOn);
      terminal.print("buffer tank");
      break;
    case fromWatering://From Watering
      digitalWrite(fromGarage, RelayOn);
      digitalWrite(fromWatering, RelayOn);
      terminal.print("watering tank");
      break;
    default:
      error(100);
  }

  terminal.print(" -> ");

  byte to[] = {toDump, toTap, toBuffer, toWatering, toPink, toGreen, toBlue, toRed};
  digitalWriteGroup(to, LEN(to), RelayOff);
  switch (currentJob.to) {
    case toDump:
      digitalWrite(toDump, RelayOn);
      terminal.print("dump");
      break;
    case toTap:
      digitalWrite(toTap, RelayOn);
      terminal.print("tap");
      break;
    case toBuffer:
      digitalWrite(toBuffer, RelayOn);
      terminal.print("buffer tank");
      break;
    case toWatering:
      digitalWrite(toWatering, RelayOn);
      terminal.print("watering tank");
      break;
    case toPink:
      digitalWrite(toPink, RelayOn);
      terminal.print("pink zone");
      break;
    case toGreen:
      digitalWrite(toGreen, RelayOn);
      terminal.print("green zone");
      break;
    case toBlue:
      digitalWrite(toBlue, RelayOn);
      terminal.print("blue zone");
      break;
    case toRed:
      digitalWrite(toRed, RelayOn);
      terminal.print("red zone");
      break;
    /*case toGrey://To Grey
      break;*/
    default:
      error(101);
  }

  digitalWrite(mainPump, RelayOn);
  terminal.println();
  blynkJobUpdate();
}
