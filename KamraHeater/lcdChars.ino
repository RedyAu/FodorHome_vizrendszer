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

byte onIcon[] = {
  B11100,
  B10010,
  B11100,
  B10011,
  B11100,
  B00111,
  B00100,
  B00111
};

byte offIcon[] = {
  B10100,
  B10100,
  B11000,
  B10101,
  B10101,
  B00001,
  B00001,
  B00001
};

byte lastHourIcon[] = {
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000,
  B10001,
  B11111
};

byte lastOnIcon[] = {
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000,
  B10101,
  B00000
};


void createChars() {
  lcd.createChar(0, temperature);
  lcd.createChar(2, droplet);
  lcd.createChar(3, onIcon);
  lcd.createChar(4, offIcon);
  lcd.createChar(5, lastHourIcon);
  lcd.createChar(6, lastOnIcon);
}
