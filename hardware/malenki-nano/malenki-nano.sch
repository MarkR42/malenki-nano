EESchema Schematic File Version 4
LIBS:malenki-nano-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Malenki-Nano"
Date "2019-05-15"
Rev ""
Comp "Mark Robson"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L mal_nano:generic-ldo U2
U 1 1 5CDC3072
P 3100 1750
F 0 "U2" H 3100 1842 50  0000 C CNN
F 1 "generic-ldo" H 3100 1751 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-89-3" H 3100 1750 50  0001 C CNN
F 3 "" H 3100 1750 50  0001 C CNN
	1    3100 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5CDC3232
P 2500 2050
F 0 "C2" H 2615 2096 50  0000 L CNN
F 1 "10uF" H 2615 2005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2538 1900 50  0001 C CNN
F 3 "~" H 2500 2050 50  0001 C CNN
	1    2500 2050
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5CDC3533
P 3650 2050
F 0 "C3" H 3765 2096 50  0000 L CNN
F 1 "10uF" H 3765 2005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3688 1900 50  0001 C CNN
F 3 "~" H 3650 2050 50  0001 C CNN
	1    3650 2050
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Male J1
U 1 1 5CDC3C93
P 1300 1900
F 0 "J1" H 1408 2081 50  0000 C CNN
F 1 "Conn_01x02_Male" H 1408 1990 50  0000 C CNN
F 2 "malenki-nano:PinHeader_1x02_P2.54mm_BIG1" H 1300 1900 50  0001 C CNN
F 3 "~" H 1300 1900 50  0001 C CNN
	1    1300 1900
	1    0    0    -1  
$EndComp
Text Notes 1250 1600 0    50   ~ 0
Power in
Wire Wire Line
	1500 1900 2250 1900
Wire Wire Line
	2500 1900 2800 1900
Connection ~ 2500 1900
Wire Wire Line
	1500 2000 1800 2000
Wire Wire Line
	1800 2000 1800 2200
Connection ~ 2500 2200
Wire Wire Line
	3100 2200 3650 2200
Connection ~ 3100 2200
Wire Wire Line
	3650 1900 3400 1900
Wire Wire Line
	4100 1900 3650 1900
Connection ~ 3650 1900
$Comp
L power:GND #PWR02
U 1 1 5CDC503D
P 1800 2200
F 0 "#PWR02" H 1800 1950 50  0001 C CNN
F 1 "GND" H 1805 2027 50  0000 C CNN
F 2 "" H 1800 2200 50  0001 C CNN
F 3 "" H 1800 2200 50  0001 C CNN
	1    1800 2200
	1    0    0    -1  
$EndComp
Connection ~ 1800 2200
Text Label 1850 1900 0    50   ~ 0
VBAT
Text Label 3850 1900 0    50   ~ 0
VLOGIC
$Comp
L mal_nano:ATtiny1614 U3
U 1 1 5CDC52F1
P 5150 4100
F 0 "U3" H 5400 4900 50  0000 C CNN
F 1 "ATtiny1614" H 5550 4800 50  0000 C CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 5150 4100 50  0001 C CNN
F 3 "" H 5150 4100 50  0001 C CNN
	1    5150 4100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5CDC5AD8
P 4450 3300
F 0 "C4" H 4565 3346 50  0000 L CNN
F 1 "0.1uF" H 4565 3255 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4488 3150 50  0001 C CNN
F 3 "~" H 4450 3300 50  0001 C CNN
	1    4450 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 3150 4450 3150
Wire Wire Line
	5150 3150 5150 3350
$Comp
L power:GND #PWR04
U 1 1 5CDC6687
P 4450 3450
F 0 "#PWR04" H 4450 3200 50  0001 C CNN
F 1 "GND" H 4455 3277 50  0000 C CNN
F 2 "" H 4450 3450 50  0001 C CNN
F 3 "" H 4450 3450 50  0001 C CNN
	1    4450 3450
	1    0    0    -1  
$EndComp
Text Label 4900 3150 0    50   ~ 0
VLOGIC
$Comp
L power:GND #PWR05
U 1 1 5CDC6D64
P 5150 5100
F 0 "#PWR05" H 5150 4850 50  0001 C CNN
F 1 "GND" H 5155 4927 50  0000 C CNN
F 2 "" H 5150 5100 50  0001 C CNN
F 3 "" H 5150 5100 50  0001 C CNN
	1    5150 5100
	1    0    0    -1  
