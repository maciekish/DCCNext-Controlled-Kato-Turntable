EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "DCC Controlled Kato Turntable"
Date "2020-08-09"
Rev "0.1"
Comp "John van Staaijeren"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 5F1F27A4
P 7700 2100
F 0 "J2" V 8200 2400 50  0000 C CNN
F 1 "Kato Turntable (Red=SW)" V 8100 2000 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 7700 2100 50  0001 C CNN
F 3 "~" H 7700 2100 50  0001 C CNN
	1    7700 2100
	0    -1   -1   0   
$EndComp
$Comp
L DCC-Controlled-Kato-Turntable_Library:Relay_HFD2_005-S-L2-D K1
U 1 1 5F21B158
P 8900 3650
F 0 "K1" V 9465 3675 50  0000 C CNN
F 1 "HFD2_005-S-L2-D" V 9374 3675 50  0000 C CNN
F 2 "DCC-Controlled-Kato-Turntable:Relay_HFD2_W7.62mm_LongPads" H 8900 3650 50  0001 C CNN
F 3 "https://www.hongfa.com/product/detail/9974ede8-6db0-490e-8d9a-212d7dc3cf80" H 8900 3650 50  0001 C CNN
	1    8900 3650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8700 2300 8700 3000
Wire Wire Line
	8800 2300 8800 3000
Text Notes 4000 2050 1    50   ~ 0
P15-D18 (A4-SDA)
Text Notes 4100 2050 1    50   ~ 0
P14-D17 (A3)
Text Notes 4200 2050 1    50   ~ 0
P13-D16 (A2)
Text Notes 4300 2050 1    50   ~ 0
P12-D15 (A1)
Text Notes 4400 2050 1    50   ~ 0
P11-D14 (A0)
Text Notes 5100 2050 1    50   ~ 0
P8-D10 (PWM)
Text Notes 5200 2050 1    50   ~ 0
P7-D9 (PWM)
Text Notes 5300 2050 1    50   ~ 0
P6-D8
Text Notes 5400 2050 1    50   ~ 0
P5-D7
Text Notes 5500 2050 1    50   ~ 0
P4-D6 (PWM)
Text Notes 5600 2050 1    50   ~ 0
P3-D5 (PWM)
Text Notes 5700 2050 1    50   ~ 0
P2-D4
Text Notes 5800 2050 1    50   ~ 0
P1-D3
Text Notes 7400 2050 1    50   ~ 0
Switch
Text Notes 7500 2050 1    50   ~ 0
GND
Text Notes 7600 2050 1    50   ~ 0
TurnM1
Text Notes 7700 2050 1    50   ~ 0
TurnM2
Text Notes 8000 2050 1    50   ~ 0
BridgeL
Text Notes 8100 2050 1    50   ~ 0
BridgeR
Text Notes 3800 2050 1    50   ~ 0
+5V
Text Notes 3700 2050 1    50   ~ 0
GND
Text Notes 4900 2050 1    50   ~ 0
P10-D12
NoConn ~ 8600 4350
NoConn ~ 8700 4350
NoConn ~ 8800 4350
NoConn ~ 8900 4350
Wire Wire Line
	6500 3500 6500 3400
Wire Wire Line
	6500 3400 6300 3400
Wire Wire Line
	6300 3500 6300 3400
Wire Wire Line
	8400 3900 8450 3900
Text Notes 8700 2000 1    50   ~ 0
K
Text Notes 8800 2000 1    50   ~ 0
J
Wire Wire Line
	8700 3000 8400 3000
Wire Wire Line
	7600 2300 7600 3900
Wire Wire Line
	9350 3900 9350 4000
Wire Wire Line
	9300 3900 9350 3900
Wire Wire Line
	9300 4000 9350 4000
Wire Wire Line
	5800 3400 6300 3400
Text Notes 3900 2050 1    50   ~ 0
P16-D19 (A5-SCL)
Text Notes 5000 2050 1    50   ~ 0
P9-D11 (PWM)
Wire Wire Line
	7500 2300 7500 2800
Wire Wire Line
	5800 2300 5800 2600
Wire Wire Line
	7400 2300 7400 2600
Connection ~ 7500 2800
Wire Wire Line
	8100 2300 8100 2800
Connection ~ 8100 2800
Wire Wire Line
	5900 4300 5800 4300
Wire Wire Line
	5800 4900 5900 4900
