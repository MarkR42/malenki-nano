Malenki-ESC
=======

[logo]:(pcb.png)

Malenki-ESC is a DC electronic speed controller for small robots. An ESC converts signals from
e.g. a radio receiver into higher current pulses for motors. 

It specifications are not yet fixed, but the plan is:

* Power: 6-13 volts, from a lipo 2S or 3S pack
* Three reversible DC motor drivers: two for tank-steering and one for "weapon"
* Single wire input, supporting either PPM or maybe serial protocols like s-bus or i-bus
* 5v power output / regulator / "BEC" to power a radio (not suitable for a large servo)
* Battery level monitor
* Some status LEDs
* Failsafe operation (turns off motors if signal lost)

The main focus is on a very tiny footprint to fit into small devices.

