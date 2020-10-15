/*
  -  Connect to Nano IoT Keyboard implant and relay BLE keystrokes over Serial 1 to Xbee. (10/9/20, Jonathan Fischer)
  -  Scan for Targets based on Service UUID, create list, scan for strongest target signal and connect
  - Special note: if connection to a parameter is needed, run it in a loop until the characteristic is found
*/

#include <ArduinoBLE.h>
int32_t data;
char dataRx[20];
char emptydata[20];
String parsedData[8];
String message = "";
bool lockout = false;

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

  // start scanning for peripheral address passed from Xbee
  //BLE.scan();
  BLE.scanForAddress("24:0a:c4:c5:05:36");
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
      Serial.print("Found target: ");
      Serial.println(peripheral.address());
      peripheral.connect();
        BLE.stopScan();
        //setInsomnia(peripheral);
        setRecordKeys(peripheral);
        delay(1000);
        monitorKeystrokes(peripheral);
        // peripheral disconnected, start scanning again
        BLE.scan();
    }
  }
}

void monitorKeystrokes(BLEDevice p) {
  // connect to the peripheral
  Serial.println("Connecting ...");
  if (p.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }
  p.discoverService("C103");
  // send the password over
  BLECharacteristic passCharacteristic = p.characteristic("0110");
  
  if (!passCharacteristic) {
    Serial.println("Peripheral does not have Password characteristic!");
    p.disconnect();
    return;
  } else if (!passCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable Password characteristic!");
    p.disconnect();
    return;
  }
    else {
      Serial.println("Writing password...");
      //Serial.println((char*)"8ajI=6otrlb@l?lp");
      //passCharacteristic.writeValue((char*)"8ajI=6otrlb@l?lp");
      passCharacteristic.writeValue((char*)"pass");
  }
  

  // retrieve the keystroke characteristic
  BLECharacteristic keystrokeCharacteristic = p.characteristic("0111");

  // subscribe to the keystroke characteristic
  Serial.println("Subscribing keystroke characteristic ...");
  if (!keystrokeCharacteristic) {
    Serial.println("no keystroke characteristic found!");
    p.disconnect();
    return;
  } else if (!keystrokeCharacteristic.canSubscribe()) {
    Serial.println("keystroke characterisitc is not subscribable!");
    p.disconnect();
    return;
  } else if (!keystrokeCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    p.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
  }

  while (p.connected()) {
    // while the peripheral is connected

    // check if the value of the simple key characteristic has been updated
    if (keystrokeCharacteristic.valueUpdated()) {
      // yes, get the value, characteristic is 1 byte so use byte value
      
      keystrokeCharacteristic.readValue(dataRx, 20);
      Serial.print("dataRx: ");
      Serial.println(dataRx);
      Serial1.write(dataRx);
    }
  }
  Serial.println("Device disconnected!");
}

void setInsomnia(BLEDevice p){
  if (p) {
    p.connect();
    p.discoverService("C103");

    // send the password over
    BLECharacteristic insomniaCharacteristic = p.characteristic("0101");

    // subscribe to the insomnia characteristic
    Serial.println("Finding insomnia characteristic ...");
    if (!insomniaCharacteristic) {
      Serial.println("no insomnia characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Insomnia characteristic found!");
      Serial.println("Writing insomnia activation password");
      insomniaCharacteristic.writeValue((char*)"Caffeine");
    }
  }
  p.disconnect();
}

void allowSleep(BLEDevice p){
  if (p) {
    p.connect();
    p.discoverService("C103");

    // send the password over
    BLECharacteristic insomniaCharacteristic = p.characteristic("0101");

    // subscribe to the insomnia characteristic
    Serial.println("Finding insomnia characteristic ...");
    if (!insomniaCharacteristic) {
      Serial.println("no insomnia characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Insomnia characteristic found!");
      Serial.println("Writing insomnia deactivation password");
      insomniaCharacteristic.writeValue((char*)"Snooze");
    }
  }
  p.disconnect();
}

void setWipeCardP(BLEDevice p){
  if (p) {
    p.connect();
    p.discoverService("C104");

    // send the password over
    BLECharacteristic wipeCharacteristic = p.characteristic("0210");

    // subscribe to the insomnia characteristic
    Serial.println("Finding Wipe SD Card characteristic ...");
    if (!wipeCharacteristic) {
      Serial.println("no Wipe SD Card characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Wipe SD Card characteristic found!");
      Serial.println("Writing Wipe SD Card activation password");
      wipeCharacteristic.writeValue((char*)"wipe");
    }
  }
  p.disconnect();
}

void setRecordKeys(BLEDevice p){
  if (p) {
    p.connect();
    p.discoverService("C104");

    // send the password over
    BLECharacteristic recordCharacteristic = p.characteristic("0210");

    // subscribe to the insomnia characteristic
    Serial.println("Finding Record Keys characteristic ...");
    if (!recordCharacteristic) {
      Serial.println("no Record Keys characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Record keys characteristic found!");
      Serial.println("Writing Record activation password");
      recordCharacteristic.writeValue((char*)"Record");
    }
  }
  p.disconnect();
}

void stopRecordKeys(BLEDevice p){
  if (p) {
    p.connect();
    p.discoverService("C104");

    // send the password over
    BLECharacteristic recordCharacteristic = p.characteristic("0210");

    // subscribe to the insomnia characteristic
    Serial.println("Finding Record Keys characteristic ...");
    if (!recordCharacteristic) {
      Serial.println("no Record Keys characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Record keys characteristic found!");
      Serial.println("Writing Record deactivation password");
      recordCharacteristic.writeValue((char*)"Stop");
    }
  }
  p.disconnect();
}

void setMode(BLEDevice p, byte mode){
  if (p) {
    p.connect();
    p.discoverService("C103");

    // send the password over
    BLECharacteristic modeCharacteristic = p.characteristic("0110");

    // subscribe to the mode characteristic
    Serial.println("Finding Mode characteristic ...");
    if (!recordCharacteristic) {
      Serial.println("no Mode characteristic found!");
      p.disconnect();
      return;
    } else {
      Serial.println("Mode characteristic found!");
      Serial.println("Writing Mode value");
      modeCharacteristic.writeValue((mode);
    }
  }
  p.disconnect();
}
