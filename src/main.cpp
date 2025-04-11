#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define EEPROMPORT1 1  // Начало имени (10 байт)
#define EEPROMPORT2 11 // Сумма
#define EEPROMPORT3 12 // Начало UID (4 байта)

MFRC522 mfrc522(SS_PIN, RST_PIN);

class Person
{
public:
  char Name[10];
  byte sum;
  byte mfrc[4];
};

Person me;

void setup()
{
  Serial.begin(9600); // Инициализация Serial в начале
  SPI.begin();
  mfrc522.PCD_Init();

  // Читаем данные из EEPROM
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

  // Проверка на пустую карту и ввод имени
  if (me.mfrc[0] == 0xFF && me.mfrc[1] == 0xFF) // Проверка на пустую EEPROM
  {
    Serial.println("Введите имя через Serial (до 9 символов):");
    while (Serial.available() == 0)
    {
    }
    String input = Serial.readStringUntil('\n');
    input.trim();
    strncpy(me.Name, input.c_str(), 9);
    me.Name[9] = '\0'; // Завершающий нуль
    for (int i = 0; i < 10; i++)
    {
      EEPROM.write(EEPROMPORT1 + i, me.Name[i]);
    }
  }

  Serial.println("Поднесите карту к считывателю...");
}

void loop()
{
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Регистрация новой карты, если UID пустой
  if (me.mfrc[0] == 0 && me.mfrc[1] == 0 && me.mfrc[2] == 0 && me.mfrc[3] == 0)
  {
    for (byte i = 0; i < 4; i++)
    {
      me.mfrc[i] = mfrc522.uid.uidByte[i];
      EEPROM.write(EEPROMPORT3 + i, me.mfrc[i]);
    }
    Serial.println("Карта зарегистрирована!");
  }

  // Сравнение UID
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
      Serial.println("Сумма достигла максимума (255)!");
    }
    Serial.print("Имя: ");
    Serial.println(me.Name);
    Serial.print("Сумма: ");
    Serial.println(me.sum);
  }
  else
  {
    Serial.println("Карта не соответствует!");
  }

  mfrc522.PICC_HaltA();
  delay(100);
}