#define tune1 150
#define tune2 50
#define tune3 300
#define ISA_GSM_SMS_CONFIG 1

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <PRGSM.h>
#define box_size 4
#define dosage_limit 4

struct
{
  int a[3] = {3, 0, 0};
  int b[dosage_limit][3];
  int flag = 0;
  int mode = 0;

  //mode 1  med needs to be taken
  //mode 2  med taken
  //mode 3  med not taken (This mode is NOT Used)
  //flag 1  box has been set
}  box[box_size];

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif

int handshake_flag = 1;
String inString = "";

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {3, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 9}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char customKey;

String __number__ = "" ;
String __message__ = "" ;

LiquidCrystal lcd(A10, A11, A12, A13, A14, A15);

// f1=1 -> atleast 1 box is set
// f3=1 -> meds need to be taken atleast for one box

int f1 = 0, f3 = 0;
int ii = 0, kk = 0, i = 0, j = 0, hr, mn, temp = 0, flag3 = 0;
int count_box = 0;
int addr = 0;
char c, eep;
int k, cursorx = 0, cursory = 3;
int st, dig = 0, nn = 0, num = -1;
int count = 1;
String stp = "", cx = "", cy = "";
uint16_t identifier;
int skip = 0;

int tyear, tmonth, tday, thour, tmin, tsec;
String tcx = "", tcy = "", tcz = "", dcx = "", dcy = "", dcz = "", dst = "", tst = "", tst2 = "";



void setup()
{
#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  ISASerial.begin(9600);
  gsmSerial.begin(9600);
  DataPort.begin(9600);


  pinMode(19, INPUT);           // BOX 1
  pinMode(18, INPUT);           // BOX 2
  pinMode(17, INPUT);           // BOX 3
  pinMode(16, INPUT);           // BOX 4

  pinMode( 8, OUTPUT);
  pinMode(11, OUTPUT);          // LED
  pinMode(12, OUTPUT);          // LED

  pinMode(10, OUTPUT);          // BUZZER

  for (kk = 0; kk < box_size; kk++)
  {
    for (i = 0; i < dosage_limit; i++)
    {
      for (j = 0; j < 3; j++)
      {
        box[kk].b[i][j] = 0;
      }
    }
  }
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  welcomescreen();
  retrive_from_eeprom();
  defaultpage();
}

String raw_data = "P02QS129/JULY/15 17:48:45;  24;  07; 50; 0000; 0000; 0000; 0000; 0000;000;;;0";
bool server_switch = false;

void loop()
{
  //ISAgsmPIPE();

  defaultpage();
  defaultdt();

if ((digitalRead(19) == 1) || (digitalRead(18) == 1) || (digitalRead(17) == 1) || (digitalRead(16) == 1))
box_is_open();

  while (1)
  {
    if (f1 == 1)
    {
      c = customKeypad.getKey();
      if (c == '1' || c == '2' || c == '3' || c == '4')
      {
        break;
      }
      else
      {
        c = ' '; checkmeds();
      }
    }

    else
    {
      break;
    }
  }

  if (c != '1' && c != '2' && c != '3' && c != '4')
    c = customKeypad.getKey();

  if (c == '1')
  {
    selectbox();
    if (ii != -5)
    {
      setvalues();
    }
  }
  else if (c == '2')
  {
    selectbox();
    if (ii != -5)
    {
      resetbox();
    }
  }
  else if (c == '3')
  {
    medsleft();
  }
  else if (c == '4')
  {
    selectbox();
    if (ii != -5)
    {
      schedule();
    }
  }

}


