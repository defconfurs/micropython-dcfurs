# main.py -- put your code here!
import pyb
import dcfurs
import random
from pyb import Pin
from pyb import Timer
from pyb import Accel

print("Hello World!")
dcfurs.matrix_init()
dcfurs.clear()
def cylon(x):
    dcfurs.clear()
    for y in range(0, 7):
        dcfurs.set_pixel(x-1, y, 10)
        dcfurs.set_pixel(x, y, 200)
        dcfurs.set_pixel(x+1, y, 10)

def owo():
    dcfurs.clear()
    ## Draw the Oh's
    for x in range(1,6):
        dcfurs.set_pixel(0,x,256)
        dcfurs.set_pixel(4,x,256)
        dcfurs.set_pixel(13,x,256)
        dcfurs.set_pixel(17,x,256)
    for x in range(1,4):
        dcfurs.set_pixel(x,0,256)
        dcfurs.set_pixel(x,6,256)
    for x in range(14,17):
        dcfurs.set_pixel(x,0,256)
        dcfurs.set_pixel(x,6,256)
    ## What's this?
    dcfurs.set_pixel(6,2,256)
    dcfurs.set_pixel(6,3,256)
    dcfurs.set_pixel(7,4,256)
    dcfurs.set_pixel(8,3,256)
    dcfurs.set_pixel(9,3,256)
    dcfurs.set_pixel(10,4,256)
    dcfurs.set_pixel(11,2,256)
    dcfurs.set_pixel(11,3,256)

def boop():
    dcfurs.clear()
    # Gimmie a B!
    for x in range(0,7):
        dcfurs.set_pixel(1, x, 256)
    dcfurs.set_pixel(2,0,256)
    dcfurs.set_pixel(3,0,256)
    dcfurs.set_pixel(4,1,256)
    dcfurs.set_pixel(4,2,256)
    dcfurs.set_pixel(2,3,256)
    dcfurs.set_pixel(3,3,256)
    dcfurs.set_pixel(4,4,256)
    dcfurs.set_pixel(4,5,256)
    dcfurs.set_pixel(2,6,256)
    dcfurs.set_pixel(3,6,256)
    # And an Oh!
    dcfurs.set_pixel(7,0,256)
    dcfurs.set_pixel(7,4,256)
    for x in range(1,4):
        dcfurs.set_pixel(6,x,256)
        dcfurs.set_pixel(8,x,256)
    # And an another Oh!
    dcfurs.set_pixel(10,0,256)
    dcfurs.set_pixel(10,4,256)
    for x in range(1,4):
        dcfurs.set_pixel(9,x,256)
        dcfurs.set_pixel(11,x,256)
    # Gimmie a P!
    for x in range(0,7):
        dcfurs.set_pixel(13, x, 256)
    dcfurs.set_pixel(14,0,256)
    dcfurs.set_pixel(15,0,256)
    dcfurs.set_pixel(16,1,256)
    dcfurs.set_pixel(16,2,256)
    dcfurs.set_pixel(14,3,256)
    dcfurs.set_pixel(15,3,256)

def rain_fall(fbuf):
    ## For each pixel in the buffer:
    y = len(fbuf)-1
    while y >= 0:
        row = fbuf[y]
        for x in range(0,len(row)):
            px = row[x]
            if ((px >= 255) and (y < (len(fbuf)-1))):
                fbuf[y+1][x] = px
            row[x] = px >> 2
        y -= 1
    ## Redraw the display
    for y in range(0,len(fbuf)):
        row = fbuf[y]
        for x in range(0, len(row)):
            dcfurs.set_pixel(x, y, row[x])

def rain_new(fbuf):
    row = fbuf[0]
    row[random.randint(0, len(row)-1)] = 255

## Matrix 'rain' animation
def matrix_rain():
    fbuf = [bytearray(18),bytearray(18),bytearray(18),bytearray(18),bytearray(18),bytearray(18),bytearray(18)]
    while True:
        rain_new(fbuf)
        for x in range(0,3):
            rain_fall(fbuf)
            pyb.delay(150)

def mtick(timer):
    dcfurs.matrix_loop()

## Setup the Accelerometer for tap detection
imu = pyb.Accel()
imu.write(0x7, 0x00)    # Switch to standby mode
imu.write(0x8, 0x00)    # Set sampling rate to 120Hz
imu.write(0x6, 0x04)    # Enable tap detection interrupt
imu.write(0x9, 0x0f)    # Set tap threshold to 15 counts.
imu.write(0x7, 0xc1)    # Set push-pull active-high interrupt, back to active mode.
wkup = pyb.Pin('MMA_INT', Pin.IN)

## Print some accelerometer debug at the end of each loop
def acceldebug():
    print(" wkup=" + hex(wkup.value()) + " tilt=" + hex(imu.read(0x3)))
    #for z in range(0, 10):
    #    print("     regs: " + hex(imu.read(z)))

## Run the main test pattern
print("Starting test pattern...")
mtimer = pyb.Timer(4, freq=16000, callback=mtick)

## Wait for a tap event.
owo()
while not wkup.value():
    pyb.delay(100)
boop()
pyb.delay(3000)

## Run the show.
while True:
    for x in range(0, 17):
        cylon(x)
        pyb.delay(75)
        acceldebug()
    for x in range(0, 17):
        cylon(17-x)
        pyb.delay(75)
        acceldebug()
