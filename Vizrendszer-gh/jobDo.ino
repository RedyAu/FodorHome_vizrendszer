void jobStop() {
  terminal.println("\njobDo: Stopping.\n");

  digitalWrite(mainPump, RelayOff);

  delay(100);

  digitalWriteGroup(fromValves, LEN(fromValves), RelayOff);

  delay(500);

  digitalWriteGroup(toValves, LEN(toValves), RelayOff);

  Blynk.virtualWrite(V60, 0); //Display 0 on watering progress bar after stopping
  Blynk.virtualWrite(V62, 0);

  blynkJobUpdate();
}

void blynkJobUpdate() {
  Blynk.virtualWrite(toTap, (digitalRead(toTap) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(10, (digitalRead(toPink) == RelayOn) ? 128 : 0);
  Blynk.virtualWrite(13, (digitalRead(toGreen) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(16, (digitalRead(toBlue) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(19, (digitalRead(toRed) == RelayOn) ? 255 : 0);
  Blynk.virtualWrite(22, (digitalRead(toGrey) == RelayOn) ? 128 : 0);

  Blynk.virtualWrite(mainPump, (digitalRead(mainPump) == RelayOn) ? 255 : 0);
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

  terminal.print(" -> ");

  byte to[] = {toTap, toPink, toGreen, toBlue, toRed, toGrey};
  digitalWriteGroup(to, LEN(to), RelayOff);
  switch (currentJob.to) {
    case toTap:
      digitalWrite(toTap, RelayOn);
      terminal.print("tap");
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
    case toGrey:
      digitalWrite(toGrey, RelayOn);
      terminal.print("grey zone");
      break;
    case AllValves: //30-36
      digitalWriteGroup(toValves, LEN(toValves), RelayOn);
      terminal.print("all valves");
      break;
    default:
      error(101);
  }
  if (currentJob.from != AllValves) {
    digitalWrite(mainPump, RelayOn);
  } else {
    digitalWrite(mainPump, RelayOff);
  }
  terminal.println();
  blynkJobUpdate();
}