$EndComp
Text Notes 2850 1550 0    50   ~ 0
3.3v regulator
$Comp
L mal_nano:A7105-MODULE U1
U 1 1 5CDC9C70
P 2100 5550
F 0 "U1" H 2250 6350 50  0000 C CNN
F 1 "A7105-MODULE" H 2500 6250 50  0000 C CNN
F 2 "malenki-nano:A7105-module" H 2100 5550 50  0001 C CNN
F 3 "" H 2100 5550 50  0001 C CNN
	1    2100 5550
	1    0    0    -1  
$EndComp
Text Notes 2200 5050 0    50   ~ 0
Radio Receiver module
Text Notes 5400 3100 0    50   ~ 0
MCU\n
$Comp
L power:GND #PWR03
U 1 1 5CDCA41E
P 2100 6250
F 0 "#PWR03" H 2100 6000 50  0001 C CNN
F 1 "GND" H 2105 6077 50  0000 C CNN
F 2 "" H 2100 6250 50  0001 C CNN
F 3 "" H 2100 6250 50  0001 C CNN
	1    2100 6250
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5CDCC5F2
P 1400 4800
F 0 "C1" H 1515 4846 50  0000 L CNN
F 1 "0.1uF" H 1515 4755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1438 4650 50  0001 C CNN
F 3 "~" H 1400 4800 50  0001 C CNN
	1    1400 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5CDCCAD0
P 1400 4950
F 0 "#PWR01" H 1400 4700 50  0001 C CNN
F 1 "GND" H 1405 4777 50  0000 C CNN
F 2 "" H 1400 4950 50  0001 C CNN
F 3 "" H 1400 4950 50  0001 C CNN
	1    1400 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	1400 4650 2100 4650
Wire Wire Line
	2100 4650 2100 4950
Text Label 1800 4650 0    50   ~ 0
VLOGIC
Text GLabel 2700 5250 2    50   Input ~ 0
SPI_SDIO
Text GLabel 2700 5350 2    50   Input ~ 0
SPI_SCK
Text GLabel 2700 5450 2    50   Input ~ 0
SPI_SCS
Text GLabel 1500 5350 0    50   Input ~ 0
GIO1
Text GLabel 4500 3850 0    50   Input ~ 0
MOTOR3R
Text GLabel 4500 3950 0    50   Input ~ 0
MOTOR3F
Text GLabel 4500 4350 0    50   Input ~ 0
MOTOR2F
Text GLabel 5950 3850 2    50   Input ~ 0
MOTOR1F
Text GLabel 5950 4250 2    50   Input ~ 0
MOTOR1R
Text GLabel 5950 4350 2    50   Input ~ 0
MOTOR2R
Text GLabel 5950 4050 2    50   Input ~ 0
TXDEBUG
Text GLabel 5950 4150 2    50   Input ~ 0
UPDI
$Comp
L Connector:Conn_01x04_Male J2
U 1 1 5CDCE93F
P 4900 1800
F 0 "J2" H 5008 2081 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5008 1990 50  0000 C CNN
F 2 "Connector_PinHeader_2.00mm:PinHeader_1x04_P2.00mm_Vertical" H 4900 1800 50  0001 C CNN
F 3 "~" H 4900 1800 50  0001 C CNN
	1    4900 1800
	1    0    0    -1  
$EndComp
Text Notes 4700 1450 0    50   ~ 0
Programming / diagnostic
$Comp
L power:GND #PWR07
U 1 1 5CDD00E2
P 6200 2100
F 0 "#PWR07" H 6200 1850 50  0001 C CNN
F 1 "GND" H 6205 1927 50  0000 C CNN
F 2 "" H 6200 2100 50  0001 C CNN
F 3 "" H 6200 2100 50  0001 C CNN
	1    6200 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 1700 6200 1700
Wire Wire Line
	6200 1700 6200 2100
Wire Wire Line
	5100 1800 5900 1800