Wire Wire Line
	6900 3900 7600 3900
Wire Wire Line
	6900 4100 7700 4100
Wire Wire Line
	6900 4500 7800 4500
Wire Wire Line
	6500 5500 6500 5300
Wire Wire Line
	6300 5500 6300 5300
Wire Wire Line
	6200 5500 6200 5300
Wire Wire Line
	5100 2300 5100 4500
$Comp
L DCC-Controlled-Kato-Turntable_Library:Conn_DCCNext_01x22 J1
U 1 1 5F3B9FF5
P 4800 2100
F 0 "J1" V 4900 1900 50  0000 C CNN
F 1 "DCCNext Connector" V 4800 500 50  0000 C CNN
F 2 "DCC-Controlled-Kato-Turntable:PinHeader_DCCNext_1x22_P2.54mm_Vertical" H 4800 2100 50  0001 C CNN
F 3 "~" H 4800 2100 50  0001 C CNN
	1    4800 2100
	0    1    -1   0   
$EndComp
Wire Wire Line
	4450 4550 4450 4650
Wire Wire Line
	4850 4450 4850 4650
Wire Wire Line
	4050 5500 4450 5500
Connection ~ 4050 5500
Wire Wire Line
	4050 5350 4050 5500
Wire Wire Line
	3650 5500 4050 5500
Wire Wire Line
	4850 5350 4850 5500
Wire Wire Line
	4850 4950 4850 5050
$Comp
L Device:R R1
U 1 1 5F24F185
P 4850 5200
F 0 "R1" H 4920 5246 50  0000 L CNN
F 1 "1k" H 4920 5155 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 4780 5200 50  0001 C CNN
F 3 "~" H 4850 5200 50  0001 C CNN
	1    4850 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 5F24ECEE
P 4850 4800
F 0 "D1" V 4900 4700 50  0000 R CNN
F 1 "Red" V 4800 4700 50  0000 R CNN
F 2 "LED_THT:LED_D3.0mm" H 4850 4800 50  0001 C CNN
F 3 "~" H 4850 4800 50  0001 C CNN
	1    4850 4800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4450 5500 4850 5500
Connection ~ 4450 5500
Wire Wire Line
	4450 5350 4450 5500
Wire Wire Line
	3650 5350 3650 5500
Wire Wire Line
	4450 4950 4450 5050
Wire Wire Line
	4050 4950 4050 5050
Wire Wire Line
	3650 4950 3650 5050
$Comp
L Device:R R3
U 1 1 5F2402B4
P 4050 5200
F 0 "R3" H 4120 5246 50  0000 L CNN
F 1 "1k" H 4120 5155 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3980 5200 50  0001 C CNN
F 3 "~" H 4050 5200 50  0001 C CNN
	1    4050 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5F23FED3
P 3650 5200
F 0 "R4" H 3720 5246 50  0000 L CNN
F 1 "1k" H 3720 5155 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3580 5200 50  0001 C CNN
F 3 "~" H 3650 5200 50  0001 C CNN
	1    3650 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5F23F470
P 4450 5200
F 0 "R2" H 4520 5246 50  0000 L CNN
F 1 "1k" H 4520 5155 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 4380 5200 50  0001 C CNN
F 3 "~" H 4450 5200 50  0001 C CNN
	1    4450 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D2
U 1 1 5F236251
P 4450 4800
F 0 "D2" V 4500 4700 50  0000 R CNN
F 1 "Green" V 4400 4700 50  0000 R CNN
F 2 "LED_THT:LED_D3.0mm" H 4450 4800 50  0001 C CNN
F 3 "~" H 4450 4800 50  0001 C CNN
	1    4450 4800
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED D3
U 1 1 5F2357DA
P 4050 4800
F 0 "D3" V 4100 4700 50  0000 R CNN
F 1 "Yellow" V 4000 4700 50  0000 R CNN
F 2 "LED_THT:LED_D3.0mm" H 4050 4800 50  0001 C CNN
F 3 "~" H 4050 4800 50  0001 C CNN
	1    4050 4800
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED D4
U 1 1 5F234B5D
P 3650 4800
F 0 "D4" V 3700 4700 50  0000 R CNN
F 1 "Blue" V 3600 4700 50  0000 R CNN
F 2 "LED_THT:LED_D3.0mm" H 3650 4800 50  0001 C CNN
F 3 "~" H 3650 4800 50  0001 C CNN
	1    3650 4800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5800 2600 7400 2600
