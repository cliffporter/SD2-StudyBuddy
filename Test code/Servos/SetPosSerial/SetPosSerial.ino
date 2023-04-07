/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
bool attached = false;
char buffer[6];

void setup() {
  // myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  while(!Serial) {}
  Serial.println("Enter servo pin:");
}

void loop() 
{
  // while (Serial.available())
  // {
  //   int input = Serial.parseInt();

  //   if(attached == false)
  //   {
  //     myservo.attach(input); 
  //     attached = true;
  //     Serial.println("Enter servo position:");
  //     break;
  //   }
  //   Serial.print("Writing");
  //   Serial.println(input);
  //   myservo.write(input);
  // }
  int index;  
  while (Serial.available()) 
  {
    index = Serial.readBytesUntil('\n', buffer, 5);  //newline or max of 5 chars
    buffer[index] = '\0';
  
    //Serial.println(buffer);  //so you can see the captured String 
    int n = atoi(buffer);  //convert readString into a number
    Serial.println(n); //so you can see the integer

    if(attached == false)
    {
      myservo.attach(n); 
      attached = true;
      Serial.println("Enter servo position:");
      break;
    }

    myservo.write(n);
    buffer[0] = '\0';
  } 
}

