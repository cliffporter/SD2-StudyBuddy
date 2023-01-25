#include <Adafruit_TCA8418.h>

Adafruit_TCA8418 tio;
Adafruit_TCA8418 keypad;

#define ROWS 3
#define COLS 4

char keymap[COLS][ROWS] = {
    {'#', '0', '*'},
    {'9', '8', '7'},
    {'6', '5', '4'},
    {'3', '2', '1'},
};


void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(__FILE__);

  if (! tio.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("TCA8418 not found, check wiring & pullups!");
    while (1);
  }

  if (! keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("keypad not found, check wiring & pullups!");
    while (1);
  }

  //  SET INPUT MODE
  tio.pinMode(5, INPUT_PULLUP);
  

  // configure the size of the keypad matrix.
  // all other pins will be inputs
  keypad.matrix(ROWS, COLS);

  // flush the internal buffer
  keypad.flush();

  //delay(1000);
}


void loop()
{
  if (!tio.digitalRead(5)){
    if (keypad.available() > 0)
      {
        //  datasheet page 15 - Table 1
        int k = keypad.getEvent();
        if (k & 0x80) Serial.print("PRESS\tR: ");
        else Serial.print("RELEASE\tR: ");
        k &= 0x7F;
        k--;
        //Serial.print(k / 10);
        //Serial.print("\tC: ");
        //Serial.print(k % 10);
        //Serial.println();
          Serial.print(keymap[k%10][k/10]);
          Serial.println();
          keypad.flush();
      }
  }
  //Serial.println();

  // other code here
  //delay(1000);
}