void setvalues()
{
  while (1)
  {
    lcd.clear();
    box[ii].a[0] = 1;

    if (box[ii].a[0] == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("QUANTITY : ");
      Serial.print("\nNo. of Meds : ");
      box[ii].a[1] = strint();
      Serial.print(box[ii].a[1]);
      delay(1000);
      lcd.clear();

      if (box[ii].a[1] >= 1)
      {
        lcd.setCursor(0, 0);
        lcd.print("DOSAGE (max 4) :  ");
        Serial.print("\n\nNo. of times per day (max 6) : ");
        box[ii].a[2] = strint();

        if (box[ii].a[2] > 0)
        {
          Serial.print(box[ii].a[2]);
          delay(1000);
          lcd.clear();
        }
        else
        {
          box[ii].a[0] = 3;
          Serial.println("\n");
          lcd.clear();
          break;
        }

        if (box[ii].a[2] > dosage_limit)
        {
          box[ii].a[0] = 3;
          Serial.println("\n");
          lcd.clear();
          break;
        }

        Serial.print("\n");
        for (i = 1; i <= box[ii].a[2]; i++)
        {
          lcd.setCursor(0, 0);
          lcd.print("Time " + (String)i + " (HHMM) : ");
          Serial.print("\nTime " + (String)i + " (HHMM) : ");
          st = strint();
          box[ii].b[i - 1][1] = st % 100;
          box[ii].b[i - 1][0] = st / 100;
          if (box[ii].b[i - 1][1] >= 0 && box[ii].b[i - 1][1] <= 60 && box[ii].b[i - 1][0] >= 0 && box[ii].b[i - 1][0] <= 23)
          {
            lcd.setCursor(0, 3);
            lcd.print((String)box[ii].b[i - 1][0] + ":" + (String)box[ii].b[i - 1][1]);
            Serial.print((String)box[ii].b[i - 1][0] + ":" + (String)box[ii].b[i - 1][1]);
            box[ii].b[i - 1][2] = 1;
            delay(1000);
            lcd.clear();
          }

          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter valid time");
            delay(1000);
            lcd.clear();
            Serial.print("\nEnter valid time");
            i--;
            continue;
          }
        }
        lcd.setCursor(0, 0);
        lcd.print(" DONE!");
        Serial.print("\n\n  DONE!");
        box[ii].flag = 1;
        box[ii].mode = 0;
        f1 = 1;

        write_to_eeprom();
        delay(1000);
        break;
      }

      else
      {
        box[ii].a[0] = 3;
        Serial.println("\n");
        break;
      }
    }
  }
}

void checkmeds()
{
  defaultpage();
  defaultdt();

  get_time();
  hr = thour;
  mn = tmin;
  f3 = 0;
  count_box = 0;

  for (kk = 0; kk < box_size; kk++)
  {
    for (i = 0; i < dosage_limit; i++)
    {
      if ((hr == box[kk].b[i][0]) && (mn == box[kk].b[i][1]) && (box[kk].flag == 1))
      {
        delay(1000);
        box[kk].mode = 1;
        count_box++;
        f3 = 1;
      }
    }
  }

  if (f3 == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dst);
    lcd.setCursor(12, 0);
    lcd.print(tst2);
    lcd.setCursor(5, 1);
    lcd.print("TAKE MEDS!");
    Serial.println("Take Meds!");
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);

    short_buzzer();

    delay(1000);

    mn = mn + 1;                 // Vary this to change the alert time
    if (mn >= 60)
    {
      mn = mn % 60; hr++;
    }
    if (hr == 24)
      hr = 0;

    for (kk = 0; kk < box_size; kk++)
    {
      if (box[kk].mode == 1)
      {
        lcd.setCursor(kk * 5, 3);
        lcd.print("Box" + (String)(kk + 1));
      }
    }
    check_meds_taken();
  }
}

