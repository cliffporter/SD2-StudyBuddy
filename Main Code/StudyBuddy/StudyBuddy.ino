#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Adafruit_TCA8418.h>


//Paramaters
#define RFID_RESCAN_DELAY 1000
#define RFID_TAG {0x93, 0xFE, 0xCF, 0x1D}
#define RFID_CARD {0x73, 0x9B, 0x17, 0x94}
#define SERVO_UNLOCK_POSITION 180
#define SERVO_LOCK_POSITION 0
#define KEYPAD_ROWS 3
#define KEYPAD_COLS 4

//Peripheral assignment
//(using hardware I2C, SPI, UART)
#define DISP_CS             13
#define DISP_RST            2
//<setup for display here>

#define RFID_CS             11 
#define RFID_RST            10              
MFRC522 rfid(RFID_CS, RFID_RST);

#define SERVO_1             9
#define SERVO_2             7
#define SERVO_3             5
Servo servo1;
Servo servo2;
Servo servo3;

#define REED_1              5
#define REED_2              6
#define REED_3              7
Adafruit_TCA8418 tio;
Adafruit_TCA8418 keypad;
char keymap[KEYPAD_COLS][KEYPAD_ROWS] = {
    {'#', '0', '*'},
    {'9', '8', '7'},
    {'6', '5', '4'},
    {'3', '2', '1'},
};


//Globals
struct Compartment {
  int number;
  bool isLocked;
  bool openedEarly;
  Servo servo;

  bool usingTimer;
  long unlockTime;
  bool usingRFID;
  byte authTag[10];
  bool usingPin;
  char pin[5];
  bool usingFP;
  int fpNum;
};
Compartment locker1;
Compartment locker2;
Compartment locker3;

long lastRFIDScan;
byte lastRFIDTag[10];

char enteredPin[5];
int pinIndex; 


void setup() 
{
  Serial.begin(9600); 
  while (!Serial); //wait for serial to begin
  Serial.println("Serial Begin:");
  SPI.begin();

  //RFID setup
  rfid.PCD_Init();	
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see RFID lib Readme
	rfid.PCD_DumpVersionToSerial();

  //Servos
  servo1.attach(SERVO_1);
  servo2.attach(SERVO_2); 
  servo3.attach(SERVO_3); 

  //Locker setup
  locker1 = createCompartment(1);
  locker2 = createCompartment(2);
  locker3 = createCompartment(3);
  
  //TCA8418 I2C GPIO Expander/Keypad setup
  if (! tio.begin(TCA8418_DEFAULT_ADDR, &Wire)) 
  {
    Serial.println("TCA8418 not found, check wiring & pullups!");
    while (1);
  }
  if (! keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) 
  {
    Serial.println("keypad not found, check wiring & pullups!");
    while (1);
  }
  keypad.matrix(KEYPAD_ROWS, KEYPAD_COLS);
  tio.pinMode(REED_1, INPUT_PULLUP);
  tio.pinMode(REED_2, INPUT_PULLUP);
  tio.pinMode(REED_3, INPUT_PULLUP);
  keypad.flush();

  //pinMode(A1, INPUT);


  //Globals init
  pinIndex = 0;


  //Lock servos (remove later)
  servoLock(1);
  delay(1000);
  servoLock(2);
  delay(1000);
  servoLock(3);

  //Test unlocking
  byte tag[] = RFID_TAG;
  startLock(2, -1, tag, NULL, -1);
  Serial.println("Locker #2 locked with tag");

  startLock(1, 10, NULL, NULL, -1);
  Serial.println("Unlocking #1 in 10s");
}

void loop() 
{
  //Check for unlocking conditions
  checkTimeUp();
  scanRFID();

  //pin
  char pressed = checkKeypad();
  pinBuilder(pressed);

}

/*--------------------------------------------*/
/*---------------RFID FUNCTIONS---------------*/
/*--------------------------------------------*/

