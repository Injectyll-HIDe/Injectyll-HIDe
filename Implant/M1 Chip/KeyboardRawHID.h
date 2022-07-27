/*
   Copyright (c) 2018 Arduino.  All right reserved.
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
   */

#ifndef KEYBOARD_RAW_HID_H
#define KEYBOARD_RAW_HID_H

#include <hidboot.h>
#include <usbhub.h>

class KeyboardRawHID : public KeyboardReportParser {
  public:
    KeyboardRawHID(USBHost &usb) : hostKeyboard(&usb) {
      hostKeyboard.SetReportParser(0, this);
    };

  protected:
    void Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf);

  private:
    HIDBoot<HID_PROTOCOL_KEYBOARD> hostKeyboard;
};

#endif