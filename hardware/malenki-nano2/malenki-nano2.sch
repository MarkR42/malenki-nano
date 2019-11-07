EESchema Schematic File Version 5
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Malenki-Nano2"
Date "2019-11-07"
Rev ""
Comp "Mark Robson"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
Comment5 ""
Comment6 ""
Comment7 ""
Comment8 ""
Comment9 ""
$EndDescr
$Comp
L malenki-nano-rescue:C-Device C2
U 1 1 5CDC3232
P 2500 2050
F 0 "C2" H 2615 2096 50  0000 L CNN
F 1 "10uF" H 2615 2005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2538 1900 50  0001 C CNN
F 3 "~" H 2500 2050 50  0001 C CNN
	1    2500 2050
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:C-Device C3
U 1 1 5CDC3533
P 3650 2050
F 0 "C3" H 3765 2096 50  0000 L CNN
F 1 "10uF" H 3765 2005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3688 1900 50  0001 C CNN
F 3 "~" H 3650 2050 50  0001 C CNN
	1    3650 2050
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:Conn_01x02_Male-Connector J1
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
	3650 1900 3400 1900
Wire Wire Line
	4100 1900 3650 1900
Connection ~ 3650 1900
$Comp
L malenki-nano-rescue:GND-power #PWR02
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
Wire Wire Line
	5000 2900 5000 3100
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
L malenki-nano-rescue:GND-power #PWR03
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
L malenki-nano-rescue:C-Device C1
U 1 1 5CDCC5F2
P 1400 4800
F 0 "C1" H 1515 4846 50  0000 L CNN
F 1 "0.1uF" H 1515 4755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1438 4650 50  0001 C CNN
F 3 "~" H 1400 4800 50  0001 C CNN
	1    1400 4800
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:GND-power #PWR01
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
Text GLabel 5600 4300 2    50   Input ~ 0
MOTOR1R
Text GLabel 5600 4400 2    50   Input ~ 0
MOTOR2R
Text GLabel 5600 3400 2    50   Input ~ 0
UPDI
$Comp
L malenki-nano-rescue:Conn_01x04_Male-Connector J2
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
L malenki-nano-rescue:GND-power #PWR07
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
Text GLabel 5150 6000 1    50   Input ~ 0
BLINKY
$Comp
L malenki-nano-rescue:R-Device R1
U 1 1 5CDD24A6
P 5150 6450
F 0 "R1" H 5220 6496 50  0000 L CNN
F 1 "220R" H 5220 6405 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5080 6450 50  0001 C CNN
F 3 "~" H 5150 6450 50  0001 C CNN
	1    5150 6450
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:GND-power #PWR06
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
L malenki-nano-rescue:LED-Device D1
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
L malenki-nano-rescue:C-Device C5
U 1 1 5CDD5C99
P 8650 1850
F 0 "C5" H 8765 1896 50  0000 L CNN
F 1 "10uF" H 8765 1805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8688 1700 50  0001 C CNN
F 3 "~" H 8650 1850 50  0001 C CNN
	1    8650 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8650 2000 8650 2100
$Comp
L malenki-nano-rescue:GND-power #PWR08
U 1 1 5CDD746F
P 8650 2150
F 0 "#PWR08" H 8650 1900 50  0001 C CNN
F 1 "GND" H 8655 1977 50  0000 C CNN
F 2 "" H 8650 2150 50  0001 C CNN
F 3 "" H 8650 2150 50  0001 C CNN
	1    8650 2150
	1    0    0    -1  
$EndComp
Text GLabel 8900 1500 0    50   Input ~ 0
MOTOR1F
Text GLabel 8900 1300 0    50   Input ~ 0
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
L malenki-nano-rescue:Conn_01x02_Male-Connector J5
U 1 1 5CDD8946
P 10700 1400
F 0 "J5" H 10672 1282 50  0000 R CNN
F 1 "Conn_01x02_Male" H 10672 1373 50  0000 R CNN
F 2 "malenki-nano:PinHeader_1x02_P2.54mm_BIG1" H 10700 1400 50  0001 C CNN
F 3 "~" H 10700 1400 50  0001 C CNN
	1    10700 1400
	-1   0    0    1   
$EndComp
Wire Wire Line
	10500 1400 10250 1400
Wire Wire Line
	10000 1500 10250 1500
Wire Wire Line
	10250 1500 10250 1400
