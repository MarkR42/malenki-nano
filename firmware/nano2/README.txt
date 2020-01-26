AFHDS2A protocol

* Uses the A7105 radio, 
    bitrate (TBC)
    FEC (forward error correction) - must be ENABLED
    Packet length: 37 bytes (must be correct)
    Radio ID is 0x54, 0x75, 0xc5, 0x2a

* Packets sent about every 3.8ms 
    (3850us according to Multiprotcol)

* channel hops - the tx channel ID is sent in the bind packets.
    bind packets contain the channel hopping IDs.

    rx channel = tx channel - 1

-- Packet types:

The first byte contains the packet type.

0x58 = Sticks packet
0xbb = Bind1
0xbc = Bind2

In Bind mode, channels 0x0d or 0x8c are used (alternately?)

There are a few other types of packet too.

-- Packet format:

offset 0 = CMD TYPE (1 byte)
offset 1 = tx ID (4 bytes)
offset 5 = Rx ID (4 bytes) (usually all 0, we ignore it anyway)
    - this is used for telemetry, if multiple rxs
        are sending telemetry back.

-- Bind packet format:

    offset 9-10 = unknown

    offset 11 = Hopping channels (16 bytes) - a list of channels
        the rest of the packet - unknown or unused.

Example bind packets:
bc c4 1b b4 c4 ff ff ff ff 00 00 6b 79 42 19 49 53 5b 24 6f 3d 2f 57 12 2a 74 80 01 80 ff ff ff ff ff ff ff ff 
bb c4 1b b4 c4 ff ff ff ff 01 00 6b 79 42 19 49 53 5b 24 6f 3d 2f 57 12 2a 74 80 ff ff ff ff ff ff ff ff ff ff


-- Sticks packet format
offset 9 - sticks, 2 bytes per channel.
14 channels = 28 bytes.

Example sticks packet:
58 c4 1b b4 c4 25 f3 98 41 dc 05 dc 05 e8 03 dc 05 e8 03 e8 03 e8 03 dc 05 dc 05 dc 05 dc 05 dc 05 dc 05 dc 85

-- Other packets:

Packet seen when turning tx off:
56 c4 1b b4 c4 25 f3 98 41 ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f ff 0f