//check for an RFID tag
//returns a pointer to the byte array containing its uid
byte * scanRFID() 
{
  if ( ! rfid.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return NULL;
  }
  if ( ! rfid.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return NULL;
  }

  //A tag was found
  byte * scanned = rfid.uid.uidByte;

  //ignore the last scanned tag untill it has been removed for x seconds
  if( memcmp(scanned, &lastRFIDTag, sizeof(scanned)) == 0 && millis()-lastRFIDScan < RFID_RESCAN_DELAY)
  {
    lastRFIDScan = millis();
    return NULL;
  }

  Serial.print("Scanned tag: ");
  printByteAr(scanned);
  
  // byte card[] = RFID_CARD;
  // byte tag[] = RFID_TAG;

  // if( memcmp(scanned, card, sizeof(scanned)) == 0)
  // {
  //   Serial.println("card!");
  // }
  // else if ( memcmp(scanned, tag, sizeof(scanned)) == 0)
  // {
  //   Serial.println("tag!");
  // }

  //Save info to prevent rescaning
  memcpy(lastRFIDTag, scanned, sizeof(scanned));
  lastRFIDScan = millis();

  //Check if this RFID tag is registered to a locker
  // int lockerNum = checkForRegisteredRFID(scanned);
  // if(lockerNum > 0)
  //   unlock(lockerNum);

  return scanned;
}

//Returns the locker number of the fist locker with a matching rfid tag
//returns -1 if none
int checkForRegisteredRFID(byte * scanned)
{
  //printByteAr(locker2.authTag);

  if( locker1.usingRFID && memcmp(scanned, locker1.authTag, sizeof(scanned)) == 0)
  {
    return 1;
  }
  else if ( locker2.usingRFID && memcmp(scanned, locker2.authTag, sizeof(scanned)) == 0)
  {
    return 2;
  }
  else if ( locker3.usingRFID && memcmp(scanned, locker3.authTag, sizeof(scanned)) == 0)
  {
    return 3;
  }
  else
  {
    return -1;
  }
}
//Print an array of bytes in hex
void printByteAr(byte * b)
{
  Serial.print("0x");
  char hexCar[2];
  for (int i=0; i < sizeof(b); i++)
  {
     //Serial.print(b[i], HEX);
    sprintf(hexCar, "%02X", b[i]);
    Serial.print(hexCar);
  }
  Serial.println();
}

/*----------------------------------------------*/
/*---------------LOCKER FUNCTIONS---------------*/
/*----------------------------------------------*/

//Configure and start lock
void startLock(int lockerNumber, int seconds, byte * rfidTag, char * pin, int fpNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);

  if(seconds > 0)
    setLockTime(comp, seconds);
  if(rfidTag != NULL)
    setRFIDTag(comp, rfidTag);
  if(pin != NULL )
    setPin(comp, pin);
  if(fpNumber >= 0 )
    setFingerprint(comp, fpNumber);
  
  if(comp->usingTimer || comp->usingRFID || comp->usingPin || comp->usingFP)
  {
    servoLock(lockerNumber);
  }
}
//Main unlock function
void unlock ( Compartment * c )  //can call with pointer or number (see helpers)
{
  Serial.printf("Unlocking Locker %d\n", c->number);
  servoUnlock(c->number);
  clearLocker(c);
}

//Initialize a Compartment
Compartment createCompartment (int lockerNumber)
{
  Compartment newCom;
  Servo srvo = getServo(lockerNumber);

  newCom.number = lockerNumber;
  newCom.isLocked = false;
  newCom.openedEarly = false;
  newCom.servo = srvo;

  newCom.usingTimer = false;
  newCom.unlockTime = -1;
  newCom.usingRFID = false;
  newCom.usingPin = false;
  newCom.usingFP = false;
  newCom.fpNum = -1;
  
  
  return newCom;
}
//Clear/ reinitialize a Compartment
void clearLocker(Compartment * c)
{
  c->isLocked = false;
  c->openedEarly = false;

  c->usingTimer = false;
  c->unlockTime = -1;
  c->usingRFID = false;
  memset(c->authTag, 0, sizeof(c->authTag));
  c->usingPin = false;
  memset(c->pin, 0, sizeof(c->pin));
  c->usingFP = false;
  c->fpNum = -1;
}
//Check all compartments to see if their time is up, unlock if true
void checkTimeUp()
{
  uint32_t time = millis();
  
  if(locker1.usingTimer && time >= locker1.unlockTime )
  {
    unlock(&locker1);
  }

  if(locker2.usingTimer && time >= locker2.unlockTime )
  {
    unlock(&locker2);
  }

  if(locker3.usingTimer && time >= locker3.unlockTime )
  {
    unlock(&locker3);
  }
}
//Unlock a compartment, check for errors and clear its settings
void unlock ( int lockerNumber )
{
  Compartment * comp = getLockerPointer(lockerNumber);
  unlock(comp);
}
void setLockTime(Compartment * c, int seconds)
{
  c->usingTimer = true;
  c->unlockTime = millis() + seconds*1000;
}
void setRFIDTag(Compartment * c, byte * tag)
{
  c->usingRFID = true;
  memcpy(c->authTag, tag, sizeof(tag));  
}
void setPin(Compartment * c, char * pin)
{
  c->usingPin = true;
  memcpy(c->pin, pin, sizeof(pin));
}
void setFingerprint(Compartment * c, int fpNumber)
{
  c->usingFP = true;
  c->fpNum = fpNumber;
}
//Returns the pointer of the locker specified
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
//Read the value of a reed switch 0->closed 1->open
int getReed(int lockerNumber)
{
  switch(lockerNumber)
  {
    case 1:
    {
      return tio.digitalRead(REED_1);
    }
    case 2:
    {
      return tio.digitalRead(REED_2);
    }
    case 3:
    {
      return tio.digitalRead(REED_3);
    }
  }
  return -1;
}


