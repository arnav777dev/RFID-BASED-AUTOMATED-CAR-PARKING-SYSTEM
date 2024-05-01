#include <MFRC522.h>
#include <Keypad.h>
#include<Servo.h>
#include <U8glib.h>

struct Car {
  int balance;
  unsigned long st;
  const char* UID;
  const char* cn;
  const char* pin;
};

#define SS_PIN 10
#define RST_PIN 9
Servo boom;
MFRC522 mfrc522(SS_PIN, RST_PIN);

const byte ROWS = 4;
const byte COLS = 4;
const char hexaKeys[ROWS][COLS]  = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {8, 7, 6};
byte colPins[COLS] = {5, 4, 3};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
Car carA  = {0, 0, "334E07FB", "HR 26 BV 7249", "1234"}; // Set pin for car A
Car carB = {500, 0, "334656EC", "UP 12    7777", "1234"}; // Set pin for car B
Car carC = {500, 0, "0315B111", "HR 26 EP 5621", "1234"}; // Set pin for car C
Car carD = {500, 0, "83037211", "UP 12    0001", "1234"}; // Set pin for car D
Car carE = {500, 0, "63A87011", "HR 31 J  0214", "1234"};
void setup() {
  boom.attach(2);
  u8g.begin();
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.begin(9600);
  boom.write(0);
}


void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    u8g.firstPage();
    do {
      String content = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      Serial.println(content);
      Car* currentCar = nullptr;
      if (content == carA.UID) {
        currentCar = &carA;
      } else if (content == carB.UID) {
        currentCar = &carB;
      } else if (content == carC.UID) {
        currentCar = &carC;
      } else if (content == carD.UID) {
        currentCar = &carD;
      } else if (content == carE.UID) {
        currentCar = &carE;
      }

      if (currentCar != nullptr) {
        processToll(currentCar);
      } else {
        u8g.firstPage();
        do {
          u8g.setFont(u8g_font_ncenB10);
          u8g.drawStr(15, 10, "Unauthorized");
          u8g.drawStr(30, 30, "Access!!");
          u8g.drawStr(60, 60, "X");
        } while ( u8g.nextPage() );
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    } while ( u8g.nextPage() );

    delay(2000);
  }
}

void processToll(Car* car) {
  if (car->st != 0) {
    int ct = millis() - car->st;
    float fees = (ct / 1000) * 0.5;

    if ( car->balance >= fees) {
      car->balance -= fees;
      car->st = 0;
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_ncenB10);
        u8g.drawStr(30, 11, "Fees Paid.");
        u8g.setPrintPos(20, 30);
        u8g.print("$");
        u8g.print(fees);
        u8g.setPrintPos(70, 30);
        u8g.print(ct / 1000);
        u8g.print(" hr");
        u8g.setPrintPos(15, 46);
        u8g.print(car->cn);
        u8g.setPrintPos(20, 60);
        u8g.print("Thank You!!");
      } while ( u8g.nextPage() );
      // Move the servo to 90 degrees
      boom.write(90);
      unsigned long startTime = millis(); // Record the start time
      while (millis() - startTime < 2000) {
        // Keep the servo at 90 degrees for 2 seconds
      }
      boom.write(0); // Reset the servo position

    } else {
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_ncenB10);
        u8g.drawStr(15, 10, "Insufficient");
        u8g.drawStr(30, 30, "Balance!!");
        u8g.drawStr(60, 60, "X");
      } while ( u8g.nextPage() );

      delay(1000);
      // Prompt for PIN
      String enteredPIN = promptForPIN();
      if (enteredPIN == car->pin) {
        car->st = 0;
        u8g.firstPage();
        do {
          u8g.setFont(u8g_font_ncenB10);
          u8g.drawStr(15, 30, "PIN Correct");
          u8g.setPrintPos(15, 46);
          u8g.print(car->cn);
          u8g.setPrintPos(20, 60);
          u8g.print("Thank You!!");
        } while ( u8g.nextPage() );
        // Move the servo to 90 degrees
        boom.write(90);
        unsigned long startTime = millis(); // Record the start time
        while (millis() - startTime < 2000) {}
        boom.write(0); // Reset the servo position
      } else {
        u8g.firstPage();
        do {
          u8g.setFont(u8g_font_ncenB10);
          u8g.drawStr(15, 30, "PIN Incorrect");
          u8g.drawStr(35, 50, "Be Aside!!");
        } while ( u8g.nextPage() );
      }
    }
  }
  else {
    car->st = millis();
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_ncenB10);
      u8g.drawStr(30, 10, "Welcome!!");
      u8g.setPrintPos(20, 30);
      u8g.setPrintPos(10, 50);
      u8g.print(car->cn);
    } while ( u8g.nextPage() );

    boom.write(90);
    unsigned long startTime = millis(); // Record the start time
    while (millis() - startTime < 2000) {}
    boom.write(0); // Reset the servo position
  }
}

String promptForPIN() {
  String enteredPIN = "";
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_ncenB10);
    u8g.drawStr(15, 20, "Enter PIN:");

    u8g.setFont(u8g_font_ncenB10);
    u8g.drawStr(30, 30, enteredPIN.c_str());

  } while ( u8g.nextPage() );

  while (enteredPIN.length() < 4) {
    char customKey = customKeypad.getKey();
    if (customKey) {
      enteredPIN += customKey;
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_ncenB24);
        u8g.drawStr(30, 30, enteredPIN.c_str());
      } while ( u8g.nextPage() );
      delay(200); // delay to avoid reading multiple times
    }
  }
  return enteredPIN;
}
