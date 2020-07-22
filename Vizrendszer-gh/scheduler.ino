void scheduler() {
  time_t t = now();
  secondsToday += hour(t) * 3600;
  secondsToday += minute(t) * 60;
  secondsToday += second(t);

  if ((dailyWateringAtSeconds > secondsToday) && !doneToday) { //If we're past the set starting time
    if (!skipNextWatering) {
      beginWatering(setWateringDuration * 60, Normal);
      terminal.println("Beginning daily watering...");
    } else {
      skipNextWatering = false;
      terminal.println("Skipping today's watering session...");
    }
    doneToday = true;
  }

  if ((hour() + minute()) == 0) doneToday = false; //At midnight, reset (Because of this, the set start time can't be 0:00 or else it'll _wreak havoc_ or what.)
}
