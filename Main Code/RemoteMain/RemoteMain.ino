#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

//Button pin definitions
#define BUTTON_LEFT 13
#define BUTTON_RIGHT 12
#define BUTTON_BACK 11
#define BUTTON_UNLOCK 10

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

long lastSent;
int msgNum;

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
  Serial.begin(115200);
  // while ( !Serial )
  // {}
  if(!Serial)
  {
    delay(100);
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_UNLOCK, INPUT_PULLUP);




  ///BLE Setup
  #if CFG_DEBUG
    // Blocking wait for connection when debug mode is enabled via IDE
    while ( !Serial ) yield();
  #endif
  
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");

  lastSent = millis()+2000;
  msgNum=1;





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

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
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
    case 2: {
      ConnectionMenu();
      break;
    }
  }

}

void LoopCheck()
{

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
  bool lastLocked = currentComp->isLocked;
  while(1)
  {
    LoopCheck();


    //Check for change in lock state to update screen
    if(lastLocked != currentComp->isLocked)
    {
      return;
    }

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
    LoopCheck();
    
    if(!currentComp->isLocked)
    {
      currentState = 0;
      return;
    }

    if(!digitalRead(BUTTON_BACK) && millis()-buttonTime > 300)
    {
      buttonTime = millis();
      currentState = 0;
      return;
    }

    if(!digitalRead(BUTTON_UNLOCK) && millis()-buttonTime > 300)
    {
      buttonTime = millis();

      TransmitUnlockSignal();

      currentState = 0;
      return;
    }

  }
}

//Menu to be held in while connection is broken
void ConnectionMenu()
{
  drawConnectionMenu();
  while(1)
  {
    LoopCheck();

  }
}

//shift currentComp right
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

//shift currentComp left
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
---------------------------BLE Functions--------------------------------
------------------------------------------------------------------------
*/

void TransmitUnlockSignal()
{

}

void RecieveTransmissions()
{

}


// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}


/*
------------------------------------------------------------------------
---------------------------Display Functions----------------------------
------------------------------------------------------------------------
*/

//draw primary menu based on currentComp
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

//draw unlock prompt
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

//draws current time remaining
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

void drawConnectionMenu()
{
  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(2);
  display.write("Connecting...");

  display.display();  
}


