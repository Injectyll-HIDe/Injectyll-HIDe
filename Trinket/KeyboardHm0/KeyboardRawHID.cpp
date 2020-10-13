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

#include "KeyboardRawHID.h"

void KeyboardRawHID::Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf)
{
  // Run parent class method so keyboard LEDs are updated.
  KeyboardReportParser::Parse(hid, is_rpt_id, len, buf);
  // Send a copy of the keyboard HID report out Serial1 as ASCII text
  // terminated with newline.
  if (len > 7) {
    char hidReport[64];
    int hidReportLen;
    hidReportLen = snprintf(hidReport, sizeof(hidReport),
        "K,%d,%d,%d,%d,%d,%d,%d,%d\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    if ((hidReportLen > 0) && (hidReportLen < sizeof(hidReport))) {
      Serial1.write(hidReport, hidReportLen);
    }
  }
}