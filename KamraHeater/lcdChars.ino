byte temperature[] = {
  B01001,
  B01001,
  B10010,
  B10010,
  B01001,
  B01001,
  B10010,
  B10010
};

byte exclamationMark[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B00100,
  B00000,
  B00100
};

byte droplet[] = {
  B00100,
  B00100,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110,
  B00000
};

byte littleClock[] = {
  B00000,
  B01110,
  B10101,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000
};

byte statusButton[] = {
  B00100,
  B00100,
  B01110,
  B10101,
  B10101,
  B10001,
  B01110,
  B00000
};


void createChars() {
  lcd.createChar(0, temperature);
  lcd.createChar(1, exclamationMark);
  lcd.createChar(2, droplet);
  lcd.createChar(3, littleClock);
  lcd.createChar(4, statusButton);
}
