/*
 * Things to keep in mind when implementing into Injectyll-HIDe:
 * need to make sure that the password for running the script is reset prior to trying to execute the script a second time.
 * need to add lockout in BLEp to block input keystrokes while running script
 */

#include <Keyboard.h>
#include <SPI.h>
#include <SD.h>

bool lockout = false;

void setup(){
  Keyboard.begin();
  Serial.begin(115200);
  Serial1.begin(115200);

  //Start SD card parameters
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

void loop(){
  if (lockout == false){  
    launchScript("Hello.txt");
    //delay(1000);
  }
}

void launchScript(String n) {
/* Keystroke Injection/Attack Script layout
*ps-<Key> = Keyboard.press(<KEY>) or Keyboard.press('<char>') 
*pt-<String> = Keyboard.print("<String>")
*ra- = Keyboard.releaseAll();
*dl-<# of milliseconds> = delay(<# of milliseconds>)
*/
  String path = "/Scripts/" + n;
  File myFile = SD.open(path);
  String line = "";
  if (myFile){
    while (myFile.available()){
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
  } else {
    myFile.close();
    Serial.println("There was an error reading the submitted script name");
  }
  Serial.println("File was launched successfully");
  lockout = true;
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
    Serial.println("There was an error with the script parsing.");
    Serial.println("Exiting Attack script.");
    return;
  }
  return;
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
