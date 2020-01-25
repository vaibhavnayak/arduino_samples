#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "String.h"
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include "RF24.h"  // Download and Install (See above)

// Defining Number of DMD Panels
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1

#define maxNumberCount 3
#define buzzer 4
#define countAddress 0
#define baseAddress 1

RF24 myRadio (5, 10); // "myRadio" is the identifier you will use in following methods
/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node"}; // Create address for 1 pipe.
int dataReceived;  // Data that will be received from the transmitter

char rowOne[3][15];

int numberCount = 2;
int counts = 0;
int countOne = 0;
int countTwo = 0;

bool check = true;

String data = "";
int rnum = 0, track = 0, tk = 0;
int fg = 0, rz = 0;
volatile int ff = 0;
long q = 0;
volatile int chk = 0;

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
//SoftwareSerial mySerial(3, 2);

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
  --------------------------------------------------------------------------------------*/
void ScanDMD()
{
  dmd.scanDisplayBySPI();
}

int flag[3] = {0, 0, 0};          // 0 -> no input , 1 -> input provided
char lcd_token[3][3];

char inChar;
int cx = 0 , cy = 0;    //cursor positions
int count_enter = 0;

char Data[3][3];
int m, n, p;
int kk = 0;

void setup()
{
  pinMode(buzzer, OUTPUT);
  //digitalWrite(buzzer, HIGH);
  Serial.begin(115200);
  delay(1000);
  //  initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 4820 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  dmd.clearScreen( true );
  dmd.selectFont(Arial_Black_16);
  Timer1.attachInterrupt( ScanDMD );
  myRadio.begin();  // Start up the physical nRF24L01 Radio
  myRadio.setChannel(108);  // Above most Wifi Channels
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  myRadio.setPALevel(RF24_PA_MIN);
  //  myRadio.setPALevel(RF24_PA_MAX);  // Uncomment for more power

  myRadio.openReadingPipe(1, addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)
  myRadio.startListening();
  attachInterrupt(0, irqpin, CHANGE);
  delay(1000);
}

void loop()
{

  Serial.println("            1 ");

  if (ff == 1)
  {
    //attachInterrupt(0, irqpin, CHANGE);
    Serial.println(" int atch ");
    // Timer1.attachInterrupt( ScanDMD );
    //END Radio available
    dmd.selectFont(Arial_Black_16);

    char cc = (rnum % 10) + '0';
    lcd_token[1][track] = cc;

    tk = track;
    track = (++track) % 2;

    Serial.println(" dmd ");
    ff = 0;
    // Timer1.detachInterrupt();
    //chk=1;
    Serial.println("            2 ");
    rz = 1;
    //tk = (++tk) % 2;

  }
  Serial.println("            3 ");

  if (rz == 0)
  {
    if (lcd_token[1][tk] != ' ')
    { dmd.clearScreen( true );
      dmd.drawChar(  2,  1, lcd_token[1][tk], GRAPHICS_NORMAL );
      chk = 0;
      Serial.println("            4 ");
      for (q = 0; q < 5000000; q++)
      {
        if (fg == 1)
        {
          break;
        }
        if (chk == 1)
        { Serial.println("            5 ");
          chk = 0;
          tk = (++tk) % 2;
          fg = 1;
          break;
        }
      }
      if (fg == 0)
      {
        tk = (++tk) % 2;
      }
    }
  }
  else
  {
    tk = (++tk) % 2;
  }
  Serial.println("            6 ");

  if (rz == 0)
  {
    if (lcd_token[1][tk] != ' ')
    { dmd.clearScreen( true );
      dmd.drawChar(  2,  1, lcd_token[1][tk], GRAPHICS_NORMAL );
      chk = 0;
      Serial.println("            7 ");
      //  fg=0;
      for (q = 0; q < 5000000; q++)
      {
        if (fg == 1)
        {
          break;
        }
        if (chk == 1)
        { Serial.println("            8 ");
          chk = 0;
          tk = (++tk) % 2;
          fg = 1;
          break;
        }
      }
      if (fg == 0)
      {
        tk = (++tk) % 2;
      }
    }
  }
  else
  {
    tk = (++tk) % 2;
  }
  Serial.println("            9 ");
  fg = 0;
  rz = 0;

}

void irqpin()
{
  // Timer1.detachInterrupt();

  if ( myRadio.available()) // chk for incoming data from transmitter
  {

    Serial.println("Int det");
    while (myRadio.available())  // While there is data ready
    {
      myRadio.read(&rnum, sizeof(rnum) ); // Get the data payload (You must have defined that already!)
      myRadio.read(&rowOne, sizeof(rowOne) ); //  Transmit the data
      Serial.println("inside while  "); Serial.println(myRadio.available());
      Serial.println(rnum);
      Serial.print(" row one : ");
      Serial.print(rowOne);
    }

    Serial.println("outside while");
    // DO something with the data, like print it
    Serial.print("Data received = ");
    Serial.println(rnum);
    //data="";
    if (rnum == 1)
    {
      Serial.println(" Execute 1 ");
    }
    else
    {
      Serial.println(" Bypass ");
    }

  }

  ff = 1;
  chk = 1;

}

void ringBell()
{
  digitalWrite(buzzer, HIGH);
  delay(50);
  digitalWrite(buzzer, LOW);
  // delay(1000);
}

void dmdPrintData()
{
  readDataEEPROM();
  // Serial.println(countOne);
  // Serial.println(counts);
  dmd.selectFont(Arial_Black_16);
  for (int i = 0; i <= counts; i++)
  {
    //Serial.println("hi");
    if (rowOne[i][0] != ' ')
    {
      Serial.println(rowOne[i]);
      dmd.drawChar( 2,  1, rowOne[i][0], GRAPHICS_NORMAL);
      dmd.drawChar( 12,  1, rowOne[i][1], GRAPHICS_NORMAL);
      dmd.drawChar( 22,  1, rowOne[i][2], GRAPHICS_NORMAL);
      delay(5000);
      dmd.clearScreen( true );
    }
  }
}

void writeDataEEPROM()
{
  if (check)
  {
    // Serial.println("hi");
    EEPROM.write(countAddress, numberCount % 3);
  }

  switch (numberCount % 3)
  {
    case 0:
      {
        for (int i = 0; i < countOne; i++)
          EEPROM.write(baseAddress + i, rowOne[numberCount][i]);
        break;
      }
    case 1:
      {
        for (int i = 0; i < countOne; i++)
          EEPROM.write(baseAddress + (3 * numberCount) + i, rowOne[numberCount][i]);
        break;
      }
    case 2:
      {
        for (int i = 0; i < countOne; i++)
          EEPROM.write(baseAddress + (3 * numberCount) + i, rowOne[numberCount][i]);
        break;
      }
    default:
      break;
  }
}

void readDataEEPROM()
{
  counts = EEPROM.read(countAddress);
  //Serial.println(numberCount);

  for (int j = 0; j <= counts; j++)
  {
    for (int i = 0; i < 3; i++)
    {
      rowOne[j][i] = EEPROM.read(baseAddress + (3 * j) + i);
      //Serial.println(rowOne[j][i]);
    }
  }
}

