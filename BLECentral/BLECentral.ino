/*
  -  Connect to Nano IoT Keyboard implant and relay BLE keystrokes over Serial 1 to Xbee. (10/9/20, Jonathan Fischer)
  -  Scan for Targets based on Service UUID, create list, scan for strongest target signal and connect
  - write last parsed datablock
*/

#include <ArduinoBLE.h>
int32_t data;
char dataRx[20];
char emptydata[20];
String parsedData[8];
String message = "";

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central - Relay test");
  Serial.println("Make sure to turn on the device.");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // Check if the peripheral is our board, the local name will be:
    // "BLE Keyboard"
    if (peripheral.localName() == "BLE Keyboard") {
      //Serial.print("RSSI: ");
      //Serial.println(peripheral.rssi());
      // stop scanning
      BLE.stopScan();

      monitorKeystrokes(peripheral);

      // peripheral disconnected, start scanning again
      BLE.scan();
    }
  }
}

void monitorKeystrokes(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering service 0xC103 ...");
  if (peripheral.discoverService("C103")) {
    Serial.println("Service discovered");
  } else {
    Serial.println("Attribute discovery failed.");
    peripheral.disconnect();

    while (1);
    return;
  }

  // send the password over
  BLECharacteristic passCharacteristic = peripheral.characteristic("0110");
  
  if (!passCharacteristic) {
    Serial.println("Peripheral does not have Password characteristic!");
    peripheral.disconnect();
    return;
  } else if (!passCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable Password characteristic!");
    peripheral.disconnect();
    return;
  }
    else {
      Serial.print("Writing password...");
      //Serial.println((char*)"8ajI=6otrlb@l?lp");
      passCharacteristic.writeValue((char*)"8ajI=6otrlb@l?lp");
  }
  

  // retrieve the keystroke characteristic
  BLECharacteristic keystrokeCharacteristic = peripheral.characteristic("0111");

  // subscribe to the keystroke characteristic
  Serial.println("Subscribing keystroke characteristic ...");
  if (!keystrokeCharacteristic) {
    Serial.println("nokeystroke characteristic found!");
    peripheral.disconnect();
    return;
  } else if (!keystrokeCharacteristic.canSubscribe()) {
    Serial.println("keystroke characterisitc is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!keystrokeCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    // check if the value of the simple key characteristic has been updated
    if (keystrokeCharacteristic.valueUpdated()) {
      // yes, get the value, characteristic is 1 byte so use byte value
      
      keystrokeCharacteristic.readValue(dataRx, 20);
      Serial.print("dataRx: ");
      Serial.print(dataRx);
      Serial1.write(dataRx);
      /*Serial.print("dataRx[0]: ");
      Serial.println(dataRx[0]);
      Serial.print("dataRx[1]: ");
      Serial.println(dataRx[1]);
      Serial.print("dataRx[2]: ");
      Serial.println(dataRx[2]);
      Serial.print("dataRx[3]: ");
      Serial.println(dataRx[3]);
      Serial.print("dataRx[4]: ");
      Serial.println(dataRx[4]);
      Serial.print("dataRx[5]: ");
      Serial.println(dataRx[5]);
      Serial.print("dataRx[6]: ");
      Serial.println(dataRx[6]);
      Serial.print("dataRx[7]: ");
      Serial.println(dataRx[7]);
    byte index = 0;
    String block = "";
    //parse each value looking for delimiters
    for (byte i = 0; i < sizeof(dataRx); i++){
      if (String(dataRx[i]) == ",") {
        parsedData[index] = block;
        index++;
      }
      else {
        block.concat(dataRx[i]);
      }
      if (i == sizeof(dataRx)-1){
        parsedData[index] = block;
      }
    }
    Serial.print("Parsed data array [0]: ");
    Serial.println(parsedData[0]);
        Serial.print("Parsed data array [1]: ");
    Serial.println(parsedData[1]);
        Serial.print("Parsed data array [2]: ");
    Serial.println(parsedData[2]);
        Serial.print("Parsed data array [3]: ");
    Serial.println(parsedData[3]);
        Serial.print("Parsed data array [4]: ");
    Serial.println(parsedData[4]);
        Serial.print("Parsed data array [5]: ");
    Serial.println(parsedData[5]);
        Serial.print("Parsed data array [6]: ");
    Serial.println(parsedData[6]);
        Serial.print("Parsed data array [7]: ");
    Serial.println(parsedData[7]);
      if (dataRx[0] != 0) {
        message.concat(getMod(byte(dataRx[0])));
        }
      for (byte i = 1; i < sizeof(datRx); i++){
          if (message.length() > 0){
            message.concat(" ");
          }
          message.concat(getKey(byte(dataRx[i])));
      }
      Serial.print("Key press: ");
      Serial.println(message);
    }
    else {
      for (byte i = 0; i < sizeof(dataRx); i++) dataRx[i] = emptydata[i];
    } */
    }
  }

  Serial.println("Keyboard disconnected disconnected!");
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

/*void findTarget(BLEDevice p){
  //address of previous target
  //previous target signal
  //Set Timer for 5 seconds

  //
  if (p) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(p.address());
    Serial.print(" '");
    Serial.print(p.localName());
    Serial.print("' ");
    Serial.print(p.advertisedServiceUuid());
    Serial.println();

    // Check if the peripheral is our board, the local name will be:
    // "BLE Keyboard"
    if (peripheral.localName() == "BLE Keyboard") {
      Serial.print("RSSI: ");
      Serial.println(peripheral.rssi());
      // stop scanning
      BLE.stopScan();

      // peripheral disconnected, start scanning again
      BLE.scan();
    }
  }
}*/
