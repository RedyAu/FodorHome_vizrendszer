void continuityCheck() {
  //if (debug) Serial.println("debug contiCheck;");

  if (digitalRead(fromGarage) && (digitalRead(fromBuffer) || digitalRead(fromWatering))) bool fromGarage = true;
  else bool fromGarage = false;

  if (
    digitalRead(mainPump) //When the main pump is running, at least one input and one output valve should be open.
    &&
    (((digitalRead(fromWell) ? 1 : 0) + (fromGarage ? 1 : 0)) == 0)
    ||
    (((digitalRead(toBuffer) ? 1 : 0) + (digitalRead(toWatering) ? 1 : 0) + (digitalRead(toDump) ? 1 : 0) + (digitalRead(toTap) ? 1 : 0) == 0))
  ) {

    if (debug) Serial.println("debug contiCheckBAD;");

    allStop(false);
    error(true, 120);
  }
}
