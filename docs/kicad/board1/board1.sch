EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR03
U 1 1 61DFC811
P 1600 1500
F 0 "#PWR03" H 1600 1250 50  0001 C CNN
F 1 "GND" H 1605 1327 50  0000 C CNN
F 2 "" H 1600 1500 50  0001 C CNN
F 3 "" H 1600 1500 50  0001 C CNN
	1    1600 1500
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR01
U 1 1 6717D3ED
P 1350 1100
F 0 "#PWR01" H 1350 950 50  0001 C CNN
F 1 "+12V" V 1350 1350 50  0000 C CNN
F 2 "" H 1350 1100 50  0001 C CNN
F 3 "" H 1350 1100 50  0001 C CNN
	1    1350 1100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 692E12E0
P 3950 1200
F 0 "#PWR02" H 3950 1050 50  0001 C CNN
F 1 "+5V" V 3950 1400 50  0000 C CNN
F 2 "" H 3950 1200 50  0001 C CNN
F 3 "" H 3950 1200 50  0001 C CNN
	1    3950 1200
	0    1    1    0   
$EndComp
Connection ~ 3150 1200
Wire Wire Line
	2750 1200 3150 1200
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 66D2CE52
P 2650 1400
F 0 "J2" V 2900 1350 50  0000 C CNN
F 1 "5V Jump" V 2800 1350 50  0000 C CNN
F 2 "0_my_footprints:myPinHeader_1x02" H 2650 1400 50  0001 C CNN
F 3 "~" H 2650 1400 50  0001 C CNN
	1    2650 1400
	0    -1   1    0   
$EndComp
$Comp
L Device:D D1
U 1 1 67022BE0
P 2500 1200
F 0 "D1" H 2500 983 50  0000 C CNN
F 1 "1N4003" H 2500 1075 50  0000 C CNN
F 2 "0_my_footprints:myDiodeSchotsky" H 2500 1200 50  0001 C CNN
F 3 "~" H 2500 1200 50  0001 C CNN
	1    2500 1200
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR05
U 1 1 6718228E
P 3150 1650
F 0 "#PWR05" H 3150 1400 50  0001 C CNN
F 1 "GND" H 3155 1477 50  0000 C CNN
F 2 "" H 3150 1650 50  0001 C CNN
F 3 "" H 3150 1650 50  0001 C CNN
	1    3150 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 671A6D4E
P 3150 1350
F 0 "C1" H 3268 1396 50  0001 L CNN
F 1 "100uf" H 3300 1350 50  0000 L CNN
F 2 "0_my_footprints2:CP_my100uf" H 3188 1200 50  0001 C CNN
F 3 "~" H 3150 1350 50  0001 C CNN
	1    3150 1350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 671A6D48
P 2350 1500
F 0 "#PWR04" H 2350 1250 50  0001 C CNN
F 1 "GND" H 2355 1327 50  0000 C CNN
F 2 "" H 2350 1500 50  0001 C CNN
F 3 "" H 2350 1500 50  0001 C CNN
	1    2350 1500
	1    0    0    -1  
$EndComp
$Comp
L cnc3018_Library:BUCK01 M1
U 1 1 67182287
P 2000 1350
F 0 "M1" H 2150 1350 50  0001 C CNN
F 1 "BUCK01" H 1900 1350 50  0000 C CNN
F 2 "0_my_footprints2:buck-mini360" H 1950 1650 50  0001 C CNN
F 3 "" H 1950 1650 50  0001 C CNN
	1    2000 1350
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 692F3A66
P 900 1200
F 0 "J1" H 900 1450 50  0000 C CNN
F 1 "12 IN" H 900 1350 50  0000 C CNN
F 2 "0_my_footprints:myPheonix2x3.81_right" H 900 1200 50  0001 C CNN
F 3 "~" H 900 1200 50  0001 C CNN
	1    900  1200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1100 1200 1350 1200
Wire Wire Line
	1100 1300 1350 1300
Wire Wire Line
	1350 1300 1350 1500
Wire Wire Line
	1350 1500 1600 1500
Connection ~ 1600 1500
Wire Wire Line
	1350 1100 1350 1200
