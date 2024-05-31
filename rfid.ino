void manageRFID()
{
  if (mfrc522.PICC_IsNewCardPresent())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card Detected!");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait.....");
    if (mfrc522.PICC_ReadCardSerial())
    {
      if (readCard())
      {
        success(500);
        if (readByte[0] == FILLCARD)
        {
          Serial.println("Card Detected as FILLCARD");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Filling All");
          lcd.setCursor(0, 1);
          lcd.print("Rack Via CARD");
          delay(2000);
          fillAll(maxRackCapacity);
          writeToEPPROM('f'); // write to eeprom
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Success..");
          success(500);
        }
        else if (readByte[0] == RECHCARD)
        {
          Serial.println("Card Detected as RECHCARD");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Batch Write");
          lcd.setCursor(0, 1);
          lcd.print("Mode Via CARD");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Scan Card!");
          halt();
          rechCardDetected = true;
          while (rechCardDetected)
          {
            // Serial.println("inside ");
            if (mfrc522.PICC_IsNewCardPresent())
            {
              if (mfrc522.PICC_ReadCardSerial())
              {
                // writeByte[0] = 107;
                if (readCard())
                {
                  if (readByte[0] == FILLCARD || readByte[0] == RECHCARD || readByte[0] == PADCARD)
                  {
                    Serial.println("This is Menu card");
                    if (readByte[0] == RECHCARD)
                    {
                      rechCardDetected = false;
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("Exiting..");
                      delay(1000);
                    }
                    else
                    {
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("Use RECH CARD");
                      lcd.setCursor(0, 1);
                      lcd.print("to exit..");
                      delay(1000);
                    }
                  }
                  else
                  {
                    writeByte[15] = padAmount;
                    if (writeCard(writeByte))
                    {
                      lcd.setCursor(0, 1);
                      lcd.print("Done");
                      success(500);
                    }
                  }
                }
              }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Scan Card!");
              halt();
            }
          }
        }
        else if (readByte[0] == PADCARD)
        {
          Serial.println("Card Detected as PADCARD");
          padAmount = int(readByte[15]);
          writeToEPPROM('b');
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set Pad Amount");
          lcd.setCursor(0, 1);
          lcd.print("via CARD");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Pad Amount:");
          lcd.setCursor(0, 1);
          lcd.print(padAmount);
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Success..");
          success(500);
        }
        else if (readByte[15] != 0) // && readByte[0] == 107) // verify the card and available quantity on card
        {
          dumpToWriteVar(readByte, 16);
          if (!writeCard(writeByte))
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Write Failed");
            lcd.setCursor(0, 1);
            lcd.print("Try Again");
            warning();
          }
          else
          {
            byte lastData = readByte[15];
            readCard();
            if (getStock() != 0) // check the stock before proceeding
            {
              if (lastData != readByte[15]) // check the stock before proceeding
              {
                delay(500);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Left for Card:");
                lcd.setCursor(0, 1);
                lcd.print(writeByte[15]);
                delay(3000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Receive Pad");
                lcd.setCursor(0, 1);
                lcd.print("Thank you!");
                runMotor();
                success(800);
                delay(1000);
              }
              else // print if card scan time is fast
              {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Sorry");
                lcd.setCursor(0, 1);
                lcd.print("Hold card for 1Sec");
                warning();
              }
            }

            else // print if no stock remaining
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Sorry");
              lcd.setCursor(0, 1);
              lcd.print("No Stocks");
              warning();
            }
          }
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Opps!");
          lcd.setCursor(0, 1);
          lcd.print("Zero Balance");
          warning();
        }
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Error Reading");
        lcd.setCursor(0, 1);
        lcd.print("Try Again");
        warning();
      }
    }
    halt();
    status = 'n';
    changeDone = true;
  }
}

bool readCard()
{
  byte buffersize = 18;
  if (auth_A())
  {
    rfidstatus = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, readByte, &buffersize);
    if (rfidstatus != MFRC522::STATUS_OK)
    {
      return false;
    }
  }
  return true;
}
bool writeCard(byte writeByte[])
{
  if (auth_B())
  {
    rfidstatus = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, writeByte, 16);
    if (rfidstatus != MFRC522::STATUS_OK)
    {
      return false;
    }
  }
  return true;
}
void dumpToWriteVar(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i]);
    writeByte[i] = buffer[i];
  }
  writeByte[15]--;
}
void halt()
{
  mfrc522.PICC_HaltA();      // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
}
bool auth_A()
{
  rfidstatus = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, authAddr, &key, &(mfrc522.uid));
  if (rfidstatus != MFRC522::STATUS_OK)
  {
    return false;
  }
  return true;
}
bool auth_B()
{
  rfidstatus = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, authAddr, &key, &(mfrc522.uid));
  if (rfidstatus != MFRC522::STATUS_OK)
  {
    return false;
  }
  return true;
}