void check_meds_taken()
{

  delay(1000);

  while (1)
  {
    get_time();
    if ((hr == thour) && (mn == tmin))
    {
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);
      break;
    }
    Serial.println("      inside while ");

    if (digitalRead(19) == 1)
    {
      if (box[0].mode == 1)
      {
        Serial.println("    BOX 1 ");
        count_box--;
        box[0].mode = 2;
        box[0].a[1]--;
        eep = box[0].a[1] + '0';
        EEPROM.write(1, eep);
        eep = box[0].mode + '0';
        EEPROM.write(4, eep);
        
        while(1)
        {
          if (digitalRead(19) == 0)
          break;
        }
      }
      else
      {
        wrong_medicine_buzzer();
      }
    }
    if (digitalRead(18) == 1)
    {
      if (box[1].mode == 1)
      {
        Serial.println("    BOX 2 ");
        count_box--;
        box[1].mode = 2;
        box[1].a[1]--;
        eep = box[1].a[1] + '0';
        EEPROM.write(101, eep);
        eep = box[1].mode + '0';
        EEPROM.write(104, eep);
        
        while(1)
        {
          if (digitalRead(18) == 0)
          break;
        }
      }
      else
      {
        wrong_medicine_buzzer();
      }
    }
    if (digitalRead(17) == 1)
    {
      if (box[2].mode == 1)
      { Serial.println("    BOX 3 ");
        count_box--;
        box[2].mode = 2;
        box[2].a[1]--;
        eep = box[2].a[1] + '0';
        EEPROM.write(201, eep);
        eep = box[2].mode + '0';
        EEPROM.write(204, eep);
        
        while(1)
        {
          if (digitalRead(17) == 0)
          break;
        }
      }
      else
      {
        wrong_medicine_buzzer();
      }
    }
    if (digitalRead(16) == 1)
    {
      if (box[3].mode == 1)
      { Serial.println("    BOX 4 ");
        count_box--;
        box[3].mode = 2;
        box[3].a[1]--;
        eep = box[3].a[1] + '0';
        EEPROM.write(301, eep);
        eep = box[3].mode + '0';
        EEPROM.write(304, eep);

        while(1)
        {
          if (digitalRead(16) == 0)
          break;
        }
      }
      else
      {
        wrong_medicine_buzzer();
      }
    }
  }

  delay(1000);
  lcd.clear();

  // Meds Taken or Not

  for (kk = 0; kk < box_size; kk++)
  {
    lcd.setCursor(0, kk);
    if (box[kk].mode == 2)
    {
      lcd.setCursor(0, kk);
      lcd.print("Box" + (String)(kk + 1) + " Meds Taken");
      box[kk].mode = 0;
    }
    else if (box[kk].mode == 1)
    {
      lcd.print("Box" + (String)(kk + 1) + " Meds Not Taken");
      box[kk].mode = 0;
      long_buzzer();

      __number__ = "+918908973381" ;
      __message__ = " Meds Not Taken" ;
      // sendSMS(__number__, __message__);

    }
  }

  delay(5000);


  medsleft();

  for (ii = 0; ii < box_size; ii++)
  {
    if (box[ii].a[1] == 0)
    {
      resetbox();
    }
  }
}




char get_time() {

  if (handshake_flag == 0) {

    ISAgsmSend("AT+CTZU=1");
    if (ISAgsmListen(1, 3, "OK")) return 1;
    else return 0;
  }

  else {

    ISAgsmSend("AT+CCLK?");

    if (ISAgsmListen(1, 5, "OK"))
    {
      String time_info = ISAgsmListenResponses[2];
      char start_format = time_info.indexOf('\"');
      char end_format = time_info.indexOf('\"', start_format + 1);
      time_info = time_info.substring(start_format, end_format);

      //Serial.println("time info:" + time_info);
      //s.set_time(time_info);
      inString = (String)time_info.charAt(1) + (String)time_info.charAt(2);      tyear = inString.toInt();
      inString = (String)time_info.charAt(4) + (String)time_info.charAt(5);      tmonth = inString.toInt();
      inString = (String)time_info.charAt(7) + (String)time_info.charAt(8);      tday = inString.toInt();
      inString = (String)time_info.charAt(10) + (String)time_info.charAt(11);      thour = inString.toInt();
      inString = (String)time_info.charAt(13) + (String)time_info.charAt(14);      tmin = inString.toInt();
      inString = (String)time_info.charAt(16) + (String)time_info.charAt(17);      tsec = inString.toInt();
      Serial.println((String)tday + "-" + (String)tmonth + "-" + (String)tyear + "\t" + (String)thour + ":" + (String)tmin + ":" + (String)tsec);

      tcx = ""; tcy = ""; tcz = "";
      dcx = ""; dcy = ""; dcz = "";

      dst = ""; tst = ""; tst2 = "";

      if (tday < 10)
        dcx = "0" + (String)tday;     else dcx = (String)tday;
      if (tmonth < 10)
        dcy = "0" + (String)tmonth;   else dcy = (String)tmonth;
      if (tyear < 10)
        dcz = "0" + (String)tyear;    else dcz = (String)tyear;

      dst = dcx + "/" + dcy + "/" + dcz;

      if (thour < 10)
        tcx = "0" + (String)thour;    else tcx = (String)thour;
      if (tmin < 10)
        tcy = "0" + (String)tmin;     else tcy = (String)tmin;
      if (tsec < 10)
        tcz = "0" + (String)tsec;     else tcz = (String)tsec;

      tst = tcx + ":" + tcy + ":" + tcz;
      tst2 = tcx + ":" + tcy;

      return 1;
    }
    else {
      Serial.println(F("CLOCK READ ERROR !"));
      Serial.println(F("-------"));
      return 0;
    }
  }
}


