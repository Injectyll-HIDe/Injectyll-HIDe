# C2 Overview

This document will cover the initial set up of the C2 interface, as well as the functionality of the C2 features.
## Initial C2 setup

This section will cover the initial setup procedure of the C2 after the C2 radio has been configured.

### Required Hardware
- Configured radio for C2 (see radio guide for details)
- USB Interface for C2
  - Here's a Sparkfun interface for the TH Digi radio: https://www.sparkfun.com/products/11697
- Host to run C2 on

### Software Installation
1) Install Python on your host if it isn't already (https://www.python.org/downloads/)
2) Launch a CLI
3) Download the necessary libraries
```bash
  pip install digi-xbee
  pip install blessed

```
4) Download the ihide.py code from the repository
5) Open the code in a text editor and change the passwords for the project as desired
6) Connect the C2 radio and its USB Dongle to the computer
7) Use Python to launch the C2 program
```bash
   python ihide.py

```
8) Locate the USB interface for the radio
9) Select the appropriate baud rate for the radio
10) Scan for available radios (option #2) and wait 10 seconds for the scan to finish
11) If the interface lists a radio, you know that you have established a connection. If you do not see a radio listed, try double checking the radio configurations.
## Basic Operation

There is a general procedure when first running the C2.

1) Launch the C2
2) Pick the desired USB radio interface from the list
3) Pick the baud rate for the radio interface
4) If a device scan has never been conducted or if additional devices have been added, run a scan to update the target list.
5) Issue an enable command to any devices that you want to interact with.
6) Enjoy!


## Features

1) List available targets
<pre>
    This function lists the information from all previously discovered radios on the last scan```
</pre>
2) Scan for Implants
<pre>
    This function will start a 10 second scan in which the radio will record all discovered devices in the same network```
</pre>
3) Activate Implants
<pre>
    This function tells the implants to accept commands from the C2. This needs to be done prior to any other commands.
</pre>
4) Get implant information
<pre>
    This function will get the time since last detected key press, the Key Recording toggle status, and the Insomnia mode toggle status for the specified device.
</pre>
5) Sniff keystrokes
<pre>
    This function allows for live sniffing of keystrokes from the selected device and will save a local copy of the keystrokes to the C2 host.
</pre>
6) Enable Insomnia mode
<pre>
    This function turns on a mouse jiggler function that prevents the target from entering a sleep mode due to inactivity.
</pre>
7) Disable Insomnia mode
<pre>
    This function turns off the mouse jiggler function on the selected target(s).
</pre>
8) Enable Keystroke Recorder
<pre>
This function turns on the keystroke recording functionality for the selected targets.  A file will be saved to the memory on the selected implant device.
</pre>
9) Disable Keystroke Recorder
<pre>
    This function disables the keystroke recording functionality for the selected targets.
</pre>
10) Download keystroke record
<pre>
    This function will allow you to select a target and a keystroke record to download from the storage of the implant to the C2 host.
</pre>
11) Go Dark mode
<pre>
    This function will disable all enabled functionality and wipe the memory of the selected implants.
</pre>
12) Get file list from target
<pre>
    This function lists the contents of the memory from the selected implant device.
</pre>
13) Receive data file
<pre>
    This function launches a data exfil script that will copy the specified document from the target to the C2 host.
</pre>
14) Launch injection script
<pre>
    This function lists the stored injection scripts and allows the user to choose one to launch on the target.
</pre>
15) Send injection script
<pre>
    This function pushes the selected injection script from the C2 host to the memory of the selected implant(s).
</pre>
16) Connect to Interactive Powershell Terminal
<pre>
    This function allows the user to connect to a reverse shell that has been launched from a previous injection script.
</pre>
17) Delete a file on the target implant
<pre>
    This function allows the user to delete the desired file from the memory of the selected implant.
</pre>
18) Reset target implant
<pre>
    This function allows the attacker to issue a software reset command to the selected devices if it is believed that they are not behaving as intended.
</pre>
19) Exit
<pre>
    This function exits the C2.
</pre>