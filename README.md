# HIDe




///Diagram///

Keyboard -> OTG -> Trinket M0 -> via Serial -> BLEPeripheral (BLEp) -> via USB -> Victim Computer

                     | 

                     --> via BLE -> BLECentral (BLEp) -> via UART -> XbeeBLE -> via Zigbee -> XbeeRepeater -> via Zigbee -> XbeeC2

                                                              |

                                                          --> XbeeC2

///Phase 1 - Key Sniffing///

    [ ] Get UART connection between Xbee and BLEC

    [ ] Program BLEc to passthrough data to Xbee

    [ ] Program XbeeBLE to translate packets and relay to XbeeC2

    [ ] Program XbeeC2 to send commands

    [ ] Program XbeeBLE to pass commands through to BLEc

    [ ] Program BLEc to pass commands through to BLEp

///Phase 2 - Key Injection///

    [ ] method to return file list to c2

    [ ] set Attack file function to load up attack script

    [ ] launch attack (with confirmation), may be combined with above if choosing from list

    [ ] function to parse script into keyboard commands

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

