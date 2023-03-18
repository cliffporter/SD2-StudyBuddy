
//#include <Adafruit_TinyUSB.h>
#include <Servo.h>


long nextPrint;
long nextReading;
int ticks;
int reading;
int side;
int last4[4];
int valNum;
int lastMin[240];
int minIndex;

void setup() 
{
  Serial.begin(9600); 
  while (!Serial); //wait for serial to begin
  Serial.println("Serial Begin:");


  nextPrint = millis();
  nextReading = millis();

  side=0;
  ticks=0;
}

void loop() 
{
  
  if(millis() >= nextReading)
  {
    reading = analogRead(A1);
    //Serial.printf("Vibration:%d ", reading);
    //Serial.printf("Ref:1000 ");
    //Serial.printf("Min:0\n");

    if((side==0&&reading>500))
    {
      ticks++;
      side=1;
    }
    else if((side==1&&reading<500))
    {
      ticks++;
      side=0;
    }

    nextReading = millis()+1;
  }

  if(millis() >= nextPrint)
  {
    last4[valNum%4] = ticks;
    valNum++;
    lastMin[minIndex%240] = ticks;
    minIndex++;
    int meantps = (last4[0]+last4[1]+last4[2]+last4[3]);
    int tpm = 0;
    for(int i=0; i<240; i++)
    {
      tpm += lastMin[i];
    }

    Serial.printf("tps:%d tpm:%d\n", meantps, tpm);
    nextPrint = millis()+250;
    ticks=0;
  }
}
