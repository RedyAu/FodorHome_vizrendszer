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
      //allStop(true); todo userinit stop?
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

    else if (command == "fullEmpty") {
      if (argument == 0) fullEmpty = false;
      else fullEmpty = true;
    }

    else if (command == "fromS") {
      holdFromS = argument;
    }

    else if (command == "toS") {
      holdToS = argument;
    }

    else if (command == "start") {
      currentJob = waterJob{NoStopNext, holdFromS, holdToS};
    }

    else if (command == "bufferTreshold") {
      bufferTreshold = argument;
      bufferTreshold /= 100;
      EEPROM.put(11, bufferTreshold);
    }

    else if (command == "error") {
      error(argument);
    }

    else if (command == "bufferTresholdNow") {
      Serial.print("\nCurrent buffer target is: ");
      Serial.println(bufferTreshold);
    }

    else {
      if (debug) {
        Serial.print("\n\nGot wrong command!!\n\n");
      }
    }
  }
}
