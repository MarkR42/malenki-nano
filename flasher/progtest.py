import logging
import pymcuprog
import serial
import serial.tools.list_ports

from pymcuprog.backend import SessionConfig
from pymcuprog.toolconnection import ToolUsbHidConnection, ToolSerialConnection
from pymcuprog.backend import Backend

# Change to INFO or DEBUG for lots of info
logging.basicConfig(level=logging.WARNING)

##########
    
def do_prog(backend, firmware_image):
    # block_size should really be a multiple of the page size
    block_size = 64
    offset = 0
    while (offset < len(firmware_image)):
        chunk = firmware_image[offset:offset + block_size]
        # pymcuprog expects a list of ints
        chunk_ints = list(map(int, chunk))
        print("Writing offset {}/{}".format(offset, len(firmware_image)))
        backend.write_memory(chunk_ints, offset_byte=offset)
        offset += block_size
    # Verify
    verify_ok = backend.verify_memory(firmware_image, memory_name='flash', offset_byte=0) 
    if not verify_ok:
        raise Exception("Failed to verify after prog")
    print("Verify ok")

#################

print("Trying to find serial ports...")
all_ports = []
for p in serial.tools.list_ports.comports():
    print("{} - {}".format(p.device, p.description))
    all_ports.append(p.device)
    
all_ports.sort()
if len(all_ports) < 1:
    raise Exception("Failed to find a serial port")
desired_port = all_ports[0]
print("Desired port {}".format(desired_port))

sc = SessionConfig("attiny1614")

transport = ToolSerialConnection(serialport=desired_port)
backend = Backend()
backend.connect_to_tool(transport)
print("Starting session...")
backend.start_session(sc)
print("Verifying..")
# backend.
firmware_image = open('firmware.bin', 'rb').read()
verify_ok = backend.verify_memory(firmware_image, memory_name='flash', offset_byte=0) 
if verify_ok:
    print("Verified OK")
else:
    print("Verify fail - will try to flash")
    # Erase
    print("Erasing")
    backend.erase()
    do_prog(backend, firmware_image)
