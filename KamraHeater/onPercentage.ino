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