Connection ~ 1350 1200
Wire Wire Line
	1350 1200 1600 1200
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 692FCF72
P 3850 1550
F 0 "J3" H 3850 1800 50  0000 C CNN
F 1 "5V Test" H 3850 1700 50  0000 C CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 3850 1550 50  0001 C CNN
F 3 "~" H 3850 1550 50  0001 C CNN
	1    3850 1550
	1    0    0    1   
$EndComp
Wire Wire Line
	3150 1200 3650 1200
Wire Wire Line
	3650 1450 3650 1200
Connection ~ 3650 1200
Wire Wire Line
	3650 1200 3950 1200
Wire Wire Line
	3650 1550 3150 1550
Wire Wire Line
	3150 1500 3150 1550
Connection ~ 3150 1550
Wire Wire Line
	3150 1550 3150 1650
$Comp
L Device:R R1
U 1 1 692C8E57
P 6200 1300
F 0 "R1" V 6300 1250 50  0000 L CNN
F 1 "1K" V 6200 1250 50  0000 L CNN
F 2 "0_my_footprints:myResistor" V 6130 1300 50  0001 C CNN
F 3 "~" H 6200 1300 50  0001 C CNN
	1    6200 1300
	0    1    -1   0   
$EndComp
Text GLabel 6550 1300 2    50   Input ~ 0
DRIVE_ROD
Text GLabel 5850 1300 0    50   Input ~ 0
S_DRIVE
Text GLabel 5850 1800 0    50   Input ~ 0
SENSOR
Text GLabel 6800 1800 2    50   Input ~ 0
SENSOR_ROD
$Comp
L Device:C C2
U 1 1 694827BA
P 6250 1950
F 0 "C2" H 6000 1850 50  0000 L CNN
F 1 "2.2uf" H 5950 1950 50  0000 L CNN
F 2 "0_my_footprints:myCapTantulum2" H 6288 1800 50  0001 C CNN
F 3 "~" H 6250 1950 50  0001 C CNN
	1    6250 1950
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 694827C0
P 5950 1950
F 0 "R3" H 6050 1950 50  0000 L CNN
F 1 "100K" V 5950 1950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5880 1950 50  0001 C CNN
F 3 "~" H 5950 1950 50  0001 C CNN
	1    5950 1950
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 694827CC
P 6500 1800
F 0 "R2" V 6600 1750 50  0000 L CNN
F 1 "1K" V 6500 1800 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6430 1800 50  0001 C CNN
F 3 "~" H 6500 1800 50  0001 C CNN
	1    6500 1800
	0    1    -1   0   
$EndComp
Wire Wire Line
	6650 1800 6800 1800
Wire Wire Line
	6350 1300 6550 1300
Wire Wire Line
	6050 1300 5850 1300
Text GLabel 8400 2100 0    50   Input ~ 0
DRIVE_ROD
Text GLabel 8400 2200 0    50   Input ~ 0
SENSOR_ROD
$Comp
L Connector_Generic:Conn_01x03 J6
U 1 1 69591300
P 8600 3400
F 0 "J6" H 8750 3300 50  0000 C CNN
F 1 "WS2812_B" H 8900 3400 50  0000 C CNN
F 2 "0_my_footprints:myJSTx03" H 8600 3400 50  0001 C CNN
F 3 "~" H 8600 3400 50  0001 C CNN
	1    8600 3400
	1    0    0    -1  
$EndComp
Text GLabel 8400 2750 0    50   Input ~ 0
PUMP+
Text GLabel 8400 2650 0    50   Input ~ 0
PUMP-
Text GLabel 7950 3400 0    50   Input ~ 0
WS2812B
$Comp
L power:GND #PWR014
U 1 1 695C7DB1
P 8400 3500
F 0 "#PWR014" H 8400 3250 50  0001 C CNN
F 1 "GND" V 8400 3300 50  0000 C CNN
F 2 "" H 8400 3500 50  0001 C CNN
F 3 "" H 8400 3500 50  0001 C CNN
	1    8400 3500
	0    1    -1   0   
$EndComp
$Comp
L power:+5V #PWR08
U 1 1 695D58B6
P 1400 2700
F 0 "#PWR08" H 1400 2550 50  0001 C CNN
F 1 "+5V" V 1400 2900 50  0000 C CNN
F 2 "" H 1400 2700 50  0001 C CNN
F 3 "" H 1400 2700 50  0001 C CNN
	1    1400 2700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR09
