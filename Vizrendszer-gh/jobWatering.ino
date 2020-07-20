/* parameters: If lastUpdate is more than 10s off, move start point and recalculate.
 * if lastUpdate is more than 12h off, drop session
 */

struct wateringZone {
  int id;
  int weight;  
};
const wateringZone zones[] = {
  {toPink, 10},
  {toGreen, 16},
  {toBlue, 28}//, //todo
  //{toRed, 12}
};

const unsigned long recalculateInactiveFor = 10 * 1000; //10s
const unsigned long killInactiveFor = 43200000; //43'200'000ms -> 12h

struct wateringSession {
  unsigned long duration;
  unsigned long startTime;
  unsigned long lastAlive;
  unsigned long elapsedTime;
  int currentUnit;
  bool purpose;
};
wateringSession currentSession;
const wateringSession emptySession;

int sumWeights;

void beginWatering(unsigned long duration, bool purpose) {  //calculate one unit time from duration and set weights - then continue
  currentSession = emptySession;
  currentSession.duration = duration;
  currentSession.purpose = purpose;
  static wateringSession previousSession;
  if ((previousSession.duration == currentSession.duration) && (previousSession.purpose == currentSession.purpose)) water(); //if same session, do the section switch
  previousSession = currentSession;

  if (wateringFinished) { //If we're beginning a new session
    currentSession.startTime = millis();
    for (int i; i < LEN(zones); i++) {
      sumWeights += zones[i].weight;
    }
  }
  watering = true;
  wateringFinished = false;
  water();
}

bool water() {
  //if haven't beginWater yet since last done, return. (?)
  //if lastUpdate old, move startTime
  //start current section based on startTime (rollover!! difference)
  //if finished reached, running false
  if (wateringFinished) return Continue;
  unsigned long deadSince = millis() - currentSession.lastAlive;
  if (deadSince > recalculateInactiveFor) {
    if (deadSince > killInactiveFor) {
      currentSession = emptySession;
      wateringFinished = true;
      return Continue;
    } else {
      //recalculate start so that after the break in watering, we continue where we left off.
      //take difference between start and lastAlive.
      //put start that amount of time before now.
      currentSession.startTime = millis() - (currentSession.lastAlive - currentSession.startTime);
    }
  }
  //Update elapsed time and lastAlive
  currentSession.lastAlive = millis();
  currentSession.elapsedTime = millis() - currentSession.startTime;
  
  //Get current unit, finish watering if all time elapsed
  currentSession.currentUnit = (int)(sumWeights * (currentSession.elapsedTime / currentSession.duration));
  if (currentSession.currentUnit > sumWeights) {
    currentSession = emptySession;
    wateringFinished = true;
    return Continue;
  }

  //Switch watering zones based on current unit
  /*
   * Make copy of currentSecion.
   * Loop trough all sections:
   *  Say we should be during second zone:
   *  Loop subtracts weight of first zone and sees that it didn't go in negatives. Thus, continues loop;
   *  Loop subtracts weight of second zone as well, and sees that it's negative. Sets currentJob to watering that zone.
  */
   int tempCurrentUnit = currentSession.currentUnit;
   for (int i; i < LEN(zones); i++) {
      tempCurrentUnit - zones[i].weight;
      if (tempCurrentUnit < 0) {
        currentJob = {NoStopNext, ((levelOf(Watering) > 0) ? fromWatering : fromWell), zones[i].id};
        break;
      }
   }
}
