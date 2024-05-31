void startMessage() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Powered By");
  lcd.setCursor(2, 1);
  lcd.print("KAICHO GROUP");
  delay(3000);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("-- KAWACH --");
  lcd.setCursor(2, 1);
  lcd.print("Pad Vending");
  delay(2000);
  homePage();
}
void homePage() {
  if (changeDone == true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    if (mType == 0)
      lcd.print("SCAN CARD HERE");
    else if (mType == 1)
      lcd.print("KAWACH VENDING");
    lcd.setCursor(0, 1);
    lcd.print("N0 OF STOCKS:");
    lcd.print(getStock());
    changeDone = false;
  }
}

void topMenu() {
  switch (topMenuPosition) {
    case 0:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write((byte)0);
      lcd.setCursor(3, 0);
      lcd.print("Pads In Rack");
      lcd.setCursor(3, 1);
      lcd.print("Fill All Rack");
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Pads In Rack");
      lcd.setCursor(0, 1);
      lcd.write((byte)0);
      lcd.setCursor(3, 1);
      lcd.print("Fill All Rack");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Fill All Rack");
      lcd.setCursor(0, 1);
      lcd.write((byte)0);
      lcd.setCursor(3, 1);
      lcd.print("Total Racks");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Total Racks");
      lcd.setCursor(0, 1);
      lcd.write((byte)0);
      lcd.setCursor(3, 1);
      lcd.print("Motor Time");
      break;
    case 4:
      if (mType != 0) {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Motor Time");
        lcd.setCursor(0, 1);
        lcd.write((byte)0);
        lcd.setCursor(3, 1);
        lcd.print("Machine Type");
      } else {
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Motor Time");
        lcd.setCursor(0, 1);
        lcd.write((byte)0);
        lcd.setCursor(3, 1);
        lcd.print("Batch Write");
      }
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Batch Write");
      lcd.setCursor(0, 1);
      lcd.write((byte)0);
      lcd.setCursor(3, 1);
      lcd.print("Machine Type");
      break;
  }
}

void menu() {
  switch (state) {
    case 0:
      homePage();
      topMenuPosition = 0;
      status = 'n';
      break;
    case 1:
      if (makeChange)
        topMenu();
      break;
  }
}

void batchWrite() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pad Amount");
  lcd.setCursor(0, 1);
  // enter number
  lcd.print(padAmount);
}
void machineType() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Machine Type");
  lcd.setCursor(0, 1);
  // enter number
  if (mType == 0) {
    lcd.print("RFID");
  } else if (mType == 1) {
    lcd.print("Coin / Button");
  }
}

void manageRack(byte num) {
  String message = "Rack" + String(num + 1) + " Quantity";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  // enter number
  lcd.print(rack[num].getQuantity());
}
void _maxRack() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Total Rack");
  lcd.setCursor(0, 1);
  lcd.print(maxRack);
}

void fillMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter capacity");
  lcd.setCursor(0, 1);
  lcd.print("of one Rack:");
  lcd.print(maxRackCapacity);
  // write to eeprom
  writeToEPPROM('f');
}
void fillingAllRack() {
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fill All");
    lcd.setCursor(0, 1);
    lcd.print("Rack");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press Okay");
    while (digitalRead(okButton))
      ;
    fillAll(maxRackCapacity);
    writeToEPPROM('f');    // write to eeprom
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Success..");
    success(500);
  }
}