void lcdrewrite() {

  lcd.setCursor(cursorx, cursory);
  lcd.print(stp);
  //delay(100);
  Serial.println("rewrite");
}

int strint()
{
  dig = 0; nn = 0; flag3 = 0;

  while (1)
  {
    num = -1;
    Serial.println("type");

    while (1)
    {
      customKey = customKeypad.getKey();

      if (((int)customKey) != 0)
        break;
    }
    Serial.println("got");
    num = (int)customKey - 48;
    Serial.println(num);

    if (customKey >= 48 && customKey <= 57)      // number
    {
      Serial.println(String(num));

      if (dig < 4)
      {
        nn = (nn * 10) + num;
        dig++;
        stp = num;
        lcdrewrite();
        cursorx += 1;
      }
    }

    else if (customKey == 42)    // clear button
    {
      if (dig > 0)
      {
        dig--;
        cursorx -= 1;
        nn = nn / 10;
        stp = " ";
        lcdrewrite();
      }
    }

    else if (customKey == 35)    // 'OK' button
    {
      Serial.println("OK");
      stp = nn;
      temp = nn;
      break;
    }
    //delay(100);
  }

  cursorx = 0; cursory = 3;
  Serial.println("temp= " + (String)temp);
  return temp;
}

void welcomescreen()
{
  int d1 = 100;
  char welcome[11] = {'S', 'M', 'A', 'R', 'T', ' '};
  char pr[15] = {'P', 'I', 'L', 'L', ' ', 'B', 'O', 'X'};

  lcd.clear();
  lcd.setCursor(7, 1);

  for (i = 0; i < 6; i++)
  {
    lcd.print(welcome[i]); delay(d1);
  }

  //delay(500);

  lcd.setCursor(6, 2);

  for (i = 0; i < 8; i++)
  {
    lcd.print(pr[i]); delay(d1);
  }

  delay(2000);
  lcd.clear();
}
void defaultpage()
{
  //lcd.clear();
  /*
    lcd.setCursor(0, 2);
    lcd.print(" 1.SET     2.RESET");
    lcd.setCursor(0, 3);
    lcd.print(" 3.CHECK MEDS");
  */
  lcd.setCursor(0, 0);
  lcd.print("1.SET      |");
  lcd.setCursor(0, 1);
  lcd.print("2.RESET    |");
  lcd.setCursor(0, 2);
  lcd.print("3.MEDS LEFT|");
  lcd.setCursor(0, 3);
  lcd.print("4.SCHEDULE |");

}

void defaultdt()
{
  get_time();

  /*  lcd.setCursor(0, 0);

    lcd.print(dst);
    lcd.setCursor(12, 0);
    lcd.print(tst);

    Serial.println(" OK  2 "+(String)tst + " "+(String)dst);
  */

  lcd.setCursor(12, 0);
  lcd.print(tst);
  lcd.setCursor(12, 3);
  lcd.print(dst);

}

