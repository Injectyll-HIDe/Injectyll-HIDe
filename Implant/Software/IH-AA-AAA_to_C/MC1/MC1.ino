/*
MIT License
Copyright (c) 2018 gdsports625@gmail.com
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/*
 * USB host mode for SAMD
 * USB keyboard HID reports are sent out Serial1 as ASCII text terminated with newline.
 */

// Require keyboard control library
#include "KeyboardRawHID.h"

// on a zero with debug port, use debug port
//#define SerialDebug Serial

// on a feather or non-debug Zero, use Serial1 (since USB is taken!)
//#define SerialDebug if (0) Serial1

// Initialize USB Controller
USBHost usb;

// Attach keyboard controller to USB
KeyboardRawHID keyboard(usb);

uint32_t lastUSBstate = 0;

void setup()
{
  Serial.begin(115200);
  Serial1.begin( 115200 );
  Serial1.println("USB Host Keyboard Controller Program started");

  if (usb.Init() == -1) {
    Serial1.println("USB Host did not start.");
    while (1) delay(1);
  }

  Serial1.println("USB Host started");
  delay( 20 );
}

void loop()
{
  // Process USB tasks
  usb.Task();

  uint32_t currentUSBstate = usb.getUsbTaskState();
  if (lastUSBstate != currentUSBstate) {
    Serial1.print("USB state changed: 0x");
    Serial1.print(lastUSBstate, HEX);
    Serial1.print(" -> 0x");
    Serial1.println(currentUSBstate, HEX);
    switch (currentUSBstate) {
      case USB_ATTACHED_SUBSTATE_SETTLE: Serial1.println("Device Attached"); break;
      case USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE: Serial1.println("Detached, waiting for Device"); break;
      case USB_ATTACHED_SUBSTATE_RESET_DEVICE: Serial1.println("Resetting Device"); break;
      case USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE: Serial1.println("Reset complete"); break;
      case USB_STATE_CONFIGURING: Serial.println("USB Configuring"); break;
      case USB_STATE_RUNNING: Serial.println("USB Running"); break;
    }
    lastUSBstate = currentUSBstate;
  }
}
