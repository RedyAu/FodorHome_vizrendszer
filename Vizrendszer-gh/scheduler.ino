void scheduler() {
  time_t t = now();
  secondsToday = 0;
  secondsToday += (unsigned long)((unsigned long)hour(t) * (unsigned long)3600);
  secondsToday += (unsigned long)((unsigned long)minute(t) * (unsigned long)60);
  secondsToday += (unsigned long)second(t);
  //Yeah I really really do love myself some type casting

  if (!isPeriodicWateringEnabled) return;
  if ((dailyWateringAtSeconds < secondsToday) && !doneToday) { //If we're past the set starting time
    terminal.print("Scheduler: Set time of day reached: ");
    terminal.print(hour(t));
    terminal.print(":");
    terminal.println(minute(t));
    
    if (!isThisWeekdaySelected((int)weekday(t))) {
      terminal.print("But today's weekday, ");
      terminal.print(weekday(t));
      terminal.println(", is not selected for watering. Skipping.");
      doneToday = true;
      return;
    }
    
    if (!skipNextWatering) {
      terminal.print("Today's weekday, ");
      terminal.print(weekday(t));
      terminal.println(", is selected for watering! Beginning daily session.");
      
      beginWatering(setWateringDuration, Normal);
    } else {
      terminal.print("Today's weekday, ");
      terminal.print(weekday(t));
      terminal.println(", is selected for watering, but this session was set to be skipped. Will water next time.");
      
      skipNextWatering = false;
    }
    doneToday = true;
  }
  if ((hour() + minute()) == 0) {
    doneToday = false; //At midnight, reset (Because of this, the set start time can't be 0:00 or else it'll _wreak havoc_ or what.)
    char strBuffer[60] = {0};
    sprintf(strBuffer, "\n\n-------\nMinutes watered today (%d.%d.%d): %d\n", year(t), month(t), day(t), wateringMinutesCompletedToday);
    terminal.print(strBuffer);
    wateringMinutesCompletedToday = 0;
  }
}