$Comp
L malenki-nano-rescue:C-Device C6
U 1 1 5CDDD929
P 8450 3900
F 0 "C6" H 8565 3946 50  0000 L CNN
F 1 "10uF" H 8565 3855 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8488 3750 50  0001 C CNN
F 3 "~" H 8450 3900 50  0001 C CNN
	1    8450 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 3600 8800 3600
$Comp
L malenki-nano-rescue:GND-power #PWR09
U 1 1 5CDDD934
P 8450 4200
F 0 "#PWR09" H 8450 3950 50  0001 C CNN
F 1 "GND" H 8455 4027 50  0000 C CNN
F 2 "" H 8450 4200 50  0001 C CNN
F 3 "" H 8450 4200 50  0001 C CNN
	1    8450 4200
	1    0    0    -1  
$EndComp
Connection ~ 8450 4200
Text GLabel 8800 3600 0    50   Input ~ 0
MOTOR2F
Text GLabel 8800 3400 0    50   Input ~ 0
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
L malenki-nano-rescue:Conn_01x02_Male-Connector J6
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
	9900 3400 10150 3400
Wire Wire Line
	10150 3400 10150 3500
Wire Wire Line
	10150 3500 10400 3500
$Comp
L malenki-nano-rescue:C-Device C7
U 1 1 5CDDF092
P 8650 5850
F 0 "C7" H 8765 5896 50  0000 L CNN
F 1 "10uF" H 8765 5805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8688 5700 50  0001 C CNN
F 3 "~" H 8650 5850 50  0001 C CNN
	1    8650 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	8650 6000 8650 6150
$Comp
L malenki-nano-rescue:GND-power #PWR010
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
Text GLabel 9050 5600 0    50   Input ~ 0
MOTOR3F
Text GLabel 9050 5400 0    50   Input ~ 0
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
L malenki-nano-rescue:Conn_01x02_Male-Connector J7
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
	10400 5600 10150 5600
Wire Wire Line
	10150 5400 10150 5500
Wire Wire Line
	10150 5500 10400 5500
Text GLabel 8250 3750 0    50   Input ~ 0
VBAT
Text GLabel 8350 1700 0    50   Input ~ 0
VBAT
Wire Wire Line
	8650 1700 8450 1700
Text GLabel 8400 5700 0    50   Input ~ 0
VBAT
Wire Wire Line
	8650 5700 8500 5700
$Comp
L malenki-nano-rescue:PWR_FLAG-power #FLG0101
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
L malenki-nano-rescue:PWR_FLAG-power #FLG0102
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
Text GLabel 5750 6000 1    50   Input ~ 0
TXDEBUG
$Comp
L malenki-nano-rescue:R-Device R2
U 1 1 5D6AB858
P 5750 6450
F 0 "R2" H 5820 6496 50  0000 L CNN
F 1 "220R" H 5820 6405 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5680 6450 50  0001 C CNN
F 3 "~" H 5750 6450 50  0001 C CNN
	1    5750 6450
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:GND-power #PWR011
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
L malenki-nano-rescue:LED-Device D2
U 1 1 5D6AB864
P 5750 6150
F 0 "D2" V 5789 6033 50  0000 R CNN
F 1 "LED Red" V 5698 6033 50  0000 R CNN
F 2 "LED_SMD:LED_1206_3216Metric_Castellated" H 5750 6150 50  0001 C CNN
F 3 "~" H 5750 6150 50  0001 C CNN
	1    5750 6150
	0    -1   -1   0   
$EndComp
$Comp
L mal_nano:MX113L U5
U 1 1 5DC4CAF8
P 9400 1150
F 0 "U5" H 9450 1381 50  0000 C CNN
F 1 "MX113L" H 9450 1290 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 9400 1150 50  0001 C CNN
F 3 "" H 9400 1150 50  0001 C CNN
	1    9400 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 1100 8450 1100
Wire Wire Line
	8450 1100 8450 1700
Connection ~ 8450 1700
Wire Wire Line
	8450 1700 8350 1700
Wire Wire Line
	9400 1800 9400 2100
Wire Wire Line
	9400 2100 8650 2100
Connection ~ 8650 2100
Wire Wire Line
	8650 2100 8650 2150
Wire Wire Line
	10000 1300 10500 1300
$Comp
L mal_nano:MX113L U6
U 1 1 5DC52CCB
P 9300 3250
F 0 "U6" H 9350 3481 50  0000 C CNN
F 1 "MX113L" H 9350 3390 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 9300 3250 50  0001 C CNN
F 3 "" H 9300 3250 50  0001 C CNN
	1    9300 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	8250 3750 8300 3750
Wire Wire Line
	8450 4050 8450 4200
