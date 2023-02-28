#!/usr/bin/env python3

#logic for parsing
#check packet for modifier
#if mod is shift and new key value added, add upper case value
#if not mod, add lower case value
#ignore key if found in next packet
#get rid of key if the value is no longer present in incoming array

#1) key packet read into pressArray
#2) pressArray is compared to prevArray to see if value is not previously present
#3) if key not previously present, modifier is checked for appropriate value and printed if key is previously seen, it is ignored
#4) current packet is saved to prevArray

pressArray = ['K','0', '0', '0', '0', '0', '0', '0', '0']
newArray = []
keyArray = []

def getMod(m):
    output = ""
    if (m > 64):
        output += "[RtAlt]"
        m -= 64
    elif m == 64:
        output += "[RAlt]"
        m -= 64
    if m > 32:
        output += "[RtShft]"
        m -= 32
    elif m == 32:
        output += "[RtShft]"
        m -= 32
    if m > 16:
        output += "[RtCtrl]"
        m -= 16
    elif m == 16:
        output += "[RtCtrl]"
        m -= 16
    if m > 8:
        output += "[LtGUI]"
        m -= 8
    elif m == 8:
        output += "[LtGUI]"
        m -= 8
    if m > 4:
        output += "[LtAlt]"
        m -= 4
    elif m == 4:
        output += "[LtAlt]"
        m -= 4
    if m > 2:
        output += "[LtShft]"
        m -= 2
    elif m == 2:
        output += "[LtShft]"
        m -= 2
    if m == 1:
        output += "[LtCtrl]"
    return output

def getShift(m):
    shiftStatus = False
    if (m > 64):
        m -= 64
    elif m == 64:
        m -= 64
    if m > 32:
        shiftStatus = True
        m -= 32
    elif m == 32:
        shiftStatus = True
        m -= 32
    if m > 16:
        m -= 16
    elif m == 16:
        m -= 16
    if m > 8:
        m -= 8
    elif m == 8:
        m -= 8
    if m > 4:
        m -= 4
    elif m == 4:
        m -= 4
    if m > 2:
        m -= 2
    elif m == 2:
        shiftStatus = True
        m -= 2
    if m == 1:
        shiftStatus = False
    return shiftStatus


getLowerKey = dict([("0", str("")),
               ("1", "Key1NotFound"),
               ("2", "Key2NotFound"),
               ("3", "Key3NotFound"),
               ("4", "a"),
               ("5", "b"),
               ("6", "c"),
               ("7", "d"),
               ("8", "e"),
               ("9", "f"),
               ("10", "g"),
               ("11", "h"),
               ("12", "i"),
               ("13", "j"),
               ("14", "k"),
               ("15", "l"),
               ("16", "m"),
               ("17", "n"),
               ("18", "o"),
               ("19", "p"),
               ("20", "q"),
               ("21", "r"),
               ("22", "s"),
               ("23", "t"),
               ("24", "u"),
               ("25", "v"),
               ("26", "w"),
               ("27", "x"),
               ("28", "y"),
               ("29", "z"),
               ("30", "1"),
               ("31", "2"),
               ("32", "3"),
               ("33", "4"),
               ("34", "5"),
               ("35", "6"),
               ("36", "7"),
               ("37", "8"),
               ("38", "9"),
               ("39", "0"),
               ("40", "\r"), #Enter
               ("41", "Esc"),
               ("42", "BkSpc"),
               ("43", "Tab"),
               ("44", " "), #SpaceBar
               ("45", "-"),
               ("46", "="),
               ("47", "["),
               ("48", "]"),
               ("49", "'\'"),
               ("50", "Key50NotMapped"),
               ("51", ";"),
               ("52", "'"),
               ("53", "`"),
               ("54", ","),
               ("55", "."),
               ("56", "/"),
               ("57", "CapsLck"),
               ("58", "F1"),
               ("59", "F2"),
               ("60", "F3"),
               ("61", "F4"),
               ("62", "F5"),
               ("63", "F6"),
               ("64", "F7"),
               ("65", "F8"),
               ("66", "F9"),
               ("67", "F10"),
               ("68", "F11"),
               ("69", "F12"),
               ("70", "PrntScrn"),
               ("71", "ScrnLck"),
               ("72", "Key72NotMapped"),
               ("73", "Ins"),
               ("74", "Home"),
               ("75", "PgUp"),
               ("76", "Del"),
               ("77", "End"),
               ("78", "PgDn"),
               ("79", "ArwR"),
               ("80", "ArwL"),
               ("81", "ArwD"),
               ("82", "ArwU"),
               ("83", "NumLck"),
               ("84", "Key84NotMapped"),
               ("85", "Key85NotMapped"),
               ("86", "Key86NotMapped"),
               ("87", "Key87NotMapped"),
               ("88", "Key88NotMapped"),
               ("89", "Key89NotMapped"),
               ("90", "Key90NotMapped"),
               ("91", "Key91NotFound"),
               ("92", "Key92NotFound"),
               ("93", "Key93NotFound"),
               ("94", "Key94NotFound"),
               ("95", "Key95NotFound"),
               ("96", "Key96NotFound"),
               ("97", "Key97NotFound"),
               ("98", "Key98NotFound"),
               ("99", "Key99NotFound"),
               ("100", "Key100NotFound"),
               ("101", "RtGUI")])


