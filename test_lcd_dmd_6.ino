#include <LiquidCrystal.h>
#include <Wire.h>
#include "String.h"
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

#include <SoftwareSerial.h>
#include <PS2Keyboard.h>

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A10, A11, A12, A13, A14, A15);
//LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

const int DataPin = 2;
const int IRQpin =  3;

PS2Keyboard keyboard;

int flag[3]={0,0,0};              // 0 -> no input , 1 -> input provided
char lcd_token[3][3];

char inChar;    
int cx=0 , cy=0;        //cursor positions
int crx = 0;
int count_enter = 0;

char Data[3][3];
int m,n,p,flag_t=0;
int kk=0;

void setup() {

  delay(1000);
  keyboard.begin(DataPin, IRQpin);
  Serial.begin(115200);
  
  // set up the LCD's number of columns and rows:
  pinMode(A10,OUTPUT);
  pinMode(A11,OUTPUT);
  pinMode(A12,OUTPUT);
  pinMode(A13,OUTPUT);
  pinMode(A14,OUTPUT);
  pinMode(A15,OUTPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.clear();

  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
      lcd_token[i][j]=' ';
    }
  }
  
  Timer1.initialize( 5000 );
  Timer1.attachInterrupt( ScanDMD );
  dmd.clearScreen( true );
}

void loop() {    
    inChar = keyboard.read();
  
    if(inChar > 0)
      {
        lcd.setCursor(crx, 1);
        lcd.print(inChar);
        //crx=1;
          
        do
        {          
          Serial.println(inChar);
          if(inChar == PS2_ENTER)
          {    
            Serial.println("Enter");        
            flag[cy]=1;

            if(kk<3)
            {
              while(kk<3)
              {
                lcd_token[cy][kk++] = ' '; 
              }        
            }
      
            if(lcd_token[cy][0] != ' ')
            {
              if((lcd_token[cy][1] == ' ') && (lcd_token[cy][2] == ' '))
              {
                lcd_token[cy][2] = lcd_token[cy][0];
                lcd_token[cy][0] = '0';
                lcd_token[cy][1] = '0';
              }
              else if((lcd_token[cy][1] != ' ') && (lcd_token[cy][2] == ' '))
              {
                lcd_token[cy][2] = lcd_token[cy][1];
                lcd_token[cy][1] = lcd_token[cy][0];
                lcd_token[cy][0] = '0';
              }
            }
      
            kk=0; 
            cy++;  
            cy=cy%3;      

            flag_t=0;
            for(int i=0;i<3;i++)
            {
              for(int j=0;j<3;j++)
              { 
                if(lcd_token[i][j] != ' ')
                flag_t++;
              }
            }
            dmd.clearScreen( true );
            lcd.setCursor(0, 1);
            lcd.print("                ");
            crx=0;
            break;
          }
        else if(inChar > 0)
        { 
          kk=kk%3;
          lcd_token[cy][kk++] = inChar;
          Serial.println("reg" + (String)lcd_token[cy][0] + (String)lcd_token[cy][1] + (String)lcd_token[cy][2]);
          lcd.setCursor(crx++, 1);
          lcd.print(inChar);          
        }
        inChar = keyboard.read();
      }while(1);
  
   }       
        lcd.setCursor(0, 0);
        disp_lcd();  
        disp_dmd();      
}

void disp_dmd()                                     //prints to dmd
{
  //dmd.clearScreen( true );
  dmd.selectFont(Arial_Black_16);

  //Serial.flush();

  initnull();
 
 for(int ii=0;ii<3;ii++)
  {
    if(flag[ii]==1)
    {
      for (byte x=0;x<DISPLAYS_ACROSS;x++) 
      {
        for (byte y=0;y<DISPLAYS_DOWN;y++) 
        {
          if(!((lcd_token[ii][0] == ' ') && (lcd_token[ii][1] == ' ') && (lcd_token[ii][2] == ' ')))
           {
           dmd.drawChar(  2,  1, lcd_token[ii][0], GRAPHICS_NORMAL );
           dmd.drawChar( 12,  1, lcd_token[ii][1], GRAPHICS_NORMAL );
           dmd.drawChar( 22,  1, lcd_token[ii][2], GRAPHICS_NORMAL );
           delay(2000);
           if(flag_t>3)
           dmd.clearScreen( true );
           }
          //delay(500);     
        }   
      }
    }
  }
         
}

void disp_lcd()                             //prints to lcd
{
  for(int ii=0;ii<3;ii++)
  { 
    lcd.print((String)lcd_token[ii][0] + (String)lcd_token[ii][1] + (String)lcd_token[ii][2]);
    lcd.print("  ");  
    Serial.print((String)lcd_token[ii][0] + (String)lcd_token[ii][1] + (String)lcd_token[ii][2]);
    Serial.print("  ");  
  }  
  Serial.println(""); 
}

void initnull()
{
  for(int ii=0;ii<3;ii++)
 {
  if(flag[ii]==0)
  {
    lcd_token[ii][0] = ' ';lcd_token[ii][1] = ' ';lcd_token[ii][2] = ' ';
    
    for(int ct=0;ct<3;ct++)
    {
      Data[ii][ct]=' ';
    }
  }
 }
}

