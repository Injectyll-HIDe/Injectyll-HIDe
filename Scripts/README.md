# Key Parsing Scripts
1) US_kb_verbose.py: 
    This script takes the keys.txt keystroke record file and adds the verbose output next to each packet so you can see what is happening.
    This script assumes the keys recorded are using US keyboard encoding.
2) US_kb_output.py:
    This script takes the keys.txt keystroke record file and interprets the key packets to give a key press output like you would expect to see in a text file.
    This script assumes the keys recorded are using US keyboard encoding.

# Injection Scripts
This document covers a brief tutorial on writing a Ducky style script, as well as listing the currently available scripts with a brief description of functionality.

## Script Tutorial  
Scripting for Injectyll-HIDe using a Rubber Ducky style syntax. Give one command per line.
- Delay: dl- \<Time Value Here\>
- Key Press: ps- \<KEY\>
- Key Stream: pt-\<string of keys\>
- Release All Keys: ra- 

## Currently Available Scripts
Find our scripts in our script repo:
https://github.com/Injectyll-HIDe/Injection_Scripts
