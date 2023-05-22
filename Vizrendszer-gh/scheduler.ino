char strBuffer[500];

float mmForWeek = 0.0, mmForToday = 0.0;

int sumMinutesWateredOnDays() {
  int sum = 0;
  for (int i = 0; i < 6; i++) {
    sum += EEPROM.read(i + 50);
  }
  Blynk.virtualWrite(V66, sum);
  return sum < 0 ? 0 : sum;
}

#include <TextFinder.h>

byte mac[] = { 0x06, 0x39, 0xd4, 0x60, 0xb5, 0x96 };
EthernetClient client;

TextFinder finder(client);

bool updateWeatherValues() {
  terminal.println("Getting new weather data from server.");
  Serial.println("Getting new weather data from server.");
  terminal.flush();
  Blynk.disconnect();

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to initialize Ethernet!");
    error(1600);
    return End;
  }

  delay(1000);

  if (client.connect("irrigation.fodor.pro", 80)) {
    client.println("GET /lastweek.txt HTTP/1.1");
    client.println("Host: irrigation.fodor.pro");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Couldn't connect to host!");
    Blynk.connect();
    error(1610);
    return End;
  }

  unsigned long requestStarted = millis(), lastHeartbeat = millis();
  char buffer[50];

  while (true) {
    if (millis() - requestStarted > 8000) {
      Serial.println("Couldn't get data - timeout");
      Blynk.connect();
      error(1620);
      return End;
    }
    // Need to keep watchdog happy while we wait for data
    if (millis() - lastHeartbeat > 1000) {
      heartbeat(true);  // true: heartbeat without Blynk
      lastHeartbeat = millis();
    }

    if (client.connected()) {
      if (!finder.getString("{", "}", buffer, 50)) {
        Serial.println("No match found!");
        Blynk.connect();
        error(1630);
        return End;
      }
      Serial.print("mmForWeek = ");
      Serial.print(mmForWeek = atof(buffer));
    } else {
      client.stop();
      Serial.println(" - Connection closed.");
      break;
    }
  }

  if (client.connect("irrigation.fodor.pro", 80)) {
    client.println("GET /today.txt HTTP/1.1");
    client.println("Host: irrigation.fodor.pro");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Couldn't connect to host!");
    Blynk.connect();
    error(1610);
    return End;
  }

  requestStarted = millis();

  while (true) {
    if (millis() - requestStarted > 8000) {
      Serial.println("Couldn't get data - timeout");
      Blynk.connect();
      error(1620);
      return End;
    }
    // Need to keep watchdog happy while we wait for data
    if (millis() - lastHeartbeat > 1000) {
      heartbeat(true);  // true: heartbeat without Blynk
      lastHeartbeat = millis();
    }

    if (client.connected()) {
      if (!finder.getString("{", "}", buffer, 50)) {
        Serial.println("No match found!");
        Blynk.connect();
        error(1630);
        return End;
      }
      Serial.print("mmForToday = ");
      Serial.print(mmForToday = atof(buffer));
    } else {
      client.stop();
      Serial.println(" - Connection closed.");
      break;
    }
  }

  Serial.println("Weather data update successful, reconnecting to Blynk.\n");
  Blynk.connect();
  Blynk.run();

  terminal.println("Got new weather data:");
  terminal.print(mmForWeek);
  terminal.println(" mm-s of watering needed over the last week,");
  terminal.print(mmForToday);
  terminal.println(" mm-s of watering forecasted to be needed today.");
  terminal.flush();

  return Continue;
}

void scheduler() {
  time_t t = now();
  secondsToday = t % 86400;

  if (dayResetDoneForDay != day(t)) {
    Serial.print("Day Reset for ");
    Serial.println(dayResetDoneForDay);
    Serial.println(day(t));

    dayResetDoneForDay = day(t);

    sprintf(strBuffer, "\n\n-------\nMinutes watered today (%d.%d.%d): %d\n", year(t), month(t), day(t) - 1, wateringMinutesCompletedToday);
    terminal.print(strBuffer);

    // Roll days back by one. Save value for today.
    EEPROM.update(50 + 7, wateringMinutesCompletedToday);
    terminal.println("Last 6 day's watering minutes:");
    for (int i = 0; i < 6; i++) {
      int val = EEPROM.read(50 + i + 1);
      EEPROM.update(50 + i, val);
      terminal.println(val);
    }
    terminal.println("---");

    wateringMinutesCompletedToday = 0;

    doneToday = false;
  }

  if (!isPeriodicWateringEnabled) return;

  if ((dailyWateringAtSeconds < secondsToday) && !doneToday) {
    sprintf(strBuffer, "\nScheduler: Set time of day reached (%d:%d).", hour(t), minute(t));
    terminal.println(strBuffer);
    doneToday = true;
    Blynk.virtualWrite(V61, 255);

    int i = 0;
    while (updateWeatherValues()) {
      terminal.println("Failed to get weather data, retrying...");
      if (i > 5) {
        terminal.println("After five tries, couldn't get weather data!");
        return;
      }
      i++;
    }

    Serial.println("Calculating length...");

    int sumMinutesNeeded = (int)((mmForWeek + mmForToday) * (float)mmToMinuteFactor);
    Serial.println((mmForWeek + mmForToday) * (float)mmToMinuteFactor);
    Serial.println(sumMinutesNeeded);
    int calculatedMinutes = sumMinutesNeeded - sumMinutesWateredOnDays();
    Serial.println(calculatedMinutes);
    Serial.println(sumMinutesWateredOnDays());

    Blynk.virtualWrite(V72, calculatedMinutes);

    if (calculatedMinutes < 0) calculatedMinutes = 0;

    unsigned long calculatedDuration = (unsigned long)calculatedMinutes * 60;
    Serial.println(calculatedDuration);

    if (calculatedDuration > minimumStartableDuration) {
      beginWatering(calculatedDuration, Normal);
    }
  }
}
