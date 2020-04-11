/*Pin and variable names for reference:
  const int fromWell;
  const int fromGarage;
  const int fromBuffer;
  const int fromWatering;

  const int toDump;
  const int toBuffer;
  const int toWatering;
  const int toTap;

  const int mainPump;

  bool isFilling;
  bool isEmptying;

  bool isWaterWatering;
  bool isWellWatering;

  bool isBufferEmptying;
  bool isWaterEmptying;
*/

void allStop(bool userInit) {
  if (debug) Serial.println("debug allStop;");

  //stop everything, with delay
  if (userInit) {
    isFilling = false;
    isEmptying = false;

    isWaterWatering = false;
    isWellWatering = false;

    isBufferEmptying = false;
    isWaterEmptying = false;

    cooling = false;
    dumping = false;
    tapFlow = false;
  }

  Serial.print(userInit ? "off 1;" : "off 0;");

  digitalWrite(mainPump, relayOFF);
  digitalWrite(toDump, relayON); //To releive excess pressure

  delay(100);

  digitalWrite(fromWell, relayOFF);
  digitalWrite(fromGarage, relayOFF);
  digitalWrite(fromBuffer, relayOFF);
  digitalWrite(fromWatering, relayOFF);

  delay(500);

  digitalWrite(toBuffer, relayOFF);
  digitalWrite(toWatering, relayOFF);
  digitalWrite(toDump, relayOFF);
  digitalWrite(toTap, relayOFF);
}
