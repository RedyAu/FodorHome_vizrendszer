/* parameters: If lastUpdate is more than 10s off, move start point and recalculate.
   if lastUpdate is more than 12h off, drop session
*/

/*const wateringZone zones[] = {
  {toPink, 10},
  {toGreen, 16},
  {toBlue, 28}//,
  //{toRed, 12}
  };*/

const unsigned long recalculateInactiveFor = 10 * 1000; //10s
const unsigned long killInactiveFor = 43200000; //43'200'000ms -> 12h

bool canMoveStart = false;

void beginWatering(unsigned long duration, bool purpose) {  //calculate one unit time from duration and set weights - then continue
  terminal.print("\n\nWatering: Starting new watering session for ");
  terminal.print(duration);
  terminal.print(" milliseconds.\nPurpose: ");
  terminal.println(purpose ? "Emptying the watering tank.\n" : "Watering for set duration.\n");
  
  currentSession = emptySession;
  currentSession.duration = duration;
  currentSession.purpose = purpose;
  currentSession.lastAlive = millis();
  updateZones(); //get new zones values from Blynk
  static wateringSession previousSession;

  if ((previousSession.duration == currentSession.duration) && (previousSession.purpose == currentSession.purpose)) water(); //if same session, do the section switch

  previousSession = currentSession;
  currentSession.startTime = millis();
/*
  sumWeights = 0;

  for (int i; i < LEN(zones); i++) {
    if (zones[i].isActive) {
      sumWeights += zones[i].weight;
    }
  }
*/
  Serial.print("waterStart sumWeights: "); //TODO removeme
  Serial.println(sumWeights);
  watering = true;
  wateringFinished = false;
  canMoveStart = currentSession.purpose == Cooling;
}

bool water() {
  //if haven't beginWater yet since last done, return. (?)
  //if lastUpdate old, move startTime
  //start current section based on startTime (rollover!! difference)
  //if finished reached, running false
  if (wateringFinished || !watering) return Continue;
  if (currentSession.purpose == Cooling && levelOf(Watering) == 0) {
    if (!cooling) currentSession.purpose = Normal; //If cooling is turned off, finish watering session.
    //If reason for watering was to empty watering tank, pause watering until full again.
    currentJob = waterJob{StopNext};
    return Continue;
  }

  unsigned long deadSince = millis() - currentSession.lastAlive;
  if (deadSince > recalculateInactiveFor) {
    if (canMoveStart) {
      //recalculate start so that after the break in watering, we continue where we left off.
      //take difference between start and lastAlive.
      //put start that amount of time before now.
      terminal.println("Watering: SHIFTING - Current session did not update for a bit, compensating for time spent without watering.");
      currentSession.startTime = millis() - (currentSession.lastAlive - currentSession.startTime);
    }
  }

  //Update elapsed time and lastAlive
  currentSession.lastAlive = millis();
  canMoveStart = true;
  currentSession.elapsedTime = millis() - currentSession.startTime;

  //Get current unit, finish watering if all time elapsed (type casting issues make this look so complicated)
  long double ratio = (long double)currentSession.elapsedTime / (long double)currentSession.duration;
  long double tempUnit = (long double)sumWeights * ratio;
  currentSession.currentUnit = (int)tempUnit;
  if (currentSession.currentUnit >= sumWeights) {
    currentSession = emptySession;
    wateringFinished = true;
    watering = false;
    canMoveStart = false;
    currentJob = {StopNext};
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

  int j = 0;//for loop didn't want to work :/
  while (j < LEN(zones)) {
    if (zones[j].isActive) {
      tempCurrentUnit -= zones[j].weight;
      if (tempCurrentUnit < 0) {
        currentJob = {NoStopNext, ((levelOf(Watering) > 0) ? fromWatering : fromWell), zones[j].id};
        return End;
        break;
      }
    }
    j++;
  }
}
