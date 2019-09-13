A7105-based rx/esc
------------------

The Malenki-Nano is a combined rc radio receiver with 3 channel speed controller.

The radio protocol is AFHDS-2A which is used by Turnigy Evolution and other
Turnigy branded radio gear, some of which is branded "Flysky" but it's not
compatible with the older Flysky gear.

---

How does it even work?

There is no bind plug and no servo connectors etc.

There is only a 3v3 internal regulator (no 5v)

FIRMWARE IDEAS:
---
The radio led is D1 in the schematic and connected to the a7105 GIO2 pin.

1. BIND MODE
When we first start up, it will have no memorised transmitter ID, so it almost
immediately goes into bind mode.

In bind mode all motors are off and the radio light flashes quickly. We wait to receive bind packets
from a transmitter.

Once enough bind packets are received, we decide we have a good signal, and store the tx ID and
hopping frequency info.

Then it's bound. Once it's bound it goes into normal (rx) mode.

2. NO SIGNAL MODE

If binding is complete, but we get no packets of stick data, there is no signal.

All motors are off and we wait on one channel (maybe 1st channel) for any data from the tx.
Alternatively, we could slowly go around the channels, in case one channel has terrible 
interference.

Radio LED must be off.

3. SIGNAL RECEIVED

After a signal is received, we need to keep hopping around the channels every packet 
received, or if a packet is missed. If we lose too many packets then we get out of sequence,
and have to return to no signal mode.

Wait a short time after signal received (e.g. 250ms) before enabling the drive.

Radio LED should be ON constantly when signal received.

4. SIGNAL LOST

If signal is lost we should turn the motors off.

5. REBINDING

If there is no signal - any signal - from the memorised transmitter ID, after some time limit,
say 60 seconds, we will automatically enter bind mode. This enables binding to a different
tx without reflashing or resetting the device. Only do this after power on. If there was
a signal earlier, we don't automatically enter bind mode.

If we enter rebind mode, then get a signal from the old transmitter, we can just go back to there,
so it should be able to automatically pick up the old tx if entering bind mode.


---


This directory will contain the firmware (embedded software) for 
our microcontroller (attiny1614 at time of writing)

