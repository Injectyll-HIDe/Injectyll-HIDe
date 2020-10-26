# HIDe




///Diagram///

Keyboard -> OTG -> Trinket M0 -> via Serial -> BLEPeripheral (BLEp) -> via USB -> Victim Computer

                     | 

                     --> via BLE -> BLECentral (BLEp) -> via UART -> XbeeBLE -> via Zigbee -> XbeeRepeater -> via Zigbee -> XbeeC2

                                                              |

                                                          --> XbeeC2

///Phase 1 - Key Sniffing///

    [x] Get UART connection between Xbee and BLEC

    [x] Program BLEc to passthrough data to Xbee

    [x] Program XbeeBLE to translate packets and relay to XbeeC2

    [x] Program XbeeC2 to send commands

    [x] Program XbeeBLE to pass commands through to BLEc

    [x] Program BLEc to pass commands through to BLEp

///Phase 2 - Key Injection///

    [x] method to return file list to c2

    [ ] set Attack file function to load up attack script

    [ ] launch attack (with confirmation), may be combined with above if choosing from list

    [x] function to parse script into keyboard commands

    [ ] lockout usb passthrough function until injection complete

    [ ] command menu to interact with options

///Phase 3 - Data Exfil///

    [ ] Add SD card function to BLECentral

    [ ] Add wipeCard to BLECentral

    [ ] method to chose file path to copy or dir to copy from

    [ ] method to write target data to local SD card on BLEP

    [ ] method to transfer data over to BLEC

    [ ] method on BLC to copy data to file

    [ ] method to relay file to C2

    [ ] C2 function to save file locally

///Phase 4 - Xbee Recorder///

    ---Hurdle to this is that SPI is documented but still under development for XBee things board---

    ---and so it is not actually implemented in the firmware from my understanding---

    [ ] set up SD card functionality with XBee

    [ ] method to trigger key stroke tx automatically

    [ ] record live stream to SD card

    [ ] incorporate battery with XBee Things board


///Other ideas///
  [ ] add ability to add attack scripts from C2 to the SD card on the BLEp
  [ ] Set C2 to auto-populate from an attack script file saved locally instead of hardcoding list in C2
