Malenki-ESC
=======

![alt text](pcb.png "Our PCB")

Malenki-ESC is a DC electronic speed controller for small robots. An ESC converts signals from
e.g. a radio receiver into higher current pulses for motors. 

* Power: 6-13 volts, from a lipo 2S or 3S pack
* Three reversible DC motor drivers: two for tank-steering and one for "weapon"
* Single wire input, supporting either PPM or serial protocols s-bus or i-bus
* 5v power output / regulator / "BEC" to power a radio (not suitable for a large servo)
* Battery level monitor
* Some status LEDs
* Failsafe operation (turns off motors if signal lost)

The main focus is on a very tiny footprint to fit into small devices.

