void error(bool local, int type) {
  currentError = type;

  Serial.print("error ");
  Serial.print(type);
  Serial.print(";");

  if (currentError > 999) return;

  while (error != 0) {


    delay(5000);

    serialRead();
  }

}