void selectbox()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    SELECT BOX");
  lcd.setCursor(0, 2);
  lcd.print(" 1.Box 1    2.Box 2");
  lcd.setCursor(0, 3);
  lcd.print(" 3.Box 3    4.Box 4");

  c = ' ';

  while (1)
  {
    customKey = customKeypad.getKey();
    if (customKey == '1')
    {
      ii = 0; break;
    }
    else if (customKey == '2')
    {
      ii = 1; break;
    }
    else if (customKey == '3')
    {
      ii = 2; break;
    }
    else if (customKey == '4')
    {
      ii = 3; break;
    }
    else if (customKey == '0')
    {
      lcd.clear();
      ii = -5; break;
    }
  }

  c = ' ';
}

void resetbox()
{
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 3; j++)
    {
      box[ii].b[i][j] = 0;

    }
  }

  f1 = 0;
  if (box[ii].flag == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("   BOX " + (String)(ii + 1) + " RESET");
    delay(1000);
  }

  box[ii].a[0] = 3;
  box[ii].a[1] = 0;
  box[ii].a[2] = 0;
  box[ii].flag = 0;
  box[ii].mode = 0;

  lcd.clear();

  for (kk = 0; kk < box_size; kk++)
  {
    if (box[kk].flag == 1)
    {
      f1 = 1;
    }
  }
  write_to_eeprom();
}
void medsleft()
{
  // Number of Meds left in each Box

  lcd.clear();

  for (kk = 0; kk < box_size; kk++)
  {
    lcd.setCursor(0, kk);
    lcd.print("Box" + (String)(kk + 1) + " : " + (String)box[kk].a[1] + " Meds left");
  }
  delay(3000);
  lcd.clear();
  c = ' ';
}

void schedule()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  i = 0;

  if (box[ii].flag == 0)
  {
    lcd.print("NOT SCHEDULED");
  }
  else
  {
    while (box[ii].b[i][2] != 0)
    {
      lcd.setCursor(1, i++);
      lcd.print((String)box[ii].b[i - 1][0] + ":" + (String)box[ii].b[i - 1][1]);
    }
  }
  delay(2000);
  lcd.clear();
}

void short_buzzer()
{
  
  for (j = 0; j < 2; j++)
  { 
    
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(tune2);
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(tune2);
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(250);
  digitalWrite(10, HIGH);
  delay(tune3);
  digitalWrite(10, LOW);
  delay(tune2);
  digitalWrite(10, HIGH);
  delay(tune3);
  digitalWrite(10, LOW);
  delay(150);
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(tune2);
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(tune2);
  digitalWrite(10, HIGH);
  delay(tune1);
  digitalWrite(10, LOW);
  delay(tune2);
  }
}

void long_buzzer()
{
  digitalWrite(10, HIGH);
  delay(2000);
  digitalWrite(10, LOW);
  delay(1000);
}

void wrong_medicine_buzzer()
{
  digitalWrite(10, HIGH);
  delay(50);
  digitalWrite(10, LOW);
  delay(50);
}

void box_is_open()
{
  lcd.clear();
  lcd.setCursor(14, 0);
  lcd.print("PLEASE");
  lcd.setCursor(14, 1);
  lcd.print("CLOSE");
  lcd.setCursor(14, 2);
  lcd.print("THE ");
  lcd.setCursor(14, 3);
  lcd.print("BOX");
  
  digitalWrite(10, HIGH);
  delay(100);
  digitalWrite(10, LOW);
  delay(100);
    lcd.clear();
}