getUpperKey = dict([("0", str("")),
               ("1", "Key1NotFound"),
               ("2", "Key2NotFound"),
               ("3", "Key3NotFound"),
               ("4", "A"),
               ("5", "B"),
               ("6", "C"),
               ("7", "D"),
               ("8", "E"),
               ("9", "F"),
               ("10", "G"),
               ("11", "H"),
               ("12", "I"),
               ("13", "J"),
               ("14", "K"),
               ("15", "L"),
               ("16", "M"),
               ("17", "N"),
               ("18", "O"),
               ("19", "P"),
               ("20", "Q"),
               ("21", "R"),
               ("22", "S"),
               ("23", "T"),
               ("24", "U"),
               ("25", "V"),
               ("26", "W"),
               ("27", "X"),
               ("28", "Y"),
               ("29", "Z"),
               ("30", "!"),
               ("31", "@"),
               ("32", "#"),
               ("33", "$"),
               ("34", "%"),
               ("35", "^"),
               ("36", "&"),
               ("37", "*"),
               ("38", "("),
               ("39", ")"),
               ("40", "\r"), #Enter
               ("41", "Esc"),
               ("42", "BkSpc"),
               ("43", "Tab"),
               ("44", " "), #SpaceBar
               ("45", "_"),
               ("46", "+"),
               ("47", "{"),
               ("48", "}"),
               ("49", "|"),
               ("50", "Key50NotMapped"),
               ("51", ","),
               ("52", '"'),
               ("53", "~"),
               ("54", "<"),
               ("55", ">"),
               ("56", "?"),
               ("57", "CapsLck"),
               ("58", "F1"),
               ("59", "F2"),
               ("60", "F3"),
               ("61", "F4"),
               ("62", "F5"),
               ("63", "F6"),
               ("64", "F7"),
               ("65", "F8"),
               ("66", "F9"),
               ("67", "F10"),
               ("68", "F11"),
               ("69", "F12"),
               ("70", "PrntScrn"),
               ("71", "ScrnLck"),
               ("72", "Key72NotMapped"),
               ("73", "Ins"),
               ("74", "Home"),
               ("75", "PgUp"),
               ("76", "Del"),
               ("77", "End"),
               ("78", "PgDn"),
               ("79", "ArwR"),
               ("80", "ArwL"),
               ("81", "ArwD"),
               ("82", "ArwU"),
               ("83", "NumLck"),
               ("84", "Key84NotMapped"),
               ("85", "Key85NotMapped"),
               ("86", "Key86NotMapped"),
               ("87", "Key87NotMapped"),
               ("88", "Key88NotMapped"),
               ("89", "Key89NotMapped"),
               ("90", "Key90NotMapped"),
               ("91", "Key91NotFound"),
               ("92", "Key92NotFound"),
               ("93", "Key93NotFound"),
               ("94", "Key94NotFound"),
               ("95", "Key95NotFound"),
               ("96", "Key96NotFound"),
               ("97", "Key97NotFound"),
               ("98", "Key98NotFound"),
               ("99", "Key99NotFound"),
               ("100", "Key100NotFound"),
               ("101", "RtGUI")])

def exists(k,input_list):
    x=list(map(str,input_list))
    y="-".join(x)
    if y.find("15") !=-1:
        print("Found ",k)
    else:
        print(k, " is not found")

if __name__ == '__main__':
    inputFile = input("Enter the full path to your input file: ")
    outputFile = input("Enter the full path to your output file: ")
    oF = open(outputFile, "a")
    iF = open(inputFile, "r")
    Lines = iF.readlines()
    for line in Lines:
        try:
            #clear newArray for writing incoming keystrokes
            if newArray:
                newArray.clear()
                print("clearing new array")
            
            #populate array with keystroke packet values
            line = line.strip()
            newArray = line.split(',')
            oF.write(str(newArray)+"    ")
            newArray.pop(0)
            
            #read in keyvalues, map them, add them to new array, then print entire array, then clear array
            for s in range(len(newArray)-1):
                if s == 0: #if modifier
                    if newArray[0] != "0":
                        intMod = int(newArray[0])
                        keyArray.append(getMod(intMod))
                    else:
                        pressArray[s] = "0"      
                else: #keypress instead of modifier
                    if newArray[s] != "0":
                        keyArray.append(getLowerKey[newArray[s]])
            if len(keyArray) > 0:
                oF.write(str(keyArray)+"\r")
            else:
                oF.write("<All Keys Released>\r")
            newArray.clear()
            keyArray.clear()
        except (Exception) as e:
            print(e)
        except (KeyboardInterrupt) as k:
            iF.close()
            oF.close()
    oF.close()
    iF.close()