Connection ~ 5800 4300
Wire Wire Line
	5800 4300 5800 4900
$Comp
L Driver_Motor:L293D U1
U 1 1 5F1E876E
P 6400 4500
F 0 "U1" H 6900 5350 50  0000 C CNN
F 1 "L293D" H 6950 5250 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm_LongPads" H 6650 3750 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/l293.pdf" H 6100 5200 50  0001 C CNN
	1    6400 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 3400 7400 3400
Wire Wire Line
	7400 3400 7400 4200
Wire Wire Line
	5800 3400 5800 4300
Connection ~ 6300 3400
Connection ~ 6500 3400
Wire Wire Line
	5600 2300 5600 3900
Wire Wire Line
	9350 4000 9350 4200
Wire Wire Line
	4400 4450 4850 4450
Wire Wire Line
	4300 2300 4300 4550
Wire Wire Line
	4300 4550 4450 4550
Wire Wire Line
	4200 2300 4200 4550
Wire Wire Line
	4050 4550 4050 4650
Wire Wire Line
	4100 2300 4100 4450
Wire Wire Line
	4100 4450 3650 4450
Wire Wire Line
	3650 4450 3650 4650
$Comp
L Connector_Generic:Conn_01x04 J4
U 1 1 5F4FC801
P 3000 3100
F 0 "J4" H 3050 3400 50  0000 R CNN
F 1 "I2C LCD" H 3050 3300 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 3000 3100 50  0001 C CNN
F 3 "~" H 3000 3100 50  0001 C CNN
	1    3000 3100
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3900 2300 3900 3000
Wire Wire Line
	4000 2300 4000 3100
Wire Wire Line
	3800 3200 3200 3200
Wire Wire Line
	3700 3300 3200 3300
Text Notes 2900 3350 1    50   ~ 0
LCD Connector
Wire Wire Line
	3700 2300 3700 2800
Connection ~ 3700 2800
Wire Wire Line
	3800 2300 3800 2700
Connection ~ 3800 2700
Wire Wire Line
	3700 2800 7500 2800
Wire Wire Line
	3700 2800 3700 3300
Wire Wire Line
	3800 2700 3800 3200
Wire Wire Line
	3200 3000 3900 3000
Wire Wire Line
	3200 3100 4000 3100
Wire Wire Line
	4200 4550 4050 4550
Wire Wire Line
	5800 3400 5800 2700
Connection ~ 5800 3400
Text Notes 3350 3000 0    50   ~ 0
SCL
Text Notes 3350 3100 0    50   ~ 0
SCA
Text Notes 5100 3100 1    50   ~ 0
LockM1
Text Notes 5200 3100 1    50   ~ 0
LockM2
Text Notes 5300 3100 1    50   ~ 0
L2-
Text Notes 5400 3100 1    50   ~ 0
L1-
Text Notes 5500 3100 1    50   ~ 0
TurnM2
Text Notes 5600 3100 1    50   ~ 0
TurnM1
Text Notes 6300 3300 0    50   ~ 0
L2-
Text Notes 6300 3200 0    50   ~ 0
L1-
Text Notes 7200 3750 1    50   ~ 0
L2-
Text Notes 7300 3750 1    50   ~ 0
L1-
Text Notes 7600 3750 1    50   ~ 0
TurnM1
Text Notes 7700 3750 1    50   ~ 0
TurnM2
Text Notes 7800 3750 1    50   ~ 0
LockM1
Text Notes 7900 3750 1    50   ~ 0
LockM2
Text Label 3350 3300 0    50   ~ 0
GND
Text Label 3350 3200 0    50   ~ 0
+5V
Text Label 6200 2800 0    50   ~ 0
GND
Text Label 6300 3400 0    50   ~ 0
+5V
Text Label 5800 3100 1    50   ~ 0
+5V
Text Label 5200 5500 0    50   ~ 0
GND
Text Label 7500 3750 1    50   ~ 0
GND
Text Label 7400 3750 1    50   ~ 0
+5V
Text Label 4100 3100 1    50   ~ 0
Blue
Text Label 4200 3100 1    50   ~ 0
Yellow
Text Label 4300 3100 1    50   ~ 0
Green
Text Label 4400 3100 1    50   ~ 0
Red
Text Label 6200 2600 0    50   ~ 0
Switch
Wire Wire Line
	5400 2300 5400 3200
