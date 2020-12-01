/*
 * PSEUDO
 * Have an index
 * Have a 60 long array
 * When new value comes in, write it in the next array cell
 * If we're on the end of the array, jump back to the beginning
 */

bool lastHour[60];
int lastHourIndex = 0;

void updateOnPercentage() {  
  Serial.println("Update");
  lastHour[lastHourIndex] = !digitalRead(oRelay);
  lastHourIndex++;
  if (lastHourIndex >= 60) {
    lastHourIndex = 0;
  }
}

/*
 * PSEUDO
 * Loop trough the on-array, get how much of values are "true"
 * Divide by 60, multiply by 100
 */

int getOnPercentage() {
  float sumLastHour = 0;
  
  for (int i = 0; i <= 59; i++) {
    sumLastHour += lastHour[i];  
  }
  float percentage = (sumLastHour / 60) * 100;
  return percentage;
}

/*
 * PSEUDO
 * Get millisecond difference
 * If never or less than 100 ms, show --
 * If less than a minute, show seconds
 * If less than an hour, show minutes
 * Else show hours, at max two digits.
 */

void updateLastOnDisplay() {
  unsigned long difference = millis() - lastOn;
  if ((lastOn == 0) || (difference < 1000)) {
    lastOnUnit = NoValue;
  } else if (difference < 60000) {
    lastOnUnit = Seconds;
    lastOnValue = difference / 1000;
  } else if (difference < 3600000) {
    lastOnUnit = Minutes;
    lastOnValue = difference / 60000;
  } else if (difference < 3600000000) { //Less than 1000 hours (which is the display limit)
    lastOnUnit = Hours;
    lastOnValue = difference / 3600000;
  } else {
    lastOnUnit = MaxValue;
  }
}
