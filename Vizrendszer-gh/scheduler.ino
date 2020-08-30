void scheduler() {
  time_t t = now();
  secondsToday = 0;
  secondsToday += (unsigned long)((unsigned long)hour(t) * (unsigned long)3600);
  secondsToday += (unsigned long)((unsigned long)minute(t) * (unsigned long)60);
  secondsToday += (unsigned long)second(t);
  //Yeah I really really do love myself some type casting
  
  if (!isPeriodicWateringEnabled) return;
  if (!isThisWeekdaySelected((int)weekday(t))) return;
  if ((dailyWateringAtSeconds < secondsToday) && !doneToday) { //If we're past the set starting time
    if (!skipNextWatering) {
      beginWatering(setWateringDuration, Normal);
      terminal.println("Beginning daily watering...");
    } else {
      skipNextWatering = false;
      terminal.println("Skipping today's watering session...");
    }
    doneToday = true;
  }
  if ((hour() + minute()) == 0) doneToday = false; //At midnight, reset (Because of this, the set start time can't be 0:00 or else it'll _wreak havoc_ or what.)
}
