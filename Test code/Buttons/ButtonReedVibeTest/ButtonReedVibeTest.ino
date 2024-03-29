
/***************************************************

  @file tca8418_gpio.ino

  This is an example for the Adafruit TCA8418 Keypad Matrix / GPIO Expander Breakout

  Designed specifically to work with the Adafruit TCA8418 Keypad Matrix
  ----> https://www.adafruit.com/products/XXXX

  These Keypad Matrix use I2C to communicate, 2 pins are required to
  interface.
  The Keypad Matrix has an interrupt pin to provide fast detection
  of changes. This example shows the working of polling.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_TCA8418.h>

Adafruit_TCA8418 tio;


void setup()
{
  Serial.begin(9600); 
  while (!Serial)
  // if(!Serial)
  //   delay(500);
  Serial.println(__FILE__);
  Serial.println("foo");

  if (! tio.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("TCA8418 not found, check wiring & pullups!");
    while (1);
  }
  Serial.println("bar");

  //  SET INPUT MODE
  for (int pin = 0; pin < 18; pin++)
  {
    tio.pinMode(pin, INPUT_PULLUP);
  }

  delay(1000);
}


void loop()
{
  //  SHOW PIN STATUS
  for (int pin = 0; pin < 18; pin++)
  {
    Serial.print(tio.digitalRead(pin));
    Serial.print(' ');
  }
  Serial.println();

  // other code here
  delay(1000);
}
