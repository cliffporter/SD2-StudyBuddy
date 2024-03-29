/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using SPI to communicate
 4 or 5 pins are required to interface.

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
/*
#define OLED_MOSI   11
#define OLED_CLK   13
#define OLED_DC    9
#define OLED_CS    10
#define OLED_RESET 6
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
  */


#define OLED_DC     12
#define OLED_CS     13
#define OLED_RESET  2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

  int door1locked = false;
  int door2locked = false;
  int door3locked = false;

  unsigned long timer1end = 100000;
  unsigned long timer2end;
  unsigned long timer3end;

void setup() {
  Serial.begin(9600);
  //while(!Serial);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }


  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  //delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  /*drawMainMenu();
  delay(5000);
  drawUnlockedMenu('1');
  delay(5000);
  drawTimerMenu();
  delay(5000);
  drawPasscodeMenu();
  delay(5000);
  drawRFIDMenu();
  delay(5000);
  drawFingerprintMenu();
  delay(5000);
  drawFingerprintRemoveMenu();
  delay(5000);
  drawFingerprintRetryMenu();
  delay(5000);*/
  drawUnlockedMenu(1);
  delay(2000);
  drawLockOption(true);

}

void loop() {
  // if((millis()%1000) == 0) {
  //   redrawViewMenu('1');
  // }
}

unsigned long getTimer(char num) {
  int tempNum = (int)num;
  switch (tempNum) {
    case 1:
      return timer1end;
      break;

    case 2:
      return timer2end;
      break;

    case 3:
      return timer3end;
      break;

    default:
      return (unsigned long)0;
      break;
  }

}

void redrawViewMenu(int num) {
  unsigned long seconds = (getTimer(num)-millis())/1000;
  unsigned long minute = (seconds/60)%60;
  unsigned long hour = (seconds/3600)%100;

  String hours = String(hour);
  String minutes = String(minute);

  display.fillRect(40, 18, 86, 15, SSD1306_BLACK);

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setTextSize(1);
  display.setCursor(58, 21);
  display.write("No Timer");
  
  // if(hour<10) {
  //   display.write("0");
  //   display.write(hours.c_str());
  //   display.write("h:");
  // }
  // else {
  //   display.write(hours.c_str());
  //   display.write("h:");
  // }

  // if(minute<10) {
  //   display.write("0");
  //   display.write(minutes.c_str());
  //   display.write("m");
  // }
  // else {
  //   display.write(minutes.c_str());
  //   display.write("m");
  // }

  display.display();

}

void drawViewMenu(int num) {
  display.clearDisplay();

  unsigned long seconds = (getTimer(num)-millis())/1000;
  unsigned long minute = (seconds/60)%60;
  unsigned long hour = (seconds/3600)%100;

  String hours = String(hour);
  String minutes = String(minute);

  drawSquare(num, 5, 10);

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.setCursor(75, 55);
  display.write("#:Unlock");

  display.setTextSize(2);
  display.setCursor(40, 18);
  
  if(hour<10) {
    display.write("0");
    display.write(hours.c_str());
    display.write("h:");
  }
  else {
    display.write(hours.c_str());
    display.write("h:");
  }

  if(minute<10) {
    display.write("0");
    display.write(minutes.c_str());
    display.write("m");
  }
  else {
    display.write(minutes.c_str());
    display.write("m");
  }

  display.display();
}

