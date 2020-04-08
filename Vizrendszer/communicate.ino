//Communicate with everything! Ok, well, with the Beltéri for now, but still
String received;

int holdFromS;
int holdToS;

void serialRead() {

  while (Serial.available()) { //Read Serial to a String
    char c = Serial.read();

    if (c == ';') {
      if (debug) {
        Serial.print("\n\nRaw command was: ");
        Serial.println(received);
      }
      parseCommand(received); //If the command terminates, parse it.
      received = "";
    }
    else {
      received += c;
    }
  }
}

void parseCommand(String rawCommand) {
  String command;
  int argument;

  command = rawCommand.substring(0, rawCommand.indexOf(" "));
  argument = rawCommand.substring(rawCommand.indexOf(" ") + 1).toInt();

  if (debug) {
    Serial.print("\nGot command: ");
    Serial.println(command);
    Serial.print("With argument: ");
    Serial.println(argument);
    Serial.print("\n\n");
  }

  /*
    else if (command == "") {

    }
  */
  if (command == "begin") {
    begun = 1;
    Serial.println("begun 0;");
  }

  if (begun) {

    if (command == "testGarage") {
      Serial.print("testReply ");
      Serial.print(argument);
      Serial.print(";");
    }

    else if (command == "tapFlow") {
      if (argument == 0) tapFlow = false;
      else tapFlow = true;
    }

    else if (command == "allStop") {
      allStop(true);
    }

    else if (command == "dumping") {
      if (argument == 0) dumping = false;
      else dumping = true;
    }

    else if (command == "cooling") {
      if (argument == 0) cooling = false;
      else cooling = true;

      EEPROM.put(10, cooling);
    }

    else if (command == "veryCool") {
      if (argument == 0) veryCool = false;
      else veryCool = true;
    }

    else if (command == "fromS") {
      holdFromS = argument;
    }

    else if (command == "toS") {
      holdToS = argument;
    }

    else if (command == "start") {
      allStop(true);
      waterStart(holdFromS, holdToS);
    }

    else if (command == "bufferTarget") {
      bufferTarget = argument;
      bufferTarget /= 100;
      EEPROM.put(11, bufferTarget);
    }

    else if (command == "error") {
      error(false, argument);
    }

    else if (command == "dumpToTap") {
      if (argument == 0) dumpToTap = false;
      else dumpToTap = true;
      EEPROM.put(12, dumpToTap);
    }

    else if (command == "bufferTargetNow") {
      Serial.print("\nCurrent buffer target is: ");
      Serial.println(bufferTarget);
    }

    else {
      if (debug) {
        Serial.print("\n\nGot wrong command!!\n\n");
      }
    }
  }
}
