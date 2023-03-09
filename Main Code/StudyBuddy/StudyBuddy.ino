#include <SPI.h>
#include <MFRC522.h>


//Paramaters
#define RFID_RESCAN_DELAY 1000


//Peripheral assignment
//(using hardware I2C, SPI, UART)
#define DISP_CS             13
#define DISP_RST            2
//<setup for display here>

#define RFID_CS             11 
#define RFID_RST            10              
MFRC522 rfid(RFID_CS_PIN, RFID_RST_PIN);     // Create MFRC522 instance

#define SERVO_1             9
#define SERVO_2             7
#define SERVO_3             5
//<servo setup here>


//Globals
struct Compartment {
  int number;
  bool isLocked;
  bool usingTimer;
  long unlockTime;
  bool usingRFID;
  byte authTag[10];
  bool usingPin;
  int pin;
  bool usingFP;
  int fpNum;
  bool openedEarly;
};
Compartment locker1;
Compartment locker2;
Compartment locker3;


long lastRFIDScan;
byte lastRFIDTag[10];


void setup() 
{
  Serial.begin(9600); 
  while (!Serial); //wait for serial to begin
  SPI.begin();

  //RFID setup
  rfid.PCD_Init();	
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	rfid.PCD_DumpVersionToSerial();

  //Locker setup
  locker1 = createCompartment(1);
  //Serial.write("unlocking in 10s\n");
  setLockTime(&locker1, 10);
  
}

void loop() 
{
  //check for unlock time
  if(false && millis() >= locker1.unlockTime )
  {
    Serial.write("unlocking locker 1");
    unlock(&locker1);
  }

  scanRFID();

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

  //Tag found
  byte * scanned = rfid.uid.uidByte;

  //ignore the last scanned tag untill it has been removed for x seconds
  if( memcmp(scanned, &lastRFIDTag, sizeof(scanned)) == 0 && millis()-lastRFIDScan < RFID_RESCAN_DELAY)
  {
    lastRFIDScan = millis();
    return;
  }

  Serial.print("Scanned tag: ");
  printByteAr(scanned);

  byte card[] = {0x73, 0x9B, 0x17, 0x94};
  byte tag[] = {0x93, 0xFE, 0xCF, 0x1D};

  if( memcmp(scanned, card, sizeof(scanned)) == 0)
  {
    Serial.println("card!");
  }
  else if ( memcmp(scanned, tag, sizeof(scanned)) == 0)
  {
    Serial.println("tag!");
  }

  memcpy(lastRFIDTag, scanned, sizeof(scanned));
  lastRFIDScan = millis();
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
Compartment createCompartment (int lockerNumber)
{
  Compartment newCom;

  newCom.number = lockerNumber;
  newCom.isLocked = false;
  newCom.usingTimer = false;
  newCom.unlockTime = -1;
  newCom.usingRFID = false;
  newCom.usingPin = false;
  newCom.pin = -1;
  newCom.usingFP = false;
  newCom.fpNum = -1;
  newCom.openedEarly = false;
  
  return newCom;
}
void setLockTime(Compartment *c, int seconds)
{
  c->usingTimer = true;
  c->unlockTime = millis() + seconds*1000;
}
void unlock ( Compartment *c )
{
  c->usingTimer = false;
  c->unlockTime = -1;
}