void drawFingerprintRetryMenu() {
  display.clearDisplay();

  display.setTextSize(1,2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(20, 10);
  display.write("Re-Place Finger");

  display.setCursor(35, 30);
  display.write("On Scanner");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.display();
}

void drawFingerprintRemoveMenu() {
  display.clearDisplay();

  display.setTextSize(1,2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(25, 10);
  display.write("Remove Finger");

  display.setCursor(28, 30);
  display.write("From Scanner");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.display();
}

void drawFingerprintMenu() {
  display.clearDisplay();

  display.setTextSize(1,2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(30, 10);
  display.write("Place Finger");

  display.setCursor(35, 30);
  display.write("On Scanner");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.display();
}

void drawRFIDMenu() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(10, 20);
  display.write("Scan RFID");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.display();
}

void drawPasscodeMenu() {
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(30, 20);
  display.write("____");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.setCursor(75, 55);
  display.write("#:Submit");

  //remove later
  // display.setTextSize(3);
  // display.setCursor(30, 15);
  // display.write("1234");

  display.display();
}

void drawTimerMenu() {
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(1, 20);
  display.write("__");

  display.setCursor(display.getCursorX(), 15);
  display.write("h:");

  display.setCursor(display.getCursorX(), 20);
  display.write("__");

  display.setCursor(display.getCursorX(), 15);
  display.write("m");

  display.setTextSize(1);

  display.setCursor(5, 55);
  display.write("*:Back");

  display.setCursor(75, 55);
  display.write("#:Submit");

  //remove later
  // display.setTextSize(3);
  // display.setCursor(1, 15);
  // display.write("12  34");

  display.display();
}

void drawLockOption(bool isOff) {
  if(isOff)
  {
    display.setTextColor(SSD1306_WHITE);
  }
  else
  {
    display.setTextColor(SSD1306_BLACK);
  }
  display.setCursor(10, 39);
  display.write("#:Lock");

  display.display();
}

void drawUnlockedMenu(int num) {
  display.clearDisplay();

  drawSquare(num, 12, 5);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(60, 5);
  display.write("X");

  display.setCursor(70, 5);
  display.write("1:Timer");

  display.setCursor(60, 22);
  display.write("X");  

  display.setCursor(70, 22);
  display.write("2:Code");

  display.setCursor(60, 39);
  display.write("X");

  display.setCursor(70, 39);
  display.write("3:RFID");

  display.setCursor(60, 56);
  display.write("X");

  display.setCursor(70, 56);
  display.write("4:FPrint");

  // display.setCursor(10, 39);
  // display.write("#:Lock");

  display.setCursor(10, 56);
  display.write("*:Back");

  display.display();
}

void addChecks(int line, bool isOff) {
  display.setTextSize(1);
  display.cp437(true);

  display.setTextColor(SSD1306_BLACK);

  if(isOff) 
  {
    switch(line)
    {
      case 1:
        display.setCursor(60, 5);    
        display.write("X");
        break;

      case 2:
        display.setCursor(60, 22);
        display.write("X");
        break;

      case 3:
        display.setCursor(60, 39);
        display.write("X");
        break;

      case 4:
        display.setCursor(60, 56);
        display.write("X");
        break;

      default:
        break;
    }
  }
  else
  {
    switch(line)
    {
      case 1:
        display.setCursor(60, 5);
        display.write("O");
        break;

      case 2:
        display.setCursor(60, 22);
        display.write("O");
        break;

      case 3:
        display.setCursor(60, 39);
        display.write("O");
        break;

      case 4:
        display.setCursor(60, 56);
        display.write("O");
        break;

      default:
        break;
    }
  }

  display.setTextColor(SSD1306_WHITE);

  if(isOff)
  {
    switch(line)
    {
      case 1:
        display.setCursor(60, 5);
        display.write("O");
        break;

      case 2:
        display.setCursor(60, 22);
        display.write("O");
        break;

      case 3:
        display.setCursor(60, 39);
        display.write("O");
        break;

      case 4:
        display.setCursor(60, 56);
        display.write("O");
        break;

      default:
        break;
    }
  }
  else
  {
    switch(line)
    {
      case 1:
        display.setCursor(60, 5);    
        display.write("X");
        break;

      case 2:
        display.setCursor(60, 22);
        display.write("X");
        break;

      case 3:
        display.setCursor(60, 39);
        display.write("X");
        break;

      case 4:
        display.setCursor(60, 56);
        display.write("X");
        break;

      default:
        break;
    }
  }

  display.display();
}

void drawSquare(int num, int x, int y) {
  display.fillRect(x, y, 30, 30, SSD1306_WHITE);

  display.setTextSize(3);
  display.setTextColor(SSD1306_BLACK);
  display.cp437(true);

  display.setCursor(x+8, y+4);
  display.write(String(num).c_str());

  display.display();
}

void drawLock(int lockNum, bool locked, int x) {
  drawSquare(lockNum, x, 23);
  if(locked) {
    display.fillRect(x+4, 12, 22, 11, SSD1306_WHITE);
    display.fillRect(x+10, 17, 10, 6, SSD1306_BLACK);
  }
  else {
    display.fillRect(x+4, 5, 22, 18, SSD1306_WHITE);
    display.fillRect(x+10, 10, 10, 6, SSD1306_BLACK);
    display.fillRect(x+10, 16, 16, 7, SSD1306_BLACK);
  }
  display.display();
}

void drawMainMenu() {
  display.clearDisplay();

  drawLock('1', door1locked, 9);
  drawLock('2', door2locked, 49);
  drawLock('3', door3locked, 89);
}