Text Label 5300 1800 0    50   ~ 0
VLOGIC
Text GLabel 5100 1900 2    50   Input ~ 0
TXDEBUG
Text GLabel 5100 2000 2    50   Input ~ 0
UPDI
Text GLabel 1500 5550 0    50   Input ~ 0
BLINKY
Text GLabel 4500 4050 0    50   Input ~ 0
SPI_SDIO
Text GLabel 4500 4150 0    50   Input ~ 0
SPI_SCK
Text GLabel 4500 4250 0    50   Input ~ 0
SPI_SCS
Text GLabel 5950 3950 2    50   Input ~ 0
GIO1
Text GLabel 5150 6000 1    50   Input ~ 0
BLINKY
$Comp
L Device:R R1
U 1 1 5CDD24A6
P 5150 6450
F 0 "R1" H 5220 6496 50  0000 L CNN
F 1 "220R" H 5220 6405 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5080 6450 50  0001 C CNN
F 3 "~" H 5150 6450 50  0001 C CNN
	1    5150 6450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5CDD286C
P 5150 6600
F 0 "#PWR06" H 5150 6350 50  0001 C CNN
F 1 "GND" H 5155 6427 50  0000 C CNN
F 2 "" H 5150 6600 50  0001 C CNN
F 3 "" H 5150 6600 50  0001 C CNN
	1    5150 6600
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 5CDD2ED4
P 5150 6150
F 0 "D1" V 5189 6033 50  0000 R CNN
F 1 "LED Blue" V 5098 6033 50  0000 R CNN
F 2 "LED_SMD:LED_1206_3216Metric_Castellated" H 5150 6150 50  0001 C CNN
F 3 "~" H 5150 6150 50  0001 C CNN
	1    5150 6150
	0    -1   -1   0   
$EndComp
Text Notes 1150 6700 0    50   ~ 0
Note: we drive BLINKY with the GIO pin on the \nradio module, because there are no spare pins\non the MCU
$Comp
L mal_nano:RZ7899 U5
U 1 1 5CDD4D34
P 9500 1400
F 0 "U5" H 9400 1765 50  0000 C CNN
F 1 "RZ7899" H 9400 1674 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9450 1400 50  0001 C CNN
F 3 "" H 9450 1400 50  0001 C CNN
	1    9500 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 5CDD5C99
P 8650 1850
F 0 "C5" H 8765 1896 50  0000 L CNN
F 1 "22uF" H 8765 1805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric" H 8688 1700 50  0001 C CNN
F 3 "~" H 8650 1850 50  0001 C CNN
	1    8650 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 1700 8650 1700
Wire Wire Line
	8650 2000 8650 2150
Wire Wire Line
	8900 1600 8050 1600
Wire Wire Line
	8050 1600 8050 2150
Wire Wire Line
	8050 2150 8650 2150
$Comp
L power:GND #PWR08
U 1 1 5CDD746F
P 8650 2150
F 0 "#PWR08" H 8650 1900 50  0001 C CNN
F 1 "GND" H 8655 1977 50  0000 C CNN
F 2 "" H 8650 2150 50  0001 C CNN
F 3 "" H 8650 2150 50  0001 C CNN
	1    8650 2150
	1    0    0    -1  
$EndComp
Connection ~ 8650 2150
Text GLabel 8900 1500 0    50   Input ~ 0
MOTOR1F
Text GLabel 8900 1400 0    50   Input ~ 0
MOTOR1R
Wire Notes Line
	7850 550  11150 550 
Wire Notes Line
	11150 550  11150 2500
Wire Notes Line
	11150 2500 7850 2500
Wire Notes Line
	7850 2500 7850 550 
Text Notes 9050 800  0    50   ~ 0
Motor1 - Weapon
$Comp
L Connector:Conn_01x02_Male J5
U 1 1 5CDD8946
P 10600 1600
F 0 "J5" H 10572 1482 50  0000 R CNN
F 1 "Conn_01x02_Male" H 10572 1573 50  0000 R CNN
F 2 "malenki-nano:PinHeader_1x02_P2.54mm_BIG1" H 10600 1600 50  0001 C CNN
F 3 "~" H 10600 1600 50  0001 C CNN
	1    10600 1600
	-1   0    0    1   
$EndComp
Wire Wire Line
	9900 1500 10150 1500
Wire Wire Line
	10400 1600 10150 1600
Wire Wire Line
	9900 1400 10150 1400
Wire Wire Line
	10150 1400 10150 1500
Connection ~ 10150 1500
Wire Wire Line
	10150 1500 10400 1500
Wire Wire Line
	9900 1700 10150 1700
Wire Wire Line
	10150 1700 10150 1600