/*---------------------------------------------*/
/*---------------SERVO FUNCTIONS---------------*/
/*---------------------------------------------*/

//Get the servo object of the specified locker
Servo getServo(int servoNumber)
{
  switch(servoNumber)
  {
    case 1:
    {
      return servo1;
    }
    case 2:
    {
      return servo2;
    }
    case 3:
    {
      return servo3;
    }
  }
  Servo badServo;
  return badServo;
}
//Move the servo to the unlock position
void servoUnlock (int lockerNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);
  comp->servo.write(SERVO_UNLOCK_POSITION);
}
//Move the servo to the lock position
void servoLock (int lockerNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);
  comp->servo.write(SERVO_LOCK_POSITION);
}

/*----------------------------------------------*/
/*---------------KEYPAD FUNCTIONS---------------*/
/*----------------------------------------------*/

//Read the input on the keypad.
//returns 0 if no input or the char of the pressed key.
//clears input once read
char checkKeypad()
{
  if (keypad.available() > 0)
  {
    // datasheet page 15 - Table 1
    int k = keypad.getEvent();

    // Serial.print("K: ");
    // Serial.print(k, DEC);
    // Serial.print("\t");
    // printBinLZ(k);
    // Serial.print("\t");
    // Serial.print(k, HEX);
    // Serial.println();

    // we dont care about release events
    if ( !(k & 0x80) ) 
    {
      keypad.flush();
      return 0;
    }

    k &= 0x7F; //remove leading 1 -> 1000 0000
    k--; // decrement for array

    //check for invalid entries
    if( k%10 >= KEYPAD_COLS || k/10 >= KEYPAD_ROWS )
    {
      keypad.flush();
      return 0;
    }

    char pressed = keymap[k%10][k/10];
    Serial.printf("User pressed %c", pressed);
    Serial.println();

    // Serial.print("Row: ");
    // Serial.print((k%10)+1);
    // Serial.print("\tCol: ");
    // Serial.print((k/10)+1);
    // Serial.println();
    // Serial.println();

    keypad.flush();
    return pressed;
  }

  return 0;
}
//Print binary with leading zeros
void printBinLZ(int n)
{
  for (int i = 0; i < 8; i++)
  {
        if (n < pow(2, i))
              Serial.print(B0);
  }
  Serial.print(n, BIN);
}


/*--------------------------------------------------*/
/*---------------MENU/INPUT FUNCTIONS---------------*/
/*--------------------------------------------------*/

//Add a character to the pin.
//returns 1 if pin is now complete, 0 otherwise 
int pinBuilder(char c)
{
  if( pinIndex < 4 && c != 0)
  {
    enteredPin[pinIndex++] = c;
    //Serial.println(enteredPin);
  }
  else if (pinIndex == 4)
  {
    Serial.print("Full pin: ");
    Serial.println(enteredPin);
    pinIndex = 0;
    return 1;
  }

  return 0;
}


/*---------------------------------------------------*/
/*---------------FINGERPRINT FUNCTIONS---------------*/
/*---------------------------------------------------*/





