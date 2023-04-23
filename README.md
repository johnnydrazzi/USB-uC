# USB uC
![Alt text](Images/USB_uC_27J53_2.jpg?raw=true "USB_uC_27J53")    
A bootloader for PIC microcontrollers, that makes your USB capable PIC appear to your computer as a thumb drive. Programming can be done through MPLABX or by simply dragging and dropping your Intel hex file onto the drive. No programmer, drivers or software to install.

The USB stack has been compliance tested using USB-IF's USB 3 Command Verifier (USB3CV) tool.  

**Features:**
- Bootloaders made for popular development boards.
- Different crystal options, including NO_XTAL.
- Drag and drop programming or through MPLABX.
- Read user flash as a PROG_MEM.BIN file.
- Erase user flash by deleting PROG_MEM.BIN.
- Read and write to EEPROM through a EEPROM.BIN file.
- Erase EEPROM by deleting EEPROM.BIN.
  
**Currently supports:**<br>
PIC16F1459 Family:
- PIC16F1454
- PIC16F1455
- PIC16F1459

PIC18F4450 Family:
- PIC18F2450
- PIC18F4450

PIC18F4550 Family:
- PIC18F2455
- PIC18F4455
- PIC18F2550
- PIC18F4550

PIC18F4553 Family:
- PIC18F2458
- PIC18F4458
- PIC18F2553
- PIC18F4553

PIC18F1XK50 Family:
- PIC18F14K50

PIC18F45K50 Family:
- PIC18F24K50
- PIC18F25K50
- PIC18F45K50

PIC18F46J50 Family:
- PIC18F24J50
- PIC18F44J50
- PIC18F25J50
- PIC18F45J50
- PIC18F26J50
- PIC18F46J50

PIC18F47J53 Family:
- PIC18F26J53
- PIC18F46J53
- PIC18F27J53
- PIC18F47J53
  
For more information visit the [Project Page](https://hackaday.io/project/63204-usb-c) on Hackaday.io.
