![alt text](https://github.com/mistepien/joy2usb/blob/main/top.svg)
![alt text](https://github.com/mistepien/joy2usb/blob/main/bottom.svg)

# joy2usb
Atari-like joystick adapter to USB.

<a href="https://github.com/mistepien/joy2usb/blob/main/joy2usb.pdf">Adapter</a> is actually a shield for 
<a href="https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/hardware-overview-pro-micro">Arduino Pro Micro based on ATmega 32U4</a>.

ATmega 32U4 has a hardware support for fullspeed USB, thus <a href="https://wiki.archlinux.org/title/mouse_polling_rate">polling rate 1000Hz</a> is not an issue for that chip.

To order a PCB you can use <a href="https://github.com/mistepien/joy2usb/blob/main/gerber.zip">gerber file<a>.

Attached code (so-called sketches *ino) uses port registers so that is quite efficient, debouncing is done without delay() function. In  <a href="https://github.com/mistepien/joy2usb/blob/main/firmware/joy2usb.ino">joy2usb.ino</a> all four UP/DOWN/LEFT/RIGHT switches 
are debounced separatelly, while in in <a href="https://github.com/mistepien/joy2usb/blob/main/firmware/joy2usb_simple.ino">joy2usb_simple.ino</a> rather axises are debounced, not UP/DOWN/LEFT/RIGHT switches what results in slightly faster and shorter code.

The adapter has a toggle C64/Amiga for choosing C64 or Amiga approach for FIRE2/FIRE3. According to <a href="http://wiki.icomp.de/wiki/DE-9_Joystick"> wiki.icomp.de website about DE-9 joystick</a>, FIRE2/FIRE3 are pulled to VCC in case of C64 and to GND in case of Amiga. That is not an issue for joy2usb. Switch have to be toggled when adapter is off. Amiga approach is default -- if in your joystick FIRE2/FIRE3 are pulled to GND, and C64 approach is redundant for you than you do not need to solder <b>U2, C1, C2, R1, R2, R3, D1</b> and <b>SW2</b>.

Two footprints are not trivial: DB9 male socket (<b>J1</b>) and SPDT switch (<b>SW2</b>).

Officialy the <b>J1</b> footprint (joy2usb has been designed in <a href="https://www.kicad.org/">KiCad</a>) is "DSUB-9_Male_Horizontal_P2.77x2.84mm_EdgePinOffset9.90mm_Housed_MountingHolesOffset11.32mm"
however it was chosen to fit <a href="https://www.tme.eu/pl/en/details/ld09p13a4gx00lf/d-sub-plugs-and-sockets/amphenol-communications-solutions/">   LD09P13A4GX00LF Amphenol</a> (since PCB rev. 2.3 oval pads version of this footprint is used).

<b>SW2</b> supposed to be <a href="https://www.tme.eu/pl/en/details/mfp1220/slide-switches/knitter-switch/mfp-1220">Switch SPDT MFP 1220 KNITTER-SWITCH</a>, but it can be replaced with Pinheader 1x3/1x2 (pitch terminal 2.54mm) + jumper.

<b>J2</b> is redundant -- you can use it to add some new features like autofire switch or sth.

Sometimes during uploading code to Arduino board you may face some issues -- then you will need to use RESET -- so <b>SW1</b> can be useful if you like to develope your own code.

BOM:
| Qty	| Reference(s) | Description |
|-----|--------------|-------------|
|2 | C1, C2	| ceramic capacitors 100nF |
|1 | D1 | LED 3mm |  
|1 |	J1|	DB9 Male Connector <a href="https://www.tme.eu/pl/en/details/ld09p13a4gx00lf/d-sub-plugs-and-sockets/amphenol-communications-solutions/">   LD09P13A4GX00LF Amphenol</a> |
|1 |	J2|	2x4 Pinheader. pitch terminal: 2.54mm |
|1 | R1 | resistor for D1 |
|2 | R2, R3	| resistors 10KΩ |
|1 | SW1 | Tact switch from Arduino breadboard projects |
|1 | SW2 | <a href="https://www.tme.eu/pl/en/details/mfp1220/slide-switches/knitter-switch/mfp-1220">Switch SPDT MFP 1220 KNITTER-SWITCH</a> |
|1 | U1 |	DIP20 socket (width 15.24mm) + <a href="https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/hardware-overview-pro-micro">Arduino Pro Micro</a> |
|1 |	U2	| DIP14 socket + IC 4066	 |