Connection ~ 10150 1600
Wire Wire Line
	10150 1600 9900 1600
$Comp
L mal_nano:RZ7899 U6
U 1 1 5CDDD923
P 9500 3400
F 0 "U6" H 9400 3765 50  0000 C CNN
F 1 "RZ7899" H 9400 3674 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9450 3400 50  0001 C CNN
F 3 "" H 9450 3400 50  0001 C CNN
	1    9500 3400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 5CDDD929
P 8650 3850
F 0 "C6" H 8765 3896 50  0000 L CNN
F 1 "22uF" H 8765 3805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric" H 8688 3700 50  0001 C CNN
F 3 "~" H 8650 3850 50  0001 C CNN
	1    8650 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 3700 8650 3700
Wire Wire Line
	8650 4000 8650 4150
Wire Wire Line
	8900 3600 8050 3600
Wire Wire Line
	8050 3600 8050 4150
Wire Wire Line
	8050 4150 8650 4150
$Comp
L power:GND #PWR09
U 1 1 5CDDD934
P 8650 4150
F 0 "#PWR09" H 8650 3900 50  0001 C CNN
F 1 "GND" H 8655 3977 50  0000 C CNN
F 2 "" H 8650 4150 50  0001 C CNN
F 3 "" H 8650 4150 50  0001 C CNN
	1    8650 4150
	1    0    0    -1  
$EndComp
Connection ~ 8650 4150
Text GLabel 8900 3500 0    50   Input ~ 0
MOTOR2F
Text GLabel 8900 3400 0    50   Input ~ 0
MOTOR2R
Wire Notes Line
	7850 2550 11150 2550
Wire Notes Line
	11150 2550 11150 4500
Wire Notes Line
	11150 4500 7850 4500
Wire Notes Line
	7850 4500 7850 2550
Text Notes 9050 2800 0    50   ~ 0
Motor2 - Left
$Comp
L Connector:Conn_01x02_Male J6
U 1 1 5CDDD942
P 10600 3600
F 0 "J6" H 10572 3482 50  0000 R CNN
F 1 "Conn_01x02_Male" H 10572 3573 50  0000 R CNN
F 2 "malenki-nano:PinHeader_1x02_P2.54mm_BIG1" H 10600 3600 50  0001 C CNN
F 3 "~" H 10600 3600 50  0001 C CNN
	1    10600 3600
	-1   0    0    1   
$EndComp
Wire Wire Line
	9900 3500 10150 3500
Wire Wire Line
	10400 3600 10150 3600
Wire Wire Line
	9900 3400 10150 3400
Wire Wire Line
	10150 3400 10150 3500
Connection ~ 10150 3500
Wire Wire Line
	10150 3500 10400 3500
Wire Wire Line
	9900 3700 10150 3700
Wire Wire Line
	10150 3700 10150 3600
Connection ~ 10150 3600
Wire Wire Line
	10150 3600 9900 3600
$Comp
L mal_nano:RZ7899 U7
U 1 1 5CDDF08C
P 9500 5400
F 0 "U7" H 9400 5765 50  0000 C CNN
F 1 "RZ7899" H 9400 5674 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9450 5400 50  0001 C CNN
F 3 "" H 9450 5400 50  0001 C CNN
	1    9500 5400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C7
U 1 1 5CDDF092
P 8650 5850
F 0 "C7" H 8765 5896 50  0000 L CNN
F 1 "22uF" H 8765 5805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric" H 8688 5700 50  0001 C CNN
F 3 "~" H 8650 5850 50  0001 C CNN
	1    8650 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 5700 8650 5700
Wire Wire Line
	8650 6000 8650 6150
Wire Wire Line
	8900 5600 8000 5600
Wire Wire Line
	8000 5600 8000 6150
Wire Wire Line
	8000 6150 8650 6150
$Comp
L power:GND #PWR010
U 1 1 5CDDF09D
P 8650 6250
F 0 "#PWR010" H 8650 6000 50  0001 C CNN
F 1 "GND" H 8655 6077 50  0000 C CNN
F 2 "" H 8650 6250 50  0001 C CNN
F 3 "" H 8650 6250 50  0001 C CNN
	1    8650 6250
	1    0    0    -1  
$EndComp
Connection ~ 8650 6150
Text GLabel 8900 5500 0    50   Input ~ 0
MOTOR3F
Text GLabel 8900 5400 0    50   Input ~ 0
MOTOR3R
Wire Notes Line
	7850 4550 11150 4550