U 1 1 695EAB91
P 1500 2700
F 0 "#PWR09" H 1500 2450 50  0001 C CNN
F 1 "GND" V 1500 2500 50  0000 C CNN
F 2 "" H 1500 2700 50  0001 C CNN
F 3 "" H 1500 2700 50  0001 C CNN
	1    1500 2700
	1    0    0    1   
$EndComp
$Comp
L power:+3V3 #PWR020
U 1 1 695EF873
P 4150 5300
F 0 "#PWR020" H 4150 5150 50  0001 C CNN
F 1 "+3V3" V 4150 5500 50  0000 C CNN
F 2 "" H 4150 5300 50  0001 C CNN
F 3 "" H 4150 5300 50  0001 C CNN
	1    4150 5300
	1    0    0    1   
$EndComp
Text GLabel 2100 2700 1    50   Input ~ 0
S_DRIVE
Text GLabel 2700 2700 1    50   Input ~ 0
SENSOR
Text GLabel 2300 2700 1    50   Input ~ 0
WS2812B
$Comp
L Device:C C4
U 1 1 696B0578
P 1150 2850
F 0 "C4" H 850 2900 50  0000 L CNN
F 1 "0.1uf" H 800 2800 50  0000 L CNN
F 2 "0_my_footprints:myCapCeramicSmall" H 1188 2700 50  0001 C CNN
F 3 "~" H 1150 2850 50  0001 C CNN
	1    1150 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 696BF015
P 1150 3000
F 0 "#PWR012" H 1150 2750 50  0001 C CNN
F 1 "GND" V 1150 2800 50  0000 C CNN
F 2 "" H 1150 3000 50  0001 C CNN
F 3 "" H 1150 3000 50  0001 C CNN
	1    1150 3000
	-1   0    0    -1  
$EndComp
$Comp
L power:+5V #PWR015
U 1 1 6943AF93
P 8400 3300
F 0 "#PWR015" H 8400 3150 50  0001 C CNN
F 1 "+5V" V 8400 3500 50  0000 C CNN
F 2 "" H 8400 3300 50  0001 C CNN
F 3 "" H 8400 3300 50  0001 C CNN
	1    8400 3300
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R8
U 1 1 69443C96
P 8250 3400
F 0 "R8" V 8200 3550 50  0000 L CNN
F 1 "220" V 8250 3400 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8180 3400 50  0001 C CNN
F 3 "~" H 8250 3400 50  0001 C CNN
	1    8250 3400
	0    -1   1    0   
$EndComp
Wire Wire Line
	7950 3400 8100 3400
$Comp
L power:GND #PWR06
U 1 1 694827AE
P 6100 2100
F 0 "#PWR06" H 6100 1850 50  0001 C CNN
F 1 "GND" H 6105 1927 50  0000 C CNN
F 2 "" H 6100 2100 50  0001 C CNN
F 3 "" H 6100 2100 50  0001 C CNN
	1    6100 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 1800 5950 1800
Connection ~ 5950 1800
Wire Wire Line
	5950 1800 6250 1800
Connection ~ 6250 1800
Wire Wire Line
	6250 1800 6350 1800
Wire Wire Line
	5950 2100 6100 2100
Connection ~ 6100 2100
Wire Wire Line
	6100 2100 6250 2100
Wire Wire Line
	5550 3450 5650 3450
$Comp
L Device:R R9
U 1 1 694D6E85
P 5800 3450
F 0 "R9" V 5700 3400 50  0000 L CNN
F 1 "220" V 5800 3450 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5730 3450 50  0001 C CNN
F 3 "~" H 5800 3450 50  0001 C CNN
	1    5800 3450
	0    -1   1    0   
$EndComp
Text GLabel 5550 3450 0    50   Input ~ 0
PUMP
Wire Wire Line
	6350 3150 6350 3250
Connection ~ 6350 2850
Wire Wire Line
	6550 2850 6350 2850
Wire Wire Line
	6350 2750 6350 2850