void retrive_from_eeprom()
{
  Serial.println(" Retrieve ");
  for (kk = 0; kk < 4; kk++)
  {
    addr = kk * 100;
    box[kk].a[0] = EEPROM.read(addr) - '0' ;
    Serial.println("a[o] = " + (String)addr + "value" + (String)box[kk].a[0]);
    box[kk].a[1] = EEPROM.read(++addr) - '0' ;
    Serial.println("a[1] = " + (String)addr + "value" + (String)box[kk].a[1]);
    box[kk].a[2] = EEPROM.read(++addr) - '0' ;
    Serial.println("a[2] = " + (String)addr + "value" + (String)box[kk].a[2]);
    box[kk].flag = EEPROM.read(++addr) - '0' ;
    Serial.println("flag = " + (String)addr + "value" + (String)box[kk].flag);
    box[kk].mode = EEPROM.read(++addr) - '0' ;
    Serial.println("mode = " + (String)addr + "value" + (String)box[kk].mode);

    if (box[kk].a[0] < 0)
      box[kk].a[0] = 0;
    if (box[kk].a[1] < 0)
      box[kk].a[1] = 0;
    if (box[kk].a[2] < 0)
      box[kk].a[2] = 0;
    if (box[kk].flag < 0)
      box[kk].flag = 0;
//    if (box[kk].mode < 0)
      box[kk].mode = 0;

    for (i = 0; i < 4; i++)
    {
      box[kk].b[i][0] = (((EEPROM.read(++addr)) - '0') * 10) + (EEPROM.read(++addr) - '0' );
      Serial.println("b[i][0] = " + (String)addr + "value" + (String)box[kk].b[i][0]);
      box[kk].b[i][1] = (((EEPROM.read(++addr)) - '0') * 10) + (EEPROM.read(++addr) - '0' );
      Serial.println("b[i][1] = " + (String)addr + "value" + (String)box[kk].b[i][1]);
      box[kk].b[i][2] = ((EEPROM.read(++addr)) - '0');
      Serial.println("b[i][2] = " + (String)addr + "value" + (String)box[kk].b[i][2]);

      if (box[kk].b[i][0] < 0)
        box[kk].b[i][0] = 0;
      if (box[kk].b[i][1] < 0)
        box[kk].b[i][1] = 0;
      if (box[kk].b[i][2] < 0)
        box[kk].b[i][2] = 0;

      if (box[kk].b[i][2] == 0)
      {
        box[kk].b[i][0] = 0;
        box[kk].b[i][1] = 0;
      }
    }
  }
  addr = 0;
}

void write_to_eeprom()
{
  addr = ii * 100;
  eep = box[ii].a[0] + '0'; Serial.println(" box[ii].a[0] " + (String)addr + "value" + (String)eep);
  EEPROM.write(addr, eep);
  eep = box[ii].a[1] + '0';
  EEPROM.write(++addr, eep); Serial.println(" box[ii].a[1] " + (String)addr + "value" + (String)eep);
  eep = box[ii].a[2] + '0';
  EEPROM.write(++addr, eep); Serial.println(" box[ii].a[2] " + (String)addr + "value" + (String)eep);
  eep = box[ii].flag + '0';
  EEPROM.write(++addr, eep); Serial.println(" box[ii].flag " + (String)addr + "value" + (String)eep);
  eep = box[ii].mode + '0';
  EEPROM.write(++addr, eep); Serial.println(" box[ii].mode " + (String)addr + "value" + (String)eep);

  for (i = 0; i < 4; i++)
  {

    if (box[ii].b[i][2] == 0)
    {
      eep = 0 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);
      eep = 0 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);
      eep = 0 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);
      eep = 0 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);
      eep = 0 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);
    }

    else
    {
      eep = ((box[ii].b[i][0]) / 10) + '0';
      EEPROM.write(++addr, eep); Serial.println(" (box[ii].b[i][0])/10 " + (String)addr + " value " + (String)eep);
      eep = (box[ii].b[i][0]) % 10 + '0';
      EEPROM.write(++addr, eep); Serial.println(" (box[ii].b[i][0])%10 " + (String)addr + " value " + (String)eep);
      eep = ((box[ii].b[i][1]) / 10) + '0';
      EEPROM.write(++addr, eep); Serial.println(" (box[ii].b[i][1])/10 " + (String)addr + " value " + (String)eep);
      eep = (box[ii].b[i][1]) % 10 + '0';
      EEPROM.write(++addr, eep); Serial.println(" (box[ii].b[i][1])%10 " + (String)addr + " value " + (String)eep);
      eep = 1 + '0';
      EEPROM.write(++addr, eep); Serial.println(" addr " + (String)addr + " value " + (String)eep);

    }
  }
}

