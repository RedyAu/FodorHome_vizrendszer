/*
   From: Kút / Garázs(puffer / öntöző)
          0             1        2

   To: Dump / Csap / Garázs(puffer / öntöző)
         0      1             2         3
*/

bool waterStart(int from, int to) {
  if (mainPump) {
    allStop(false);
    delay(200);
  }
  switch (from) {
    case 0:
      digitalWrite(fromWell, relayON);
      Serial.print("fromS 0;");
      break;
    case 1:
      digitalWrite(fromGarage, relayON);
      digitalWrite(fromBuffer, relayON);
      Serial.print("fromS 1;");
      break;
    case 2:
      digitalWrite(fromGarage, relayON);
      digitalWrite(fromWatering, relayON);
      Serial.print("fromS 2;");
      break;
    default:
      error(true, 100);
  }
  switch (to) {
    case 0:
      digitalWrite(toDump, relayON);
      Serial.print("toS 0;");
      break;
    case 1:
      digitalWrite(toTap, relayON);
      Serial.print("toS 1;");
      break;
    case 2:
      digitalWrite(toBuffer, relayON);
      Serial.print("toS 2;");
      break;
    case 3:
      digitalWrite(toWatering, relayON);
      Serial.print("toS 3;");
      break;
    default:
      error(true, 101);
  }

  continuityCheck();
  delay(200);

  digitalWrite(mainPump, relayON);
  Serial.print("mainPump 1;");

  return true;
}