Text GLabel 6550 2850 2    50   Input ~ 0
PUMP+
$Comp
L Diode:1N4001 D2
U 1 1 61BE277E
P 6350 3000
F 0 "D2" V 6250 3200 50  0000 L CNN
F 1 "1N5818" V 6350 3100 50  0000 L CNN
F 2 "0_my_footprints:myDiodeSchotsky" H 6350 2825 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 6350 3000 50  0001 C CNN
	1    6350 3000
	0    -1   1    0   
$EndComp
Text GLabel 2200 2700 1    50   Input ~ 0
PUMP
$Comp
L power:GND #PWR017
U 1 1 695E5C40
P 1500 3700
F 0 "#PWR017" H 1500 3450 50  0001 C CNN
F 1 "GND" V 1500 3500 50  0000 C CNN
F 2 "" H 1500 3700 50  0001 C CNN
F 3 "" H 1500 3700 50  0001 C CNN
	1    1500 3700
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J5
U 1 1 6931ED47
P 8600 2650
F 0 "J5" H 8750 2650 50  0000 C CNN
F 1 "PUMP" H 8800 2550 50  0000 C CNN
F 2 "0_my_footprints:myPheonix2x3.81_right" H 8600 2650 50  0001 C CNN
F 3 "~" H 8600 2650 50  0001 C CNN
	1    8600 2650
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET:IRLZ44N Q1
U 1 1 69A0DAE1
P 6250 3450
F 0 "Q1" H 6454 3496 50  0000 L CNN
F 1 "IRLZ44N" H 6454 3405 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-220-3_Vertical" H 6500 3375 50  0001 L CIN
F 3 "http://www.irf.com/product-info/datasheets/data/irlz44n.pdf" H 6250 3450 50  0001 L CNN
	1    6250 3450
	1    0    0    -1  
$EndComp
Connection ~ 6350 3150
Text GLabel 6550 3150 2    50   Input ~ 0
PUMP-
Wire Wire Line
	6550 3150 6350 3150
$Comp
L power:+12V #PWR013
U 1 1 61C07060
P 6350 2750
F 0 "#PWR013" H 6350 2600 50  0001 C CNN
F 1 "+12V" H 6365 2923 50  0000 C CNN
F 2 "" H 6350 2750 50  0001 C CNN
F 3 "" H 6350 2750 50  0001 C CNN
	1    6350 2750
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR021
U 1 1 693D2296
P 6350 3950
F 0 "#PWR021" H 6350 3700 50  0001 C CNN
F 1 "GND" H 6355 3777 50  0000 C CNN
F 2 "" H 6350 3950 50  0001 C CNN
F 3 "" H 6350 3950 50  0001 C CNN
	1    6350 3950
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R10
U 1 1 69417B11
P 6000 3700
F 0 "R10" V 5900 3650 50  0000 L CNN
F 1 "10K" V 6000 3700 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5930 3700 50  0001 C CNN
F 3 "~" H 6000 3700 50  0001 C CNN
	1    6000 3700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5950 3450 6000 3450
Wire Wire Line
	6000 3550 6000 3450
Connection ~ 6000 3450
Wire Wire Line
	6000 3450 6050 3450
Wire Wire Line
	6000 3850 6350 3850
Wire Wire Line
	6350 3850 6350 3650
Wire Wire Line
	6350 3850 6350 3950
Connection ~ 6350 3850
$Comp
L 0_my_symbols:SD_TYP0 M2
U 1 1 69A9CB24
P 4450 4800
F 0 "M2" V 4454 5078 50  0000 L CNN
F 1 "SD_TYP0" V 4545 5078 50  0000 L CNN
F 2 "0_my_footprints:SDCardReader_flipped" H 4250 4950 50  0001 C CNN
F 3 "" H 4250 4950 50  0001 C CNN
	1    4450 4800
	0    1    1    0   
$EndComp
Text GLabel 3650 4700 0    50   Input ~ 0
MISO
Text GLabel 3650 4800 0    50   Input ~ 0
SCLK
Text GLabel 3650 4900 0    50   Input ~ 0
MOSI
Text GLabel 3650 5000 0    50   Input ~ 0
SDCS
$Comp
L Device:R R11
U 1 1 69A9FE1F
P 4000 5100
F 0 "R11" V 4100 5050 50  0000 L CNN
F 1 "10K" V 4000 5100 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 3930 5100 50  0001 C CNN
F 3 "~" H 4000 5100 50  0001 C CNN
	1    4000 5100
	0    -1   1    0   
