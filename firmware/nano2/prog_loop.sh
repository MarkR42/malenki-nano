
opts="-d tiny1617 -c /dev/ttyUSB0 -b 115200" 
set -x
while true; do
    pyupdi.py $opts --progmode -f obj/main.hex &&
        pyupdi.py $opts -fs 128:0x2a &&
            sleep 30
    sleep 3s
done
