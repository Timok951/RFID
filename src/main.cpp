#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define EEPROMPORT1 1
#define EEPROMPORT2 11
#define EEPROMPORT3 12

MFRC522 mfrc522(SS_PIN, RST_PIN);

class Person
{
public:
  char Name[10];
  byte sum;
  byte mfrc[4];
};

Person me;
bool waitingForName = false;
bool waitingForNameOutput = false;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
  }
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("MFRC522 initialized");

  /*
    for (int i = 0; i < 20; i++)
  {
    EEPROM.write(i, 0);
  }
  Serial.println("EEPROM cleared");

  */

  me.sum = EEPROM.read(EEPROMPORT2);
  for (byte i = 0; i < 4; i++)
  {
    me.mfrc[i] = EEPROM.read(EEPROMPORT3 + i);
  }
  for (int i = 0; i < 10; i++)
  {
    me.Name[i] = EEPROM.read(EEPROMPORT1 + i);
    if (me.Name[i] == 0)
      break;
  }

  Serial.println("Land your card to reader");
}

void loop()
{
  if (waitingForName && Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0)
    {
      strncpy(me.Name, input.c_str(), 9);
      me.Name[9] = '\0';
      for (int i = 0; i < 10; i++)
      {
        EEPROM.write(EEPROMPORT1 + i, me.Name[i]);
      }
      Serial.print("Name saved: ");
      Serial.println(me.Name);
      waitingForName = false;
    }
    else
    {
      Serial.println("Empty input, please enter a name");
    }
  }

  if (waitingForNameOutput && Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0)
    {
      Serial.print("Name: ");
      Serial.println(me.Name);
      Serial.print("Sum: ");
      Serial.println(me.sum);
      waitingForNameOutput = false;
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    delay(100);
    return;
  }

  Serial.print("Scanned UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (me.mfrc[0] == 0 && me.mfrc[1] == 0 && me.mfrc[2] == 0 && me.mfrc[3] == 0)
  {
    for (byte i = 0; i < 4; i++)
    {
      me.mfrc[i] = mfrc522.uid.uidByte[i];
      EEPROM.write(EEPROMPORT3 + i, me.mfrc[i]);
    }
    Serial.println("Card was registered");
    Serial.println("Please input your name and press Enter");
    waitingForName = true;
  }
  else
  {
    bool uidMatches = true;
    for (byte i = 0; i < 4; i++)
    {
      if (mfrc522.uid.uidByte[i] != me.mfrc[i])
      {
        uidMatches = false;
        break;
      }
    }

    if (uidMatches)
    {
      if (me.sum <= 155)
      {
        me.sum += 100;
        EEPROM.write(EEPROMPORT2, me.sum);
      }
      else
      {
        Serial.println("Sum cannot exceed 255");
      }
      Serial.println("Press Enter to show card details");
      waitingForNameOutput = true;
    }
    else
    {
      Serial.println("Card does not match");
    }
  }

  mfrc522.PICC_HaltA();
  delay(500);
}