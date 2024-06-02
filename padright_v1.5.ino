#include "padrack.h"
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#define RST_PIN 9 // Configurable, see typical pin layout above
#define SS_PIN 10 // Configurable, see typical pin layout above
#define FILLCARD  70  //card identifier as machine filling 
#define RECHCARD  82  //card identifier as recharge mode
#define PADCARD   80  //card identifier as set padAmount

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

MFRC522::MIFARE_Key key;
MFRC522::StatusCode rfidstatus;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// setup variable for the machine type
// rfid card system or pushbutton or coin based
// this variable needs to be setup while manufacturing process;
byte mType = 0; // 0 for rfid based and 1 for pushbutton or coin based;
const byte mTypeAddress = 25;

//// this checks if the system is booted for the first time
char firstBoot = 'y';
const byte firstBootAddress = 100;

// --------- eeprom address for menu variables ---------
const byte rackAddress[] = {1, 2, 3, 4, 5, 6};

byte maxRack = 2; // default rack quantity is two, it should be manipulated in manufacturing process through menu;
const byte maxRackAddress = 30;
const byte maxRackCapacityAddress = 20;
const byte motorTimeAddress = 10;
const byte maxPadAddress = 15;

// regarding rfid card
byte blockAddr = 2;     // this block is uded to set the value of padAmount in rfid card
byte readByte[18];  // for padAmount
byte writeByte[16]; // for padAmount
byte authAddr = 3;
byte byteSize = sizeof(readByte);

//----------- class objects ---------------------
padrack rack[6];

//-------------------- input and output varaibles --------------
const byte motor1 = 2;
const byte motor2 = 3;
const byte motor3 = 4;
const byte motor4 = 14;
const byte motor5 = 15;
const byte motor6 = 16;
const byte menuButton = 6;
const byte selectButton = 7;
const byte okButton = 8;
const byte buzzer = 5;
const byte interrupt = 17;
const byte motor[] = {motor1, motor2, motor3, motor4, motor5, motor6};

////----------- logic variables -------------
static uint32_t previous_time;
int count = 0;
bool change = false;
bool changeDone = true; // this is for homepage changes // toggle
int maxRackCapacity = 20;
int motorTimeVariable = 1500;
int topMenuPosition = 0;
int state = 0;
bool makeChange = false; /// this bool variable is for top menu toogle
char status = 'n';
int padAmount = 1;
bool isInterrupt = false;
bool rechCardDetected = false;

byte arrow[8] = {
  0b00000,
  0b11100,
  0b10010,
  0b01001,
  0b01001,+
  0b10010,
  0b11100,
  0b00000
};

void setup()
{
  Serial.begin(9600);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.createChar(0, arrow);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  pinSetup();
  attachInterrupt(digitalPinToInterrupt(interrupt), _interrupt, RISING);
  if (EEPROM.read(firstBootAddress) != firstBoot)
  {
    Serial.println("Frist Boot");
    writeToEPPROM('y');
    delay(100);
    EEPROM.write(firstBootAddress, firstBoot);
  }
  else
  {
    Serial.println("Welcome Back");
    // writeToEPPROM('n');
  }
  readFromEEPROM();
  // Serial.println(maxRackCapacity);
  // Serial.println(padAmount);
  // key for auth rfid
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  startMessage();
  // delay(2000);
  // menu();
  machineDetails();
  if (!digitalRead(menuButton))
  {
    makeChange = true;
    changeDone = true;
    state = 1;
    status = 'n';
    menu();
    while (!digitalRead(menuButton))
    {
      ;
    }
  }
}

void loop()
{
  menuManagement();
  if (mType == 0)
  {
    manageRFID(); // when machine is rfid type
  }
  else if (mType == 1)
  {
    if (isInterrupt)
      buttonMachine(); // when machine is pushbutton or coin based
  }

  // machineDetails();
}

