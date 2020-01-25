#include <LiquidCrystal.h>
#include <Wire.h>
#include "String.h"
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

#include <SoftwareSerial.h>

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
//LiquidCrystal lcd(A10, A11, A12, A13, A14, A15);
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

int flag[3]={0,0,0};              // 0 -> no input , 1 -> input provided
char lcd_token[2][6];

char inChar;    
int cx=0 , cy=0;        //cursor positions
int count_enter = 0;

char Data[2][6];
int m,n,p,flag_t=0;
int kk=0;

void setup() {

  Serial.begin(115200);
  
  // set up the LCD's number of columns and rows:
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A5,OUTPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.clear();
  
  for(int i=0;i<2;i++)
  {
    for(int j=0;j<6;j++)
    {
      lcd_token[i][j]=' ';
    }
  }
  
  Timer1.initialize( 5000 );
  Timer1.attachInterrupt( ScanDMD );
  dmd.clearScreen( true );
}
void loop() {

  while(Serial.available()>0)
  { 
    inChar = Serial.read();
    if(inChar == '\n')
    { 
      flag[cy]=1;

      if(kk<2)
      {
        while(kk<2)
        {
          lcd_token[cy][kk++] = ' '; 
        }
        
      }
      kk=0; 
      cy++;  
      cy=cy%2;      

    }
    else
    { 
      kk=kk%2;
      lcd_token[cy][kk++] = inChar;
    }
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
 
 for(int ii=0;ii<2;ii++)
  {
    if(flag[ii]==1)
    {
      for (byte x=0;x<DISPLAYS_ACROSS;x++) 
      {
        for (byte y=0;y<DISPLAYS_DOWN;y++) 
        {
          if(!((lcd_token[ii][0] == ' ') && (lcd_token[ii][1] == ' ') && (lcd_token[ii][2] == ' ') && (lcd_token[ii][3] == ' ') && (lcd_token[ii][4] == ' ') && (lcd_token[ii][5] == ' ')))
           {
           dmd.drawChar(  2,  1, lcd_token[ii][0], GRAPHICS_NORMAL );
           dmd.drawChar( 12,  1, lcd_token[ii][1], GRAPHICS_NORMAL );
           dmd.drawChar( 22,  1, lcd_token[ii][2], GRAPHICS_NORMAL );
           dmd.drawChar( 32,  1, lcd_token[ii][3], GRAPHICS_NORMAL );
           dmd.drawChar( 42,  1, lcd_token[ii][4], GRAPHICS_NORMAL );
           dmd.drawChar( 52,  1, lcd_token[ii][5], GRAPHICS_NORMAL );
           
           delay(2000);
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
  }  
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

