
opts="-d tiny1617 -c /dev/ttyUSB0 -b 115200" 
set -x
while true; do
    # Write fuse 128 (0x80) - which is USERROW0
    pyupdi.py $opts -f obj/main.hex -fs 128:0x2a &&
            sleep 30
    sleep 3s
done