void pinSetup()
{
  pinMode(menuButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(okButton, INPUT_PULLUP);
  // pinMode(interrupt, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  for (int i = 0; i < maxRack; i++)
  {
    pinMode(motor[i], OUTPUT);
    digitalWrite(motor[i], LOW);
  }
}
void menuManagement()
{
  previous_time = millis();
  static bool buzzerNotification = false;
  while (!digitalRead(menuButton))
  {
    if (!digitalRead(okButton) && digitalRead(selectButton) && mType == 0)
    {
      if (millis() - previous_time >= 5000)
      {
        if (!buzzerNotification)
        {
          success(300);
          success(300);
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Batch Write");
        lcd.setCursor(0, 1);
        lcd.print("Mode");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan Card!");

        while (digitalRead(okButton))
        {
          Serial.println("inside ");
          if (mfrc522.PICC_IsNewCardPresent())
          {
            if (mfrc522.PICC_ReadCardSerial())
            {
              if (readCard()) {
                if (readByte[0] == FILLCARD || readByte[0] == RECHCARD || readByte[0] == PADCARD) {
                  lcd.setCursor(0, 1);
                  lcd.print("Error, Menu Card");
                  delay(1000);
                } else {
                  writeByte[0] = 0;
                  writeByte[15] = padAmount;
                  if (writeCard(writeByte))
                  {
                    lcd.setCursor(0, 1);
                    lcd.print("Done");
                    success(500);
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
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Exiting...");
        delay(500);
        state = 0;
        status = 'n';
        makeChange = true;
        changeDone = true;
        buzzerNotification = !buzzerNotification;
        menu();
      }
    }
    else if (!digitalRead(selectButton) && digitalRead(okButton))
    {
      if (millis() - previous_time >= 5000)
      {
        success(300);
        success(300);
        /// filling all rack
        fillingAllRack();
        state = 0;
        status = 'n';
        makeChange = true;
        changeDone = true;
        menu();
      }
    }
    else
    {
      state = 0;
      status = 'v';
    }
  }

  if (status == 'n')
  {
    if (!digitalRead(selectButton))
    {
      delay(300);
      topMenuPosition++;
      if (topMenuPosition == 6)
        topMenuPosition = 0;
      if (mType != 0 && topMenuPosition >= 5)
        topMenuPosition = 0;
      makeChange = true;
      changeDone = true;
    }

    if (!digitalRead(okButton) && state == 1)
    {
      delay(300);
      if (state == 1 && topMenuPosition == 0)
        status = 'r'; // pads in  rack
      else if (state == 1 && topMenuPosition == 1)
        status = 'f'; // fill all
      else if (state == 1 && topMenuPosition == 2)
        status = 'M'; // max racks
      else if (state == 1 && topMenuPosition == 3)
        status = 'm'; // motor time
      else if (state == 1 && topMenuPosition == 4)
      {
        if (mType == 0)
          status = 'b'; // batch write
        else
          status = 't';
      }
      else if (state == 1 && topMenuPosition == 5)
        status = 't'; // machine type set
      // Serial.println(status);

      switch (status)
      {
        case 'f':
          fillMenu();
          while (digitalRead(okButton))
          {
            // delay(300);
            if (!digitalRead(selectButton))
            {
              delay(300);
              maxRackCapacity = maxRackCapacity + 1;
              if (maxRackCapacity > 60)
                maxRackCapacity = 1;
              fillMenu();
            }
          }
          fillAll(maxRackCapacity);
          break;
        case 'M': // for maximum racks setting
          _maxRack();
          while (digitalRead(okButton))
          {
            // delay(300);
            if (!digitalRead(selectButton))
            {
              delay(300);
              maxRack = maxRack + 1;
              if (maxRack > 6)
                maxRack = 1;
              _maxRack();
            }
          }
          break;
        case 'm':
          motorTime();
          while (digitalRead(okButton))
          {
            // delay(300);
            if (!digitalRead(selectButton))
            {
              delay(300);
              motorTimeVariable = motorTimeVariable + 100;
              if (motorTimeVariable > 5000)
                motorTimeVariable = 100;
              motorTime();
            }
          }
          break;
        case 'r':
          for (int i = 0; i < maxRack; i++)
          {
            manageRack(i);
            while (digitalRead(okButton))
            {
              // delay(300);
              if (!digitalRead(selectButton))
              {
                delay(300);
                rack[i].incQuantity();
                manageRack(i);
              }
            }
            while (!digitalRead(okButton))
              ;
          }
          break;
        case 'b':
          batchWrite();
          while (digitalRead(okButton))
          {
            if (!digitalRead(selectButton))
            {
              delay(300);
              padAmountInc();
              batchWrite();
            }
          }
          while (!digitalRead(okButton))
            ;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Scan Card!");
          while (digitalRead(okButton))
          {
            if (mfrc522.PICC_IsNewCardPresent())
            {
              if (mfrc522.PICC_ReadCardSerial())
              {
                //               writeByte[0] = 0;
                writeByte[15] = padAmount;
                if (writeCard(writeByte))
                {
                  lcd.setCursor(0, 1);
                  lcd.print("Done");
                  success(500);
                }
              }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Scan Card!");
              halt();
            }
          }
          // halt();
          while (!digitalRead(okButton))
            ;
          /////   batch code
          break;
        case 't':
          machineType();
          while (digitalRead(okButton))
          {
            if (!digitalRead(selectButton))
            {
              delay(300);
              if (mType == 0)
                mType = 1;
              else
                mType = 0;
              machineType();
            }
          }
          while (!digitalRead(okButton))
            ;
          break;
      }
      writeToEPPROM(status);
      success(800);
      save();
      changeDone = true;
    }
  }

  menu();
  makeChange = false;
}

void motorTime()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Motor Time (ms)");
  lcd.setCursor(0, 1);
  // enter number
  lcd.print(motorTimeVariable);
}
void save()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Saving...");
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
  status = 'n';
  state = 0;
}
void writeToEPPROM(char status)
{

  if (status == 'r' || status == 'f')
  {
    for (int i = 0; i < maxRack; i++)
    {
      EEPROM.write(rackAddress[i], rack[i].getQuantity());
    }
    EEPROM.write(maxRackCapacityAddress, maxRackCapacity);
  }
  else if (status == 'm')
  {
    writeIntIntoEEPROM(motorTimeAddress, motorTimeVariable);
  }
  else if (status == 'b')
  {
    EEPROM.write(maxPadAddress, padAmount);
  }
  else if (status == 't')
  {
    EEPROM.write(mTypeAddress, mType);
  }
  else if (status == 'M')
  {
    EEPROM.write(maxRackAddress, maxRack);
  }
  else if (status = 'y')
  {
    // write when first boot
    EEPROM.write(mTypeAddress, mType);
    EEPROM.write(maxRackAddress, maxRack);
    EEPROM.write(maxPadAddress, padAmount);
    EEPROM.write(maxRackCapacity, maxRackAddress);
    writeIntIntoEEPROM(motorTimeAddress, motorTimeVariable);
    for (int i = 0; i < maxRack; i++)
    {
      EEPROM.write(rackAddress[i], maxRackCapacity);
    }
  }
}
void readFromEEPROM()
{
  padAmount = EEPROM.read(maxPadAddress);
  mType = EEPROM.read(mTypeAddress);
  maxRack = EEPROM.read(maxRackAddress); /// upper three variable must be read first
  motorTimeVariable = readIntFromEEPROM(motorTimeAddress);
  if (motorTimeVariable == -1)
    motorTimeVariable = 0;
  for (int i = 0; i < maxRack; i++)
  {
    rack[i].setQuantity(EEPROM.read(rackAddress[i]));
  }
  maxRackCapacity = EEPROM.read(maxRackCapacityAddress);
  if (maxRackCapacity < 0)
    maxRackCapacity = 1;
  for (int i = 0; i < maxRack; i++)
  {
    rack[i].setMaxQuantity(maxRackCapacity);
  }
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void runMotor()
{
  Serial.println("motor running");
  if (getStock() > 0)
  {
    for (int i = 0; i < maxRack; i++)
    {
      if (rack[i].getQuantity() != 0)
      {
        // Serial.println("motor");
        digitalWrite(motor[i], HIGH);
        delay(motorTimeVariable);
        digitalWrite(motor[i], LOW);
        rack[i].decQuantity();
        break;
      }
    }
    writeToEPPROM('r');
  }
}
void fillAll(int num)
{
  for (int i = 0; i < maxRack; i++)
  {
    rack[i].setQuantity(num);
  }
}

int getStock()
{
  int totalStock = 0;
  for (int i = 0; i < maxRack; i++)
  {
    totalStock += rack[i].getQuantity();
  }
  return totalStock;
}
void success(int _time)
{
  digitalWrite(buzzer, HIGH);
  delay(_time);
  digitalWrite(buzzer, LOW);
}
void warning()
{
  digitalWrite(buzzer, HIGH);
  delay(400);
  digitalWrite(buzzer, LOW);
  delay(400);
  digitalWrite(buzzer, HIGH);
  delay(400);
  digitalWrite(buzzer, LOW);
  delay(400);
  digitalWrite(buzzer, HIGH);
  delay(400);
  digitalWrite(buzzer, LOW);
}
void padAmountInc()
{
  padAmount++;
  if (padAmount > 255)
  {
    padAmount = 1;
  }
}
void _interrupt()
{
  if (mType == 1)
  {
    isInterrupt = true;
    // Serial.println("interrupt");
  }
}
void machineDetails()
{
  String var = "\nMachine Type : " + String(mType);
  var += "\nMaximum Rack: " + String(maxRack);
  var += "\nRack Capacity: " + String(maxRackCapacity);
  var += "\nPad Amount: " + String(padAmount);
  for (int i = 0; i < maxRack; i++)
  {
    var += "\nRack " + String(i + 1) + " Quantity: " + String(rack[i].getQuantity());
  }

  Serial.println(var);
  delay(1000);
}
void buttonMachine()
{

  if (getStock() != 0) // check the stock before proceeding
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receive Pad");
    lcd.setCursor(0, 1);
    lcd.print("Thank you!");
    runMotor();
    success(800);
    delay(1000);
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
  state = 0;
  status = 'n';
  changeDone = true;
  isInterrupt = false;
  menu();
}
