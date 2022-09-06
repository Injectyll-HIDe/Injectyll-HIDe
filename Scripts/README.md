# Injection Scripts
This document covers a brief tutorial on writing a Ducky style script, as well as listing the currently available scripts with a brief description of functionality.

## Script Tutorial  
Scripting for Injectyll-HIDe using a Rubber Ducky style syntax. Give one command per line.
- Delay: dl- \<Time Value Here\>
- Key Press: ps- \<KEY\>
- Key Stream: pt-\<string of keys\>
- Release All Keys: ra- 

## Currently Available Scripts

1) reverse-shell.txt
> This script uses PowerShell to launch a reverse shell through the COM port, back to the C2. The shell will exist as a background process until it is terminated.  Once launched, the user will have to select the option in the C2 to connect to the open session.
2) Coming soon...