Wire Wire Line
	5300 2300 5300 3300
Wire Wire Line
	5300 3300 7200 3300
Wire Wire Line
	7900 4700 6900 4700
Wire Wire Line
	5500 2300 5500 4100
Wire Wire Line
	7300 3200 5400 3200
Wire Wire Line
	5600 3900 5900 3900
Wire Wire Line
	5500 4100 5900 4100
Wire Wire Line
	5200 4700 5900 4700
Wire Wire Line
	5200 2300 5200 4700
Wire Wire Line
	5100 4500 5900 4500
Wire Wire Line
	7700 2300 7700 4100
Wire Wire Line
	7800 2300 7800 4500
Wire Wire Line
	7900 2300 7900 4700
Wire Wire Line
	7500 2800 7500 5500
Wire Wire Line
	4400 2300 4400 4450
$Comp
L Connector_Generic:Conn_01x04 J5
U 1 1 5F7BA9F7
P 3000 3900
F 0 "J5" H 3050 3500 50  0000 R CNN
F 1 "Switches" H 3050 3600 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 3000 3900 50  0001 C CNN
F 3 "~" H 3000 3900 50  0001 C CNN
	1    3000 3900
	-1   0    0    1   
$EndComp
Wire Wire Line
	3200 4000 3700 4000
Wire Wire Line
	3700 4000 3700 3300
Connection ~ 3700 3300
Wire Wire Line
	3200 3900 5000 3900
Wire Wire Line
	5000 2600 5700 2600
Wire Wire Line
	5700 2600 5700 2300
Wire Wire Line
	5000 2300 5000 2500
Wire Wire Line
	5000 2500 4900 2500
Wire Wire Line
	4900 3800 3200 3800
Wire Wire Line
	4900 2300 4900 2400
Wire Wire Line
	4900 2400 4800 2400
Wire Wire Line
	4800 3700 3200 3700
Text Label 4550 2800 0    50   ~ 0
GND
Text Label 4550 2700 0    50   ~ 0
+5V
$Comp
L Switch:SW_Push SW3
U 1 1 5F984BCA
P 2500 3900
F 0 "SW3" H 2750 3950 50  0000 C CNN
F 1 "180" H 2150 3950 50  0000 R CNN
F 2 "DCC-Controlled-Kato-Turntable:SW_Tactile_Straight_7mm" H 2500 4100 50  0001 C CNN
F 3 "~" H 2500 4100 50  0001 C CNN
	1    2500 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 3700 2250 3700
Wire Wire Line
	2250 3700 2250 3800
Wire Wire Line
	2250 3800 2300 3800
Wire Wire Line
	2250 3800 2250 3900
Wire Wire Line
	2250 3900 2300 3900
Connection ~ 2250 3800
Wire Wire Line
	2250 3900 2250 4000
Connection ~ 2250 3900
Wire Wire Line
	3200 3700 2700 3700
Wire Wire Line
	3200 3800 2700 3800
Wire Wire Line
	3200 3900 2700 3900
Wire Wire Line
	2250 4000 3200 4000
Connection ~ 3200 3800
Connection ~ 3200 3900
Connection ~ 3200 3700
Connection ~ 3200 4000
Wire Wire Line
	4800 2400 4800 3700
Wire Wire Line
	4900 2500 4900 3800
Wire Wire Line
	5000 2600 5000 3900
Wire Wire Line
	3800 2700 5800 2700
$Comp
L Connector:Screw_Terminal_01x02 J3
U 1 1 5F21CFB4
P 8700 2100
F 0 "J3" V 9050 2050 50  0000 L CNN
F 1 "DCC" V 8950 2000 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 8700 2100 50  0001 C CNN
F 3 "~" H 8700 2100 50  0001 C CNN
	1    8700 2100
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 5F984375
P 2500 3800
F 0 "SW2" H 2750 3850 50  0000 C CNN
F 1 "RIGHT" H 2200 3850 50  0000 R CNN
F 2 "DCC-Controlled-Kato-Turntable:SW_Tactile_Straight_7mm" H 2500 4000 50  0001 C CNN
F 3 "~" H 2500 4000 50  0001 C CNN
	1    2500 3800
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5F983E31
P 2500 3700
F 0 "SW1" H 2750 3750 50  0000 C CNN
F 1 "LEFT" H 2150 3750 50  0000 R CNN
F 2 "DCC-Controlled-Kato-Turntable:SW_Tactile_Straight_7mm" H 2500 3900 50  0001 C CNN
F 3 "~" H 2500 3900 50  0001 C CNN
	1    2500 3700
	1    0    0    -1  
