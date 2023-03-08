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


#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

  int door1locked = false;
  int door2locked = false;
  int door3locked = false;

  unsigned long timer1end;
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
  delay(2000);
  drawUnlockedMenu('1');
  delay(2000);
  drawTimerMenu();
  delay(2000);
  drawPasscodeMenu();
  delay(2000);
  drawRFIDMenu();
  delay(2000);
  drawFingerprintMenu();
  delay(2000);
  drawFingerprintRetryMenu();*/
  timer1end = (unsigned long)10000;
}

void loop() {
  //unsigned long currentMillis = millis();
  //drawViewMenu('1',currentMillis);
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

void drawViewMenu(char num, unsigned long currTime) {
  display.clearDisplay();
  
  unsigned long timer = getTimer(num)-millis();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(5, 55);
  display.write(timer);

  display.display();
}

void drawFingerprintRetryMenu() {
  display.clearDisplay();

  display.setTextSize(1,2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(15, 10);
  display.write("Scan Fingerprint");

  display.setCursor(50, 30);
  display.write("Again");

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

  display.setCursor(15, 20);
  display.write("Scan Fingerprint");

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
  display.setTextSize(3);
  display.setCursor(30, 15);
  display.write("1234");

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
  display.setTextSize(3);
  display.setCursor(1, 15);
  display.write("12  34");

  display.display();
}

void drawUnlockedMenu(char num) {
  display.clearDisplay();

  drawSquare(num, 12, 5);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(70, 5);
  display.write("1:Timer");

  display.setCursor(70, 22);
  display.write("2:Code");

  display.setCursor(70, 39);
  display.write("3:RFID");

  display.setCursor(70, 56);
  display.write("4:FPrint");

  display.setCursor(10, 39);
  display.write("#:Lock");

  display.setCursor(10, 56);
  display.write("*:Back");

  display.display();
}

void drawSquare(char num, int x, int y) {
  display.fillRect(x, y, 30, 30, SSD1306_WHITE);

  display.setTextSize(3);
  display.setTextColor(SSD1306_BLACK);
  display.cp437(true);

  display.setCursor(x+8, y+4);
  display.write(num);

  display.display();
}

void drawLock(char lockNum, bool locked, int x) {
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

