# main.py -- put your code here!
import pyb
import dcfurs
from pyb import Timer

print("Hello World!")
dcfurs.matrix_init()
def cylon(x):
    dcfurs.clear()
    for y in range(0, 7):
        dcfurs.set_pixel(x-1, y, 10)
        dcfurs.set_pixel(x, y, 200)
        dcfurs.set_pixel(x+1, y, 10)

def mtick(timer):
    dcfurs.matrix_loop()

## Run the main test pattern
print("Starting test pattern...")
mtimer = pyb.Timer(4, freq=16000, callback=mtick)
while True:
    for x in range(0, 17):
        cylon(x)
        pyb.delay(75)
    for x in range(0, 17):
        cylon(17-x)
        pyb.delay(75)