$EndComp
Text Notes 7900 2050 1    50   ~ 0
LockM2
Text Notes 7800 2050 1    50   ~ 0
LockM1
Wire Wire Line
	7200 3300 7200 5400
Wire Wire Line
	8500 5400 8500 5200
Wire Wire Line
	8400 5400 8500 5400
Wire Wire Line
	8300 5300 8300 5200
Wire Wire Line
	8200 5300 8300 5300
Wire Wire Line
	8400 5200 8400 5400
Wire Wire Line
	8200 5200 8200 5300
Wire Wire Line
	9200 4800 9200 5500
Wire Wire Line
	8100 4200 8100 4400
Wire Wire Line
	9100 4800 9200 4800
$Comp
L Transistor_Array:ULN2803A U2
U 1 1 5F1F80A7
P 8400 4800
F 0 "U2" V 8650 4000 50  0000 L CNN
F 1 "ULN2803A" V 8550 3750 50  0000 L CNN
F 2 "Package_DIP:DIP-18_W7.62mm_LongPads" H 8450 4150 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2803a.pdf" H 8500 4600 50  0001 C CNN
	1    8400 4800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8900 4350 8900 4400
Wire Wire Line
	8800 4350 8800 4400
Wire Wire Line
	8700 4350 8700 4400
Wire Wire Line
	8600 4350 8600 4400
Wire Wire Line
	8600 5250 8600 5200
Wire Wire Line
	8700 5250 8700 5200
Wire Wire Line
	8800 5250 8800 5200
Wire Wire Line
	8900 5250 8900 5200
NoConn ~ 8600 5250
NoConn ~ 8700 5250
NoConn ~ 8800 5250
NoConn ~ 8900 5250
Wire Wire Line
	7400 4200 8100 4200
Connection ~ 8100 4200
Wire Wire Line
	8100 4200 9350 4200
Wire Wire Line
	8200 4000 8200 4300
Wire Wire Line
	8200 4300 8300 4300
Wire Wire Line
	8300 4300 8300 4400
Connection ~ 8200 4300
Wire Wire Line
	8200 4300 8200 4400
Wire Wire Line
	8500 4300 8400 4300
Wire Wire Line
	8500 4300 8500 4400
Wire Wire Line
	8400 4300 8400 4400
Wire Wire Line
	8200 5300 7300 5300
Wire Wire Line
	7300 3200 7300 5300
Connection ~ 8200 5300
Wire Wire Line
	9200 5500 7500 5500
Connection ~ 7500 5500
Wire Wire Line
	7500 5500 6600 5500
Wire Wire Line
	8400 5400 7200 5400
Connection ~ 8400 5400
Wire Wire Line
	8100 2800 8100 3500
Wire Wire Line
	8000 2300 8000 2900
Wire Wire Line
	8450 3300 8000 3300
Wire Wire Line
	8450 3500 8100 3500
Wire Wire Line
	8450 4000 8200 4000
Connection ~ 8400 4300
Wire Wire Line
	8400 3900 8400 4300
Connection ~ 8000 2900
Wire Wire Line
	8000 2900 8000 3300
Wire Wire Line
	8400 3700 8400 3000
Wire Wire Line
	8400 3700 8450 3700
Wire Wire Line
	9300 3700 9350 3700
Wire Wire Line
	9350 3700 9350 3000
Wire Wire Line
	9450 3500 9450 2900
Wire Wire Line
	9300 3300 9550 3300
Wire Wire Line
	9550 3300 9550 2800
Connection ~ 9350 4000
Wire Wire Line
	9350 3000 8800 3000
Wire Wire Line
	9450 2900 8000 2900
Wire Wire Line
	9450 3500 9300 3500
Wire Wire Line
	9550 2800 8100 2800
Connection ~ 4850 5500
Connection ~ 6600 5500
Wire Wire Line
	6600 5300 6600 5500
Wire Wire Line
	4850 5500 6200 5500
Connection ~ 6200 5500
Wire Wire Line
	6200 5500 6300 5500
Connection ~ 6300 5500
Wire Wire Line
	6300 5500 6500 5500
Connection ~ 6500 5500
Wire Wire Line
	6500 5500 6600 5500
$EndSCHEMATC
