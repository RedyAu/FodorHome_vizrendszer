void continuityCheck() {
  //if (debug) Serial.println("debug contiCheck;");

  if (digitalRead(fromGarage) && (digitalRead(fromBuffer) || digitalRead(fromWatering))) bool fromGarage = true;
  else bool fromGarage = false;

  if (
    (digitalRead(mainPump) == relayON //When the main pump is running, at least one input and one output valve should be open.
    &&
    ((digitalRead(fromWell) + fromGarage) == 0))
    ||
    (digitalRead(toBuffer) + digitalRead(toWatering) + digitalRead(toDump) + digitalRead(toTap) == 0))
  {
    if (debug) Serial.println("debug contiCheckBAD;");
    allStop(false);
    error(true, 120);
  }
}
