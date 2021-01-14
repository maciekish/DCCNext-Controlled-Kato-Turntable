# DCCNext-Controlled-Kato-Turntable
DCCNext based PCB and additional PCB to control a Kato 20-283 Unitrack Turntable<BR>
v1.48 uses Littfinski DatenTechnik (LDT) TurnTable Decoder TT-DEC standard.<BR>
v2.07 uses Fleischmann Turntable Controller 6915 standard.<BR>
  
2020-01-17: v2.07 - New Button functionality:
Button T180  (Shortpress) = Turn 180
Button T180  (Longpress)  = Store current position as track 1
Button Right (Shortpress) = Turn 1 Step ClockWise
Button Right (Longpress)  = Reset Turntable Lock
Button Left  (Shortpress) = Turn 1 Step Counter ClockWise
Button Left  (Longpress)  = Set Turntable Lock


Schematics created in KiCAD v5.1.6<BR>
Pictures from the additional PCB (top and bottom)<BR>
<HR>
Components shopping list:<BR>
Main Components:<BR>
DCCNext:<BR>
https://www.arcomora.com/dccnext/<BR>
Order your own DCCNext complete package (choose the combiset (DCCNext + USB interf. + Box) here:<BR>
https://www.arcomora.com/reservation/<BR>
<BR>
DCCNext-Controlled-Kato-Turntable PCB:<BR>
PCB version 1.0 is still available. Can be requested by email to me (only for people in The Netherlands)<BR>
<BR>
Resistors: 4x 1kOhm 1/8 Watt
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/4000695402017.html<BR>
Switches: 3x tactile switch 7 mm
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32912104842.html<BR>
Relay: 1x HFD2_005-S-L2-D
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32983182219.html<BR>
LED 3mm: 1x Red, 1x Green, 1x Yellow, 1x Blue
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32848822296.html<BR>
IC1: H-Bridge: L293D DIP-16
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/33007327277.html<BR>
IC1-socket: DIP-16
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/4001048163251.html<BR>
IC2: Darlington Array: ULN2803A DIP-18
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32959439299.html<BR>
IC2-socket: DIP-18
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/4001048163251.html<BR>
Pinheaders 2.54mm: 2x4 pins, 1x8 pins, 1x10 pins
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32759414721.html<BR>
Kato Turntable connector: S08B-PASK-2(LF)(SN)
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/1005001693841761.html<BR>
Screwconnector 5.08mm: 1x2 pins
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/1000006518504.html<BR>
Arduino Shield Header 10 pins
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32671697975.html<BR>
Arduino Shield Header 8 pins
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/32677173846.html<BR>
<BR>
Optional:<BR>
Arduino I2C LCD 20x4 rows
&nbsp&nbsp&nbsp&nbsp
https://www.aliexpress.com/item/4001135515638.html<BR>

<HR>
http://www.katousa.com/N/Unitrack/Turntable.html<BR>
http://www.katousa.com/images/unitrack/20-283.jpg<BR>
<BR>
Followed the standard from Littfinski DatenTechnik (LDT) TurnTable Decoder TT-DEC<BR>
https://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:tt-dec<BR>
<BR>
Fleischmann Turntable Controller 6915<BR>
https://www.fleischmann.de/en/product/3915-0-0-0-0-0-0-004002005-0/products.html<BR>
ESU ECoS:<BR>
http://www.esu.eu/en/products/digital-control/<BR>

Uhlenbrock Intellibox:<BR>
https://www.uhlenbrock.de/de_DE/produkte/digizen/index.htm<BR>

Model railroad control program called: iTrain:<BR>
https://www.berros.eu/en/itrain/<BR>