Wire Wire Line
	9300 3900 9300 4200
Wire Wire Line
	9300 4200 8450 4200
Wire Wire Line
	9300 3200 8300 3200
Wire Wire Line
	8300 3200 8300 3750
Connection ~ 8300 3750
Wire Wire Line
	8300 3750 8450 3750
Wire Wire Line
	9900 3600 10400 3600
$Comp
L mal_nano:MX113L U7
U 1 1 5DC616A1
P 9550 5250
F 0 "U7" H 9600 5481 50  0000 C CNN
F 1 "MX113L" H 9600 5390 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 9550 5250 50  0001 C CNN
F 3 "" H 9550 5250 50  0001 C CNN
	1    9550 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	9550 5900 9550 6150
Wire Wire Line
	9550 6150 8650 6150
Wire Wire Line
	8500 5700 8500 5200
Wire Wire Line
	8500 5200 9550 5200
Connection ~ 8500 5700
Wire Wire Line
	8500 5700 8400 5700
$Comp
L MCU_Microchip_ATtiny:ATtiny3217-M U3
U 1 1 5DC45560
P 5000 4200
F 0 "U3" H 5000 4250 50  0000 C CNN
F 1 "ATtiny3217-M" H 5300 5250 50  0000 C CNN
F 2 "malenki-nano:qfn24-hand1" H 5000 4200 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny3217_1617-Data-Sheet-40001999B.pdf" H 5000 4200 50  0001 C CNN
	1    5000 4200
	1    0    0    -1  
$EndComp
Text GLabel 5600 4600 2    50   Input ~ 0
SPI_SCS
Text GLabel 5600 4100 2    50   Input ~ 0
SPI_SCK
Text GLabel 5600 4000 2    50   Input ~ 0
SPI_SDIO
Text GLabel 5600 4500 2    50   Input ~ 0
MOTOR2F
Text GLabel 5600 3900 2    50   Input ~ 0
MOTOR3F
Text GLabel 5600 3800 2    50   Input ~ 0
MOTOR3R
$Comp
L malenki-nano-rescue:GND-power #PWR05
U 1 1 5CDC6D64
P 5000 5300
F 0 "#PWR05" H 5000 5050 50  0001 C CNN
F 1 "GND" H 5005 5127 50  0000 C CNN
F 2 "" H 5000 5300 50  0001 C CNN
F 3 "" H 5000 5300 50  0001 C CNN
	1    5000 5300
	1    0    0    -1  
$EndComp
$Comp
L malenki-nano-rescue:GND-power #PWR04
U 1 1 5CDC6687
P 4300 3200
F 0 "#PWR04" H 4300 2950 50  0001 C CNN
F 1 "GND" H 4305 3027 50  0000 C CNN
F 2 "" H 4300 3200 50  0001 C CNN
F 3 "" H 4300 3200 50  0001 C CNN
	1    4300 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 2900 4300 2900
$Comp
L malenki-nano-rescue:C-Device C4
U 1 1 5CDC5AD8
P 4300 3050
F 0 "C4" H 4415 3096 50  0000 L CNN
F 1 "0.1uF" H 4415 3005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4338 2900 50  0001 C CNN
F 3 "~" H 4300 3050 50  0001 C CNN
	1    4300 3050
	1    0    0    -1  
$EndComp
Text Label 4750 2900 0    50   ~ 0
VLOGIC
Text GLabel 5600 3700 2    50   Input ~ 0
GIO1
Text GLabel 5600 3500 2    50   Input ~ 0
TXDEBUG
Text GLabel 5600 3600 2    50   Input ~ 0
MOTOR1F
NoConn ~ 5600 4700
NoConn ~ 5600 4800
NoConn ~ 5600 4900
NoConn ~ 5600 5000
NoConn ~ 4400 4300
NoConn ~ 4400 4400
NoConn ~ 4400 4500
NoConn ~ 4400 4600
NoConn ~ 4400 4700
NoConn ~ 4400 4800
$Comp
L mal_nano:HT7233 U2
U 1 1 5DCB118A
P 3100 1750
F 0 "U2" H 3100 1842 50  0000 C CNN
F 1 "HT7233" H 3100 1751 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 3100 1750 50  0001 C CNN
F 3 "https://datasheet.lcsc.com/szlcsc/1809191917_Holtek-Semicon-HT7233_C47970.pdf" H 3100 1750 50  0001 C CNN
	1    3100 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2200 3100 2200
Connection ~ 3100 2200
Wire Wire Line
	3100 2200 3650 2200
$EndSCHEMATC
