#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_LEFT 13
#define BUTTON_RIGHT 12
#define BUTTON_BACK 11
#define BUTTON_UNLOCK 10

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

struct Compartment 
{
  int number;
  bool isLocked;

  bool usingTimer;
  bool usingRFID;
  bool usingPin;
  bool usingFP;

  char timer[5];
};
Compartment locker1;
Compartment locker2;
Compartment locker3;

Compartment * currentComp;
int currentState;

long buttonTime;

void setup() {
  Serial.begin(9600);
  while ( !Serial )
  {}
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_UNLOCK, INPUT_PULLUP);

  locker1 = createCompartment(1);
  locker2 = createCompartment(2);
  locker3 = createCompartment(3);

  currentComp = &locker1;
  currentState = 0;

  // Clear the buffer
  display.clearDisplay();

  locker1.isLocked = true;
  locker2.isLocked = true;
  locker2.usingTimer = true;
  locker2.timer[0] = '1';
  locker2.timer[1] = '2';
  locker2.timer[2] = '3';
  locker2.timer[3] = '4';


}

void loop() {
  switch(currentState)
  {
    case 0: {
      Menu();
      break;
    }
    case 1: {
      UnlockMenu();
      break;
    }
  }

}

/*
------------------------------------------------------------------------
-----------------------------Menu Functions-----------------------------
------------------------------------------------------------------------
*/

void Menu()
{
  drawMenu();
  char lastChar = currentComp->timer[3];
  while(1)
  {
    //Persistent loop function/functions
    //Check for update (change screen)

    if(!digitalRead(BUTTON_LEFT) && millis()-buttonTime > 300)
    {
      buttonTime = millis();
      leftPress();
      return;
    }

    if(!digitalRead(BUTTON_RIGHT) && millis()-buttonTime > 300)
    {
      buttonTime = millis();
      rightPress();
      return;
    }

    if(!digitalRead(BUTTON_UNLOCK) && millis()-buttonTime > 300 && currentComp->isLocked)
    {
      buttonTime = millis();
      currentState = 1;
      return;
    }

    if(currentComp->isLocked && currentComp->usingTimer && (lastChar != currentComp->timer[3]))
    {
      writeTimer(currentComp->number);
      lastChar = currentComp->timer[3];
    }
  }
}

void UnlockMenu()
{
  drawUnlockMenu(currentComp->number);
  while(1)
  {
    //Persistent loop function/functions
    //Check for update (change screen)

    if(!digitalRead(BUTTON_BACK) && millis()-buttonTime > 300)
    {
      buttonTime = millis();
      currentState = 0;
      return;
    }

    if(!digitalRead(BUTTON_UNLOCK) && millis()-buttonTime > 300)
    {
      buttonTime = millis();
      //Unlock compartment
      return;
    }

  }
}

void rightPress()
{
  if(currentComp->number == 1)
  {
    currentComp = &locker2;
  }
  else if(currentComp->number == 2)
  {
    currentComp = &locker3;
  }
  else
  {
    currentComp = &locker1;
  }
}

void leftPress()
{
  if(currentComp->number == 1)
  {
    currentComp = &locker3;
  }
  else if(currentComp->number == 2)
  {
    currentComp = &locker1;
  }
  else
  {
    currentComp = &locker2;
  }
}

/*
------------------------------------------------------------------------
---------------------------Struct Functions-----------------------------
------------------------------------------------------------------------
*/

Compartment createCompartment (int num)
{
  Compartment newCom;

  newCom.number = num;
  newCom.isLocked = false;

  newCom.usingTimer = false;
  newCom.usingRFID = false;
  newCom.usingPin = false;
  newCom.usingFP = false;

  return newCom;
}

void clearCompartment(Compartment * c)
{
  c->isLocked = false;

  c->usingTimer = false;
  c->usingRFID = false;
  c->usingPin = false;
  c->usingFP = false;
}

Compartment * getLockerPointer(int lockerNumber)
{
  switch(lockerNumber)
  {
    case 1:
    {
      return &locker1;
    }
    case 2:
    {
      return &locker2;
    }
    case 3:
    {
      return &locker3;
    }
  }
  return NULL;
}



/*
------------------------------------------------------------------------
---------------------------Display Functions----------------------------
------------------------------------------------------------------------
*/

void drawMenu()
{
  display.clearDisplay();

  drawSquare(currentComp->number);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  if(!currentComp->isLocked)
  {
    writeUnlocked();
  }
  else if(!currentComp->usingTimer)
  {
    writeLocked();
  }
  else
  {
    writeTimer(currentComp->number);
  }

  display.display();
}

void drawUnlockMenu(int num)
{
  display.clearDisplay();
  display.setTextSize(2,3);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(5, 6);

  display.write("Unlock #");
  display.write(String(num).c_str());
  display.write("?");

  display.display();
}

void drawSquare(int num) 
{
  int x = 3;
  int y = 3;
  display.fillRect(x, y, 26, 26, SSD1306_WHITE);

  display.setTextSize(3);
  display.setTextColor(SSD1306_BLACK);
  display.cp437(true);

  display.setCursor(x+6, y+3);
  display.write(String(num).c_str());

  display.display();
}

void clearRight()
{
  display.fillRect(30, 0, 98, 32, SSD1306_BLACK);
  display.display();
}

void writeUnlocked()
{
  clearRight();

  display.setCursor(32, 10);
  display.write("Unlocked");

  display.display();
}

void writeLocked()
{
  clearRight();
  
  display.setCursor(42, 10);
  display.write("Locked");

  display.display();
}

void writeTimer(int num)
{
  clearRight();

  display.setCursor(37, 10);

  Compartment * comp = getLockerPointer(num);
  display.write(comp->timer[0]);
  display.write(comp->timer[1]);
  display.write("h:");
  display.write(comp->timer[2]);
  display.write(comp->timer[3]);
  display.write("m");

  display.display();
}


