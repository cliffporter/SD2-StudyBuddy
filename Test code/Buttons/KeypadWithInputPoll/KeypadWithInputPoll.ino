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
  Serial.begin(9600);
  while (!Serial); 
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

        Serial.print("K: ");
        Serial.print(k, DEC);
        Serial.print("\t");
        printBinLZ(k);
        Serial.print("\t");
        Serial.print(k, HEX);
        Serial.println();

        if (k & 0x80) 
          Serial.print("PRESS\tKEY: ");
        else 
          Serial.print("RELEASE\tKEY: ");
        k &= 0x7F; //remove leading 1 -> 1000 0000
        k--; // decrement for array
        Serial.print(keymap[k%10][k/10]);
        Serial.println();

        Serial.print("Row: ");
        Serial.print((k%10)+1);
        Serial.print("\tCol: ");
        Serial.print((k/10)+1);
        Serial.println();
        Serial.println();

        keypad.flush();
      }
  }
  //Serial.println();

  // other code here
  //delay(1000);
}

void printBinLZ(int n)
{
  for (int i = 0; i < 8; i++)
  {
        if (n < pow(2, i))
              Serial.print(B0);
  }
  Serial.print(n, BIN);
}
