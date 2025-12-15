# webasto.md

## RS485 pinout from controller PCB

## PCB connector to cable:

- GND	= black
- A		= brown (signal+)
- B		= orange (signal-)
- 12V 	= red

Note that the shield of the cable goes to a yellow/green with a
"O" terminal that is screwed to the controller box common ground.

I cut the cable and put a 4 pin pheonix in with the four
signals, reconnecting the shield cable. The shield does not
cross to the other side of the pheonix.

## cable to USB RS485 Dongle

Pheonix connecter to cable to dongle

- BLACK + SHIELD -> dongle GND
- BROWN (signal+) -> dongle TX+
- ORANGE (signal-) -> dongle TX-

Note that the extension cable uses the dongles ground
for the shield and that I didn't have the correct colors
for dupont wires so they don't match.


## Signal Analysis.


With logic analyzer confirmed its basically 9600 baud 8N1.

There are two packets per frame.  A packet is 19 bytes.
The PCB sends one and the panel sends one.  Here are some
representative frames with the two packets per line:

7e 7e 00 ff 11 0d 10 09 82 00 38 42 00 00 02 00 7d 00 7d
7e 7e ff 00 11 0d 00 00 82 00 3f 1d 42 3f 08 00 00 00 36

7e 7e 00 ff 11 0d 10 09 82 00 38 42 00 00 02 00 7e 00 7e
7e 7e ff 00 11 0d 00 00 82 00 3f 1d 42 3f 08 00 00 00 36

7e 7e 00 ff 11 0d 10 09 82 00 38 42 00 00 02 00 7d 00 7d
7e 7e ff 00 11 0d 00 00 82 00 3f 1d 42 3f 08 00 00 00 36

7e 7e 00 ff 11 0d 10 09 82 00 38 42 00 00 02 00 7e 00 7e
7e 7e ff 00 11 0d 00 00 82 00 3f 1d 42 3f 08 00 00 00 36

The idea is that
- packets start with 7e7e
- one direction is followed by 00ff, the other ff00.
- have been thinking the PCB (00ff) goes first and the panel (ff00) responds


### Experiment; disconnect panel and see what we get

I know yesterday I got some smaller packets, so here's the entire initial
output from the PCB upon power up:

7e 7e 00 ff 11 0d 10 09 82 00 3f 44 00 00 02 00 7d 00 7c
7e 7e 88 ff 11 04 10 09 82 f9
7e 7e 00 ff 11 0d 10 09 82 00 3f 44 00 00 02 00 7e 00 7f
7e 7e 88 ff 11 04 10 09 82 f9
7e 7e 00 ff 11 0d 10 09 82 00 3f 44 00 00 02 00 7e 00 7f
7e 7e 88 ff 11 04 10 09 82 f9

- This seems to confirm that the PCB is 00ff as thought earlier and
- There seems to be an 88ff packet (i.e. bus query?)
- I am thinking the last byte is a checksum

							  v candidate for ambient temperature
7e 7e 00 ff 11 0d 10 09 82 00 41 44 00 00 02 00 7e 00 01
7e 7e ff 00 11 0d 00 00 82 00 3f 1d 44 3f 08 00 00 00 30


recent values before starting generator.

PCB 11 0d 10 09 82 3f 2f 44 c8 00 02 00 79 00 9f            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2f 44 c8 00 02 00 79 00 9f            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 c8 00 02 00 79 00 9e            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2e 44 a8 00 02 00 79 00 fe            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 44 a8 00 02 00 79 00 fd            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38

baseline:

PCB 11 0d 10 09 82 3f 2d 43 a8 00 02 00 79 00 fa            PAN 11 0d 00 00 82 ff 3f 1b 44 ce 08 00 00 00 38
PCB 11 0d 10 09 82 3f 2d 43 a8 00 02 00 79 00 fa            PAN 11 0d 00 00 82 ff 3f 1b 43 ce 08 00 00 00 3f
PCB 11 0d 10 09 82 3f 2e 43 a8 00 02 00 79 00 f9            PAN 11 0d 00 00 82 ff 3f 1b 43 ce 08 00 00 00 3f

running off the genset for a few seconds:

BUS 11 04 10 09 82 f9 3f 45 00 00 02 00 81 00 81            PAN 11 0d 00 00 82 00 3f 1b 45 ce 08 00 00 00 c6
BUS 11 04 10 09 82 f9 3f 45 00 00 02 00 80 00 80            PAN 11 0d 00 00 82 00 3f 1b 45 ce 08 00 00 00 c6
BUS 11 04 10 09 82 f9 3f 45 00 00 02 00 80 00 80            PAN 11 0d 00 00 82 00 3f 1b 45 ce 08 00 00 00 c6

remove temperature sensor

PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 36 0a 00 00 42 00 81 00 b8            PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor




#---------------------------------------------------------------------
# power up after replacing sensor
#---------------------------------------------------------------------

BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 82 00 bc ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 00 00 02 00 81 00 bf ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor

# Airco restarts by itself

PCB 11 0d 10 09 82 3f 3a 42 00 00 02 00 81 00 bc ,xor,smb             PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 00 00 02 00 81 00 bc ,xor,smb             PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 18 00 02 00 80 00 a5 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 18 00 02 00 80 00 a5 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 18 00 02 00 81 00 a4 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 18 00 02 00 80 00 a5 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 18 00 02 00 80 00 a5 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7f 00 8a ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7f 00 8a ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7f 00 8a ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7f 00 8a ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7e 00 8b ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7e 00 8b ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 3a 42 c8 00 02 00 7e 00 8b ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 c8 00 02 00 7e 00 88 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 c8 00 02 00 7e 00 88 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 c8 00 02 00 7e 00 88 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 c8 00 02 00 7e 00 88 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 42 c8 00 02 00 7e 00 88 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor

# Soft Power down

PCB 11 0d 10 09 82 3f 2d 42 c8 00 02 00 7d 00 9f ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2d 42 c8 00 02 00 7d 00 9f ,xor                 PAN 11 0d 00 00 83 00 3f 15 42 3f 08 00 00 00 3f ,xor
PCB 11 0d 10 09 82 00 2d 42 08 00 02 00 80 00 9d ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 08 00 02 00 80 00 9d ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 08 00 02 00 80 00 9d ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 08 00 02 00 80 00 9d ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 08 00 02 00 80 00 9d ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 80 00 95 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 81 00 94 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2d 42 00 00 02 00 81 00 94 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2e 42 00 00 02 00 81 00 97 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 2e 42 00 00 02 00 81 00 97 ,xor                 PAN 11 0d 00 00 82 00 3f 15 42 3f 08 00 00 00 3e ,xor

# hard power down
# remove sensor
# hard power up


PCB 11 0d 10 09 82 00 39 0a 00 00 02 00 80 00 c9 ,xor                 PAN 11 0d 00 00 82 00 3f 15 40 3f 08 00 00 00 3c ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 08 00 00 00 3c ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 02 00 80 00 c9 ,xor                 PAN 11 0d 00 00 82 00 3f 15 28 3f 08 00 00 00 54 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 28 3f 08 00 00 00 54 ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 02 00 81 00 c8 ,xor                 PAN 11 0d 00 00 82 00 3f 15 28 3f 08 00 00 00 54 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 28 3f 08 00 00 00 54 ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 28 3f 0a 00 00 00 56 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 28 3f 0a 00 00 00 56 ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor

# stable

PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor
PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 82 00 3f 15 40 3f 0a 00 00 00 3e ,xor


# soft start to display F0 error code

PCB 11 0d 10 09 82 00 39 0a 00 00 42 00 80 00 89 ,xor,smb             PAN 11 0d 00 00 83 ff 3f 15 40 3f 0a 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 39 0a 00 00 42 00 80 00 b6 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 0a 00 00 00 c1 ,xor


# observations for further testing:

PAN OFFSET(10) changes from 0x08 to 0x0a with sensor removed.
PCB OFFSET(7)=intake temperature drops to 0x0a which would be -30C brrr

#---------------------------------------------------------------------
# power up again  gdafter replacing sensor
#---------------------------------------------------------------------


PCB 11 0d 10 09 82 00 3e 43 00 00 02 00 7f 00 78 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 40 3f 08 00 00 00 c3 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 40 3f 08 00 00 00 c3 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7f 00 47 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
BUS 11 04 10 09 82 f9 ,xor                                            PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor

# airco starts by itself

PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7f 00 47 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 00 00 02 00 7e 00 46 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 18 00 02 00 7d 00 5d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 18 00 02 00 7d 00 5d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 18 00 02 00 7e 00 5e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 18 00 02 00 7e 00 5e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 18 00 02 00 7e 00 5e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3e 43 c8 00 02 00 7d 00 8d ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3d 43 c8 00 02 00 7d 00 8e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3d 43 c8 00 02 00 7d 00 8e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3d 43 c8 00 02 00 7d 00 8e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3d 43 c8 00 02 00 7d 00 8e ,xor                 PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3c 43 c8 00 02 00 7d 00 8f ,xor,mxf             PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor
PCB 11 0d 10 09 82 3f 3c 43 c8 00 02 00 7d 00 8f ,xor,mxf             PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xor'
'
PCB 11 0d 10 09 82 3f 3c 43 c8 00 02 00 7d 00 8f ,xor,mxf             PAN 11 0d 00 00 82 ff 3f 15 43 3f 08 00 00 00 c0 ,xo


#--------------------------------------------------------
# fairly stable before mode changes
#--------------------------------------------------------
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor
PCB 11 0d 10 09 82 3f 2b 42 c8 00 02 00 7c 00 98 ,xor                 PAN 11 0d 00 00 82 ff 3f 15 42 3f 08 00 00 00 c1 ,xor

# mode change to dehumidify

PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2b 42 c8 00 02 00 7d 00 d8 ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor

# PCB did not change
# PAN(6) changed from 0x3f to 0x7e
# PAN(7)=setpoint changed from 0x15 to 0x18 (75F)
# after a few on/offs I turned it off for 30 seconds.

PCB 11 0d 10 09 82 7e 2c 42 c8 00 02 00 7e 00 dc ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2c 42 c8 00 02 00 7e 00 dc ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2c 42 c8 00 02 00 7e 00 dc ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2c 42 c8 00 02 00 7e 00 dc ,xor                 PAN 11 0d 00 00 82 ff 7e 18 42 3f 08 00 00 00 8d ,xor
PCB 11 0d 10 09 82 7e 2c 42 c8 00 02 00 7e 00 dc ,xor                 PAN 11 0d 00 00 83 00 7e 18 42 3f 08 00 00 00 73 ,xor
PCB 11 0d 10 09 82 00 2c 42 08 00 02 00 81 00 9d ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 08 00 02 00 81 00 9d ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 08 00 02 00 81 00 9d ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 08 00 02 00 81 00 9d ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 08 00 02 00 81 00 9d ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor <-- big click on pump relay
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 2c 42 00 00 02 00 81 00 95 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor

# there is no way to turn it on without it resuming operation?
# there is no fan mode.
# switch to heating mode

PCB 11 0d 10 09 82 00 36 42 00 00 02 00 81 00 8f ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 36 42 00 00 02 00 81 00 8f ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 36 42 00 00 02 00 81 00 8f ,xor                 PAN 11 0d 00 00 83 ff bd 10 42 ce 08 00 00 00 b6 ,xor
PCB 11 0d 10 09 82 bd 36 42 10 00 02 00 80 00 23 ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 36 42 00 00 02 00 81 00 8f ,xor                 PAN 11 0d 00 00 82 00 7e 18 42 3f 08 00 00 00 72 ,xor
PCB 11 0d 10 09 82 00 36 42 00 00 02 00 81 00 8f ,xor                 PAN 11 0d 00 00 83 ff ce 1e 42 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor
PCB 11 0d 10 09 82 ce 36 42 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 42 ce 08 00 00 00 ca ,xor

# PAN(6) changed from 0x7e to 0xbd one second, then changed to 0xce
# after a while it started heating.
# The heating setpoint is apparently stored separately as 0x30=0x86


PCB 11 0d 10 09 82 ce 37 43 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 00 00 02 00 81 00 41 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 18 00 02 00 80 00 58 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 18 00 02 00 80 00 58 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 18 00 02 00 80 00 58 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 18 00 02 00 80 00 58 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 18 00 02 00 80 00 58 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 98 00 02 00 7e 00 26 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 98 00 02 00 7e 00 26 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 38 43 98 00 02 00 7d 00 2a ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 38 43 98 00 02 00 7d 00 2a ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 38 43 98 00 02 00 7d 00 2a ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 38 43 98 00 02 00 7d 00 2a ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 38 43 98 00 02 00 7d 00 2a ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 37 43 98 00 02 00 7d 00 25 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor

Thats gross.  Its sending out hot humid wet air.
Here it is heating up the room, fairly stable:
The water pump is running.

PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 51 43 9c 00 02 00 7c 00 46 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7c 00 45 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor
PCB 11 0d 10 09 82 ce 52 43 9c 00 02 00 7d 00 44 ,xor                 PAN 11 0d 00 00 82 ff ce 1e 43 ce 08 00 00 00 cb ,xor


I am just going to switch it directly to auto and see which one it choses, or perhaps there is a different setpoint

Heating = 30C
Cooling = 21C
Dehumidify = 24C
Auto = ???


I will start at 25 to get it to cool down, then see what happens when I increase it.
It went into fan only mode after a few mode changes in a row.  It is now on auto and should start cooling
# PAN(6) changed from 0xce 0xdd.

PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 48 43 44 00 02 00 81 00 09 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 47 43 44 00 02 00 81 00 06 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 47 43 44 00 02 00 81 00 06 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 47 43 44 00 02 00 81 00 06 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 47 43 44 00 02 00 81 00 06 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 47 43 44 00 02 00 81 00 06 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor


Here's where it changed to actually cooling.  Taking some time.
I'm prtty sure PAN(6) is the driving "mode"

Here's about where it started cooling:

PCB 11 0d 10 09 82 bd 45 43 40 00 02 00 81 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 45 43 40 00 02 00 81 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 45 43 40 00 02 00 81 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 bd 45 43 40 00 02 00 81 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 18 43 3f 08 00 00 00 2f ,xor  <--- gd
PCB 11 0d 10 09 82 3f 45 43 48 00 02 00 81 00 8a ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 48 00 02 00 81 00 8a ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 48 00 02 00 81 00 8a ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 48 00 02 00 81 00 8a ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 48 00 02 00 81 00 8a ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 c8 00 02 00 7e 00 f5 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 c8 00 02 00 7e 00 f5 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 c8 00 02 00 7e 00 f5 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 45 43 c8 00 02 00 7e 00 f5 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 44 43 c8 00 02 00 7d 00 f7 ,xor,mx0             PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 44 43 c8 00 02 00 7d 00 f7 ,xor,mx0             PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 44 43 c8 00 02 00 7d 00 f7 ,xor,mx0             PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 44 43 c8 00 02 00 7d 00 f7 ,xor,mx0             PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 44 43 c8 00 02 00 7d 00 f7 ,xor,mx0             PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor
PCB 11 0d 10 09 82 3f 43 43 c8 00 02 00 7d 00 f0 ,xor                 PAN 11 0d 00 00 82 ff dd 19 43 3f 08 00 00 00 2e ,xor

it has coooled down to 27C=81F.   Now increasing auto setpoint to 28=82F

PCB 11 0d 10 09 82 3f 2b 42 48 00 02 00 7f 00 1b ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 48 00 02 00 7f 00 1b ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 48 00 02 00 7f 00 1b ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 48 00 02 00 7f 00 1b ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 7f 00 13 ,xor,mxf             PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 7f 00 13 ,xor,mxf             PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor <-- click
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 7f 00 13 ,xor,mxf             PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 80 00 ec ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 80 00 ec ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 80 00 ec ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 80 00 ec ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2b 42 40 00 02 00 7f 00 13 ,xor,mxf             PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2c 42 40 00 02 00 7f 00 14 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2c 42 40 00 02 00 7f 00 14 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 2c 42 40 00 02 00 7f 00 14 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor

auto and cool light remain on. fan running compressor off.

PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7e 00 00 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 39 42 40 00 02 00 7f 00 01 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7f 00 02 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7f 00 02 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7e 00 03 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7e 00 03 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7e 00 03 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 3f 3a 42 40 00 02 00 7e 00 03 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 42 3f 08 00 00 00 2a ,xor

cool light goes off;  heat light comes on?
it takes quite a while to switch between cooling and heating; maybe long enough so that ambient bocas will heate it up to 82
intake = 27C=81F; raising setpoint to 29C
Cooling light off somewhere in here:

PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 43 3f 08 00 00 00 2b ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 43 3f 08 00 00 00 2b ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1c 43 3f 08 00 00 00 2b ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor   <==
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor
PCB 11 0d 10 09 82 bd 3d 43 40 00 02 00 7e 00 87 ,xor                 PAN 11 0d 00 00 82 ff dd 1d 43 3f 08 00 00 00 2a ,xor





