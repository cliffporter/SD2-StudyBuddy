#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


//Paramaters
#define RFID_RESCAN_DELAY 1000
#define SERVO_UNLOCK_POSITION 180
#define SERVO_LOCK_POSITION 0


//Peripheral assignment
//(using hardware I2C, SPI, UART)
#define DISP_CS             13
#define DISP_RST            2
//<setup for display here>

#define RFID_CS             11 
#define RFID_RST            10              
MFRC522 rfid(RFID_CS, RFID_RST);     // Create MFRC522 instance

#define SERVO_1             9
#define SERVO_2             7
#define SERVO_3             5
Servo servo1;
Servo servo2;
Servo servo3;

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
  int pin;
  bool usingFP;
  int fpNum;
};
Compartment locker1;
Compartment locker2;
Compartment locker3;


long lastRFIDScan;
byte lastRFIDTag[10];


void setup() 
{
  Serial.begin(9600); 
  //while (!Serial); //wait for serial to begin
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
  


  //Test unlocking
  byte card[] = {0x73, 0x9B, 0x17, 0x94};
  startLock(2, -1, card, -1, -1);
  Serial.println("Locker 2 locked with card");

  delay(2000); // wait so both servos arent moving at the same time

  startLock(1, 10, NULL, -1, -1);
  Serial.println("Unlocking #1 in 10s");
}

void loop() 
{
  //Check for unlocking conditions
  checkTimeUp();
  scanRFID();

  //Handle menu functionality
}

/*---------------RFID FUNCTIONS---------------*/
void scanRFID() 
{
  if ( ! rfid.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return;
  }
  if ( ! rfid.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return;
  }

  //A tag was found
  byte * scanned = rfid.uid.uidByte;

  //ignore the last scanned tag untill it has been removed for x seconds
  if( memcmp(scanned, &lastRFIDTag, sizeof(scanned)) == 0 && millis()-lastRFIDScan < RFID_RESCAN_DELAY)
  {
    lastRFIDScan = millis();
    return;
  }

  Serial.print("Scanned tag: ");
  printByteAr(scanned);
  
  // byte card[] = {0x73, 0x9B, 0x17, 0x94};
  // byte tag[] = {0x93, 0xFE, 0xCF, 0x1D};

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
  int lockerNum = checkForRegisteredRFID(scanned);
  if(lockerNum > 0)
    unlock(lockerNum);
}

//Returns the locker number of the fist locker with a matching rfid tag
//returns -1 if none
int checkForRegisteredRFID(byte * scanned)
{
  printByteAr(locker2.authTag);

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

/*---------------LOCKER FUNCTIONS---------------*/
//Configure and start lock
void startLock(int lockerNumber, int seconds, byte * rfidTag, int pin, int fpNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);

  if(seconds > 0)
    setLockTime(comp, seconds);
  if(rfidTag != NULL)
    setRFIDTag(comp, rfidTag);
  if(pin >= 0 )
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


//Simple helper functions
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
  newCom.pin = -1;
  newCom.usingFP = false;
  newCom.fpNum = -1;
  
  
  return newCom;
}
void clearLocker(Compartment * c)
{
  c->isLocked = false;
  c->openedEarly = false;

  c->usingTimer = false;
  c->unlockTime = -1;
  c->usingRFID = false;
  c->usingPin = false;
  c->pin = -1;
  c->usingFP = false;
  c->fpNum = -1;
}
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
void setPin(Compartment * c, int pin)
{
  c->usingPin = true;
  c->pin = pin;
}
void setFingerprint(Compartment * c, int fpNumber)
{
  c->usingFP = true;
  c->fpNum = fpNumber;
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

/*---------------SERVO FUNCTIONS---------------*/
void servoUnlock (int lockerNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);
  comp->servo.write(SERVO_UNLOCK_POSITION);
}
void servoLock (int lockerNumber)
{
  Compartment * comp = getLockerPointer(lockerNumber);
  comp->servo.write(SERVO_LOCK_POSITION);
}