$EndComp
Wire Wire Line
	4150 5100 4150 5300
Connection ~ 4150 5100
$Comp
L power:GND #PWR018
U 1 1 69AABD2F
P 3650 4600
F 0 "#PWR018" H 3650 4350 50  0001 C CNN
F 1 "GND" V 3650 4400 50  0000 C CNN
F 2 "" H 3650 4600 50  0001 C CNN
F 3 "" H 3650 4600 50  0001 C CNN
	1    3650 4600
	0    1    -1   0   
$EndComp
Wire Wire Line
	4150 5000 3650 5000
Wire Wire Line
	3650 4900 4150 4900
Wire Wire Line
	4150 4800 3650 4800
Wire Wire Line
	3650 4700 3850 4700
Wire Wire Line
	4150 4600 3650 4600
Wire Wire Line
	3850 5100 3850 4700
Connection ~ 3850 4700
Wire Wire Line
	3850 4700 4150 4700
Text GLabel 2200 3700 3    50   Input ~ 0
SCLK
Text GLabel 2800 3700 3    50   Input ~ 0
MOSI
Text GLabel 2100 3700 3    50   Input ~ 0
SDCS
$Comp
L 0_my_symbols:ESP32_DEV_0 U2
U 1 1 69ACA152
P 2200 3250
F 0 "U2" H 1700 3400 50  0000 L CNN
F 1 "ESP32_DEV_0" H 1900 3400 50  0000 L CNN
F 2 "0_my_footprints:myESP32DEV0" H 1650 3350 50  0001 C CNN
F 3 "" H 1650 3350 50  0001 C CNN
	1    2200 3250
	1    0    0    -1  
$EndComp
Text GLabel 2300 3700 3    50   Input ~ 0
MISO
Wire Wire Line
	1400 2700 1150 2700
Connection ~ 1400 2700
$Comp
L power:+3V3 #PWR016
U 1 1 69B74055
P 1400 3700
F 0 "#PWR016" H 1400 3550 50  0001 C CNN
F 1 "+3V3" V 1400 3900 50  0000 C CNN
F 2 "" H 1400 3700 50  0001 C CNN
F 3 "" H 1400 3700 50  0001 C CNN
	1    1400 3700
	1    0    0    1   
$EndComp
Text GLabel 1900 3700 3    50   Input ~ 0
RX2
Text GLabel 2000 3700 3    50   Input ~ 0
TX2
Text GLabel 1700 3700 3    50   Input ~ 0
ALIVE_LED
$Comp
L 0_my_symbols:RS485_MODULE U1
U 1 1 69C21246
P 3900 2650
F 0 "U1" V 4500 2400 50  0000 L CNN
F 1 "RS485_MODULE" V 4500 2550 50  0000 L CNN
F 2 "0_my_footprints2:RS485_Module" H 3900 2750 50  0001 C CNN
F 3 "" H 3900 2750 50  0001 C CNN
	1    3900 2650
	0    1    1    0   
$EndComp
$Comp
L power:+3V3 #PWR07
U 1 1 69C30396
P 4350 2350
F 0 "#PWR07" H 4350 2200 50  0001 C CNN
F 1 "+3V3" H 4350 2550 50  0000 C CNN
F 2 "" H 4350 2350 50  0001 C CNN
F 3 "" H 4350 2350 50  0001 C CNN
	1    4350 2350
	-1   0    0    -1  
$EndComp
Text GLabel 4350 2600 2    50   Input ~ 0
RS485_A
Text GLabel 4350 2700 2    50   Input ~ 0
RS485_B
$Comp
L power:GND #PWR011
U 1 1 69C32016
P 4350 2950
F 0 "#PWR011" H 4350 2700 50  0001 C CNN
F 1 "GND" H 4350 2800 50  0000 C CNN
F 2 "" H 4350 2950 50  0001 C CNN
F 3 "" H 4350 2950 50  0001 C CNN
	1    4350 2950
	-1   0    0    -1  