Wire Notes Line
	11150 4550 11150 6500
Wire Notes Line
	11150 6500 7850 6500
Wire Notes Line
	7850 6500 7850 4550
Text Notes 9050 4800 0    50   ~ 0
Motor3 - Right
$Comp
L Connector:Conn_01x02_Male J7
U 1 1 5CDDF0AB
P 10600 5600
F 0 "J7" H 10572 5482 50  0000 R CNN
F 1 "Conn_01x02_Male" H 10572 5573 50  0000 R CNN
F 2 "malenki-nano:PinHeader_1x02_P2.54mm_BIG1" H 10600 5600 50  0001 C CNN
F 3 "~" H 10600 5600 50  0001 C CNN
	1    10600 5600
	-1   0    0    1   
$EndComp
Wire Wire Line
	9900 5500 10150 5500
Wire Wire Line
	10400 5600 10150 5600
Wire Wire Line
	9900 5400 10150 5400
Wire Wire Line
	10150 5400 10150 5500
Connection ~ 10150 5500
Wire Wire Line
	10150 5500 10400 5500
Wire Wire Line
	9900 5700 10150 5700
Wire Wire Line
	10150 5700 10150 5600
Connection ~ 10150 5600
Wire Wire Line
	10150 5600 9900 5600
Text GLabel 8450 3700 0    50   Input ~ 0
VBAT
Wire Wire Line
	8450 3700 8650 3700
Connection ~ 8650 3700
Text GLabel 8350 1700 0    50   Input ~ 0
VBAT
Wire Wire Line
	8650 1700 8350 1700
Connection ~ 8650 1700
Text GLabel 8400 5700 0    50   Input ~ 0
VBAT
Wire Wire Line
	8650 5700 8400 5700
Connection ~ 8650 5700
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5CDF9B16
P 2250 1900
F 0 "#FLG0101" H 2250 1975 50  0001 C CNN
F 1 "PWR_FLAG" H 2250 2073 50  0000 C CNN
F 2 "" H 2250 1900 50  0001 C CNN
F 3 "~" H 2250 1900 50  0001 C CNN
	1    2250 1900
	1    0    0    -1  
$EndComp
Connection ~ 2250 1900
Wire Wire Line
	2250 1900 2500 1900
Wire Wire Line
	1950 6200 2100 6200
Wire Wire Line
	2100 6250 2100 6200
Connection ~ 2100 6200
Wire Wire Line
	2100 6200 2250 6200
Wire Wire Line
	8650 6150 8650 6250
Wire Wire Line
	1800 2200 2250 2200
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5CDCEDF5
P 2250 2200
F 0 "#FLG0102" H 2250 2275 50  0001 C CNN
F 1 "PWR_FLAG" H 2250 2373 50  0000 C CNN
F 2 "" H 2250 2200 50  0001 C CNN
F 3 "~" H 2250 2200 50  0001 C CNN
	1    2250 2200
	-1   0    0    1   
$EndComp
Connection ~ 2250 2200
Wire Wire Line
	2250 2200 2500 2200
Wire Wire Line
	2500 2200 3100 2200
Text GLabel 5750 6000 1    50   Input ~ 0
TXDEBUG
$Comp
L Device:R R2
U 1 1 5D6AB858
P 5750 6450
F 0 "R2" H 5820 6496 50  0000 L CNN
F 1 "220R" H 5820 6405 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5680 6450 50  0001 C CNN
F 3 "~" H 5750 6450 50  0001 C CNN
	1    5750 6450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5D6AB85E
P 5750 6600
F 0 "#PWR011" H 5750 6350 50  0001 C CNN
F 1 "GND" H 5755 6427 50  0000 C CNN
F 2 "" H 5750 6600 50  0001 C CNN
F 3 "" H 5750 6600 50  0001 C CNN
	1    5750 6600
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D2
U 1 1 5D6AB864
P 5750 6150
F 0 "D2" V 5789 6033 50  0000 R CNN
F 1 "LED Red" V 5698 6033 50  0000 R CNN
F 2 "LED_SMD:LED_1206_3216Metric_Castellated" H 5750 6150 50  0001 C CNN
F 3 "~" H 5750 6150 50  0001 C CNN
	1    5750 6150
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
