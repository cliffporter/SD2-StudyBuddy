/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

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
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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

void setup() {
  Serial.begin(9600);
  while ( !Serial )
  {}
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  bleTestWelcomeMenu();
  // delay(2000);
  // bleTestConnectionTest(5);
  // delay(1000);
  // bleTestAckReceived(1000);
  // delay(2000);
  // bleTestTimedOut(2000);
  
}

void bleTestWelcomeMenu()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(0, 0);
  display.write("StudyBuddy BLE Test");

  display.setCursor(0, 10);
  display.write("Press button 1");

  display.setCursor(0, 20);
  display.write("to transmit");

  display.display();
}

void bleTestConnectionTest(int num)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(0, 0);
  display.write("Transmit #");

  //display.setCursor(0, 0);
  display.write(String(num).c_str());

  display.setCursor(0, 10);
  display.write("Awaiting ACK");
  

  display.display();
}

void bleTestAckReceived(int mills)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.fillRect(0, 10, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 10);
  display.write("ACK Received!");

  display.setCursor(0, 20);
  display.write("in ");
  display.write(String(mills).c_str());
  display.write("ms");
  display.display();
}

void bleTestTimedOut(int mills)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.fillRect(0, 10, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 10);
  display.write("Connection timed out");

  display.fillRect(0, 20, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 20);
  display.write("after ");
  display.write(String(mills).c_str());
  display.write("ms");
  display.display();
}

void loop() {
}

