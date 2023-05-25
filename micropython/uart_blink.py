from machine import Pin, UART
from time import sleep

# setup uart
uart0 = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1), bits=8, parity=None, stop=1)

pin  = Pin("LED", Pin.OUT)
sense = Pin(2, Pin.OUT)

txData = b'HeLlo World\n\r'

while True:
    pin.toggle()
    sense.toggle()
    sleep(1)
    uart0.write(txData)