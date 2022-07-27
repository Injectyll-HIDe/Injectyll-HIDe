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