$EndComp
Text GLabel 3400 2500 0    50   Input ~ 0
RX2
$Comp
L Device:R R4
U 1 1 69C3ABF7
P 4000 2350
F 0 "R4" V 3900 2300 50  0000 L CNN
F 1 "10K" V 4000 2350 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 3930 2350 50  0001 C CNN
F 3 "~" H 4000 2350 50  0001 C CNN
	1    4000 2350
	0    -1   1    0   
$EndComp
Wire Wire Line
	4350 2500 4350 2350
Wire Wire Line
	4350 2350 4150 2350
Wire Wire Line
	3850 2350 3650 2350
Wire Wire Line
	3650 2350 3650 2500
Text GLabel 3400 2600 0    50   Input ~ 0
TX_EN
Wire Wire Line
	3650 2500 3400 2500
Connection ~ 3650 2500
Wire Wire Line
	3650 2600 3550 2600
Wire Wire Line
	3650 2700 3550 2700
Wire Wire Line
	3550 2700 3550 2600
Connection ~ 3550 2600
Wire Wire Line
	3550 2600 3400 2600
Connection ~ 4350 2350
Text GLabel 3400 2800 0    50   Input ~ 0
TX2
$Comp
L Device:R R7
U 1 1 69C4D6CD
P 4000 2950
F 0 "R7" V 4100 2900 50  0000 L CNN
F 1 "10K" V 4000 2950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 3930 2950 50  0001 C CNN
F 3 "~" H 4000 2950 50  0001 C CNN
	1    4000 2950
	0    -1   1    0   
$EndComp
Wire Wire Line
	3550 2700 3550 2950
Wire Wire Line
	3550 2950 3850 2950
Connection ~ 3550 2700
Wire Wire Line
	4150 2950 4350 2950
Wire Wire Line
	4350 2950 4350 2800
Connection ~ 4350 2950
Wire Wire Line
	3400 2800 3650 2800
Text GLabel 1800 3700 3    50   Input ~ 0
TX_EN
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 69C67277
P 8600 4000
F 0 "J7" H 8750 4100 50  0000 C CNN
F 1 "RS485" H 8850 3950 50  0000 C CNN
F 2 "0_my_footprints:myPheonix3x3.81_right" H 8600 4000 50  0001 C CNN
F 3 "~" H 8600 4000 50  0001 C CNN
	1    8600 4000
	1    0    0    -1  
$EndComp
Text GLabel 8400 4000 0    50   Input ~ 0
RS485_A
Text GLabel 8400 3900 0    50   Input ~ 0
RS485_B
$Comp
L power:GND #PWR019
U 1 1 69C70B80
P 8400 4100
F 0 "#PWR019" H 8400 3850 50  0001 C CNN
F 1 "GND" V 8400 3900 50  0000 C CNN
F 2 "" H 8400 4100 50  0001 C CNN
F 3 "" H 8400 4100 50  0001 C CNN
	1    8400 4100
	0    1    -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J8
U 1 1 69A3A85D
P 3900 3850
F 0 "J8" V 4050 4250 50  0000 C CNN
F 1 "RS485_TERMINATOR" V 4050 3750 50  0000 C CNN
F 2 "0_my_footprints:myPinHeader_1x02" H 3900 3850 50  0001 C CNN
F 3 "~" H 3900 3850 50  0001 C CNN
	1    3900 3850
	0    -1   1    0   
$EndComp
$Comp
L Device:R R5
U 1 1 69A58E3A
P 4150 3650
F 0 "R5" V 4050 3600 50  0000 L CNN
F 1 "220" V 4150 3650 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 4080 3650 50  0001 C CNN
F 3 "~" H 4150 3650 50  0001 C CNN
	1    4150 3650
	0    -1   1    0   
$EndComp
Text GLabel 4300 3650 2    50   Input ~ 0
RS485_B
Text GLabel 3800 3650 0    50   Input ~ 0
RS485_A
Wire Wire Line
	3900 3650 3800 3650
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 69A9EB4E
P 8600 2100
F 0 "J4" H 8750 2100 50  0000 C CNN
F 1 "SENSOR" H 8850 2000 50  0000 C CNN
F 2 "0_my_footprints:myJSTx02" H 8600 2100 50  0001 C CNN
F 3 "~" H 8600 2100 50  0001 C CNN
	1    8600 2100
	1    0    0    -1  
$EndComp
$EndSCHEMATC
