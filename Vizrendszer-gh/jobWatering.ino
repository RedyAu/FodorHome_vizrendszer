/* parameters: If lastUpdate is more than 10s off, move start point and recalculate.
   if lastUpdate is more than 12h off, drop session
*/

/*const wateringZone zones[] = {
  {toPink, 10},
  {toGreen, 16},
  {toBlue, 28}//,
  //{toRed, 12}
  };*/

const unsigned long recalculateInactiveFor = 10;  //10s

bool canMoveStart = false;

bool restoreWateringSession() {
  return; // TODO implement

  bool wateringFinishedLast = EEPROM.read(13);
  if (wateringFinishedLast) {
    terminal.println("No abandoned watering session found.");
    return false;
  }

  terminal.println("Watering session was abandoned, restoring...");

  bool purposeLast = EEPROM.read(14);
  unsigned long startTimeLast;
  EEPROM.get(15, startTimeLast);
  unsigned long lastAliveLast;
  EEPROM.get(19, lastAliveLast);
  unsigned long durationLast;
  EEPROM.get(23, durationLast);

  currentSession = emptySession;
  currentSession.duration = durationLast;
  currentSession.purpose = purposeLast;
  currentSession.lastAlive = lastAliveLast;
  currentSession.startTime = startTimeLast;

  updateZones();  // get new zones values from Blynk
  // make a copy of zones for use
  for (int i = 0; i < 4; i++) {
    inProgressZones[i] = zones[i];
  }

  watering = true;
  wateringFinished = false;
  canMoveStart = currentSession.purpose == Cooling;
  
  return true;
}

void beginWatering(unsigned long duration, bool purpose) {  //calculate one unit time from duration and set weights - then continue
  terminal.print("\n\nWatering: Starting new watering session for ");
  terminal.print(duration);
  terminal.print(" seconds.\nPurpose: ");
  terminal.println(purpose ? "Emptying the watering tank.\n" : "Watering for set duration.\n");

  currentSession = emptySession;
  currentSession.duration = duration;
  currentSession.purpose = purpose;

  updateZones();  // get new zones values from Blynk
  // make a copy of zones for use
  for (int i = 0; i < 4; i++) {
    inProgressZones[i] = zones[i];
  }

  pushWateringTimes();

  static wateringSession previousSession;

  if ((previousSession.duration == currentSession.duration) && (previousSession.purpose == currentSession.purpose)) water();  //if same session, do the section switch

  previousSession = currentSession;
  currentSession.startTime = now();
  currentSession.lastAlive = now();

  Serial.println(sumWeights);
  watering = true;
  wateringFinished = false;
  canMoveStart = currentSession.purpose == Cooling;

  EEPROM.update(13, false);
  EEPROM.update(14, currentSession.purpose);
  EEPROM.put(15, &currentSession.startTime);
  EEPROM.put(19, &currentSession.lastAlive);
  EEPROM.put(23, &currentSession.duration);
}

bool water() {
  //if haven't beginWater yet since last done, return. (?)
  //if lastUpdate old, move startTime
  //start current section based on startTime (rollover!! difference)
  //if finished reached, running false
  if (wateringFinished || !watering) return Continue;
  if (currentSession.purpose == Cooling && levelOf(Watering) == 0) {
    if (!cooling) currentSession.purpose = Normal;  //If cooling is turned off, finish watering session.
    //If reason for watering was to empty watering tank, pause watering until full again.
    currentJob = waterJob{ StopNext };
    return Continue;
  }

  unsigned long deadSince = now() - currentSession.lastAlive;
  if (deadSince > recalculateInactiveFor) {
    if (canMoveStart) {
      //recalculate start so that after the break in watering, we continue where we left off.
      //take difference between start and lastAlive.
      //put start that amount of time before now.
      terminal.println("Watering: SHIFTING - Current session did not update for a bit, compensating for time spent without watering.");
      currentSession.startTime = now() - (currentSession.lastAlive - currentSession.startTime);
      EEPROM.put(15, &currentSession.startTime);
    }
  }

  //Update elapsed time and lastAlive
  currentSession.lastAlive = now();
  canMoveStart = true;

  static unsigned long lastEepromSave = now();
  if (now() - lastEepromSave > 10) {
    EEPROM.put(19, &currentSession.lastAlive);
    lastEepromSave = now();
  }
  
  currentSession.elapsedTime = now() - currentSession.startTime;

  //Get current unit, finish watering if all time elapsed (type casting issues make this look so complicated)
  long double ratio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
  long double tempUnit = (long double)sumWeights * ratio;
  currentSession.currentUnit = (int)tempUnit;
  if (currentSession.currentUnit >= sumWeights) {
    terminal.println("\nWatering completed.");
    int minutesWatered = (int)(currentSession.duration / 60);
    terminal.println(currentSession.duration);
    terminal.println(minutesWatered);
    wateringMinutesCompletedToday += minutesWatered;

    currentSession = emptySession;
    wateringFinished = true;
    EEPROM.update(13, true);
    watering = false;
    canMoveStart = false;
    currentJob = { StopNext };
    return Continue;
  }

  //Switch watering zones based on current unit
  /*
     Make copy of currentSecion.
     Loop trough all sections:
      Say we should be during second zone:
      Loop subtracts weight of first zone and sees that it didn't go in negatives. Thus, continues loop;
      Loop subtracts weight of second zone as well, and sees that it's negative. Sets currentJob to watering that zone.
  */

  int tempCurrentUnit = currentSession.currentUnit;

  int j = 0;
  while (j < LEN(inProgressZones)) {
    if (inProgressZones[j].isActive) {
      tempCurrentUnit -= inProgressZones[j].weight;
      if (tempCurrentUnit < 0) {
        currentJob = { NoStopNext, ((levelOf(Watering) > 0) ? fromWatering : fromWell), inProgressZones[j].id };
        return End;
        break;
      }
    }
    j++;
  }
}
