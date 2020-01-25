#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <SPI.h>
#include "RF24.h"  // Download and Install (See above)

#define maxLineNumber 2
#define maxDigit 3
#define keypadEnable 1

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {3, 4, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 9, A5}; //connect to the column pinouts of the keypad
//byte rowPins[ROWS] = {3, 4, A11, A12}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {A13, A14, A15}; //connect to the column pinouts of the keypad

byte addresses[][6] = {"1Node"}; // Create address for 1 pipe.
int dataTransmitted;  // Data that will be Transmitted from the transmitter
String data = "";
int tnum;

const int numRows = 2;
const int numCols = 16;

char rowOne[15];
int countOne = 0;
int place = 7;
bool check = true;
char key;
int  flag_null_token = 0;

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
//SoftwareSerial mySerial(2, 3);

// (Create an instance of a radio, specifying the CE and CS pins. )
RF24 myRadio (5, 10); // "myRadio" is the identifier you will use in following methods

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A3, A2, A1, A0, 12, 11);

void setup()
{
  //  pinMode(Option,INPUT_PULLUP);
  //  keyboard.begin(Datapin, IRQpin);
  Serial.begin(115200);
  //mySerial.begin(4800);

  lcd.begin(numCols, numRows);
  lcdCustomText();

  dataTransmitted = 100; // Arbitrary known data to transmit. Change it to test...
  myRadio.begin();  // Start up the physical nRF24L01 Radio
  myRadio.setChannel(108);  // Above most Wifi Channels
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  myRadio.setPALevel(RF24_PA_MIN);
  //  myRadio.setPALevel(RF24_PA_MAX);  // Uncomment for more power
  myRadio.openWritingPipe( addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)
  delay(1000);
}

void loop()
{
  lcdCustomText();
  getDataFromKeypad();
  if (countOne <= (maxDigit + 1))
  {
    //mySerial.write(rowOne);
    String nullstring = "000";
    nullstring.setCharAt(0, rowOne[0]);
    nullstring.setCharAt(1, rowOne[1]);
    nullstring.setCharAt(2, rowOne[2]);
    tnum = nullstring.toInt();
    //myRadio.write( &rowOne, sizeof(rowOne) ); //  Transmit the data
    if (flag_null_token == 1)
    {
      flag_null_token = 0;
      int temp_null = 9001;
      myRadio.write( &temp_null, sizeof(temp_null) );
    }
    else
    {
      myRadio.write( &tnum, sizeof(tnum) ); //  Transmit the data
      Serial.println(tnum);
      memset(rowOne, 0, sizeof(rowOne));
    }
  }
  else
  {
    memset(rowOne, 0, sizeof(rowOne));
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Limit Reached");
    lcd.setCursor(2, 1);
    lcd.print("Enter Again");
    delay(1000);
  }

}

void getDataFromKeypad()
{
  place = 7;
  countOne = 0;
  do
  {
    key = keypad.getKey();
    if (isdigit(key) || (key == '*') || ( key == '#'))
    {
      Serial.print(key);
      if (key == '*')
      {
        if (countOne == 0)
        { //mySerial.write(key);
          flag_null_token = 1;
          //myRadio.write( &key, sizeof(key) ); //  Transmit the data
        }
        else
        {
          memset(rowOne, 0, sizeof(rowOne));
          countOne = 0;
        }

        lcd.clear();
        lcd.setCursor(8, 0);
        lcd.print(key);
        delay(1000);
        lcdCustomText();
        break;
      }
      //      mySerial.write(key);
      rowOne[countOne++] = key;
      if (countOne == 1)
        lcd.clear();
      if ((countOne <= maxDigit) && (key == '#'))
      {
        for (int i = 0; i < countOne; i++)
        {
          rowOne[maxDigit - i] = rowOne[(countOne - 1) - i];
        }
        for (int i = 0; i < ((maxDigit + 1) - countOne); i++)
        {
          if (countOne == 1)
            rowOne[i] = ' ';
          else
          {
            rowOne[i] = '0';
          }
        }
      }
      lcd.setCursor(place++, 0);
      lcd.print(key);
    }
    if (key == '#')
    {
      lcdCustomText();
    }
  } while (key != '#');
}

void lcdCustomText()
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Spectrum");
  lcd.setCursor(2, 1);
  lcd.print("Technologies");
}
