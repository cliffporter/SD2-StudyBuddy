/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery


#define OLED_RESET     -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define BLE_TIMEOUT 5000

long lastSent;
int msgNum;

long transmitTime;
bool hasReceived;
long buttonTime;

void setup()
{
  Serial.begin(115200);

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


  //Display setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();


  pinMode(13, INPUT_PULLUP);

  lastSent = millis()+2000;
  msgNum=1;

  //transmitTime;
  hasReceived = true;

  drawWelcomeMenu();
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


void loop()
{
  //send a regular message
  // if( Bluefruit.connected() && millis()-lastSent >= 2000 )
  // {
  //   char send[20+1] = {'H','e','l','l','o','!', ' ', '#'};
  //   char intString[12+1];
  //   sprintf(intString, "%d", msgNum);
  //   strcat(send, intString);
  //   Serial.print("[Tx] ");
  //   Serial.println(send);

  //   bleuart.write( send );
  //   lastSent = millis();
  //   msgNum++;
  // }
  // Forward data from HW Serial to BLEUART
  while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }

  // Forward from BLEUART to HW Serial
  if ( bleuart.available() )
  {
    long receivedTime = millis();
    int latency = receivedTime-transmitTime;
    drawAckReceived(latency);


    Serial.print("[Rx] ");
    while ( bleuart.available() )
    {
      uint8_t ch;
      ch = (uint8_t) bleuart.read();
      Serial.write(ch);
    }
    Serial.println();
    Serial.println();

    hasReceived = true;
  }

  //transmit on button push
  if(!digitalRead(13) && millis()-buttonTime > 300 && hasReceived )
  {
    buttonTime = millis();
    drawConnectionTest(msgNum);

    char send[20+1] = {'H','e','l','l','o','!', ' ', '#'};
    char intString[12+1];
    sprintf(intString, "%d", msgNum);
    strcat(send, intString);
    Serial.print("[Tx] ");
    Serial.println(send);

    bleuart.write( send );

    hasReceived = false;
    transmitTime = millis();
    msgNum++;
  }

  //time out
  if( !hasReceived && millis()-transmitTime >= BLE_TIMEOUT)
  {
    drawTimedOut(millis()-transmitTime);
    hasReceived = true;
  }


}

void drawWelcomeMenu()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(0, 0);
  display.write("StudyBuddy BLE Test");

  display.setCursor(0, 10);
  display.write("Press button 1");

  display.setCursor(0, 20);
  display.write("to transmit");

  display.display();
}

void drawConnectionTest(int num)
{
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.setCursor(0, 0);
  display.write("Transmit #");

  //display.setCursor(0, 0);
  display.write(String(num).c_str());

  display.setCursor(0, 10);
  display.write("Awaiting ACK");
  

  display.display();
}

void drawAckReceived(int mills)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.fillRect(0, 10, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 10);
  display.write("ACK Received!");

  display.setCursor(0, 20);
  display.write("in ");
  display.write(String(mills).c_str());
  display.write("ms");
  display.display();
}

void drawTimedOut(int mills)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);

  display.fillRect(0, 10, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 10);
  display.write("Connection timed out");

  display.fillRect(0, 20, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 20);
  display.write("after ");
  display.write(String(mills).c_str());
  display.write("ms");
  display.display();
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
