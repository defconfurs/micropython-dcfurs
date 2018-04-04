# main.py -- put your code here!
import pyb
from pyb import Pin
print("Hello World!")

## Setup the row drivers
rows = [
    Pin('ROW1', Pin.OUT_PP),
    Pin('ROW2', Pin.OUT_PP),
    Pin('ROW3', Pin.OUT_PP),
    Pin('ROW4', Pin.OUT_PP),
    Pin('ROW5', Pin.OUT_PP),
    Pin('ROW6', Pin.OUT_PP),
    Pin('ROW7', Pin.OUT_PP)
]
for r in rows:
    r.high()

## Setup the column pins
cols = [
    Pin('COL0', Pin.OUT_PP),
    Pin('COL1', Pin.OUT_PP),
    Pin('COL2', Pin.OUT_PP),
    Pin('COL3', Pin.OUT_PP),
    Pin('COL4', Pin.OUT_PP),
    Pin('COL5', Pin.OUT_PP),
    Pin('COL6', Pin.OUT_PP),
    Pin('COL7', Pin.OUT_PP),
    Pin('COL8', Pin.OUT_PP),
    Pin('COL9', Pin.OUT_PP),
    Pin('COL10', Pin.OUT_PP),
    Pin('COL11', Pin.OUT_PP),
    Pin('COL12', Pin.OUT_PP),
    Pin('COL13', Pin.OUT_PP),
    Pin('COL14', Pin.OUT_PP),
    Pin('COL15', Pin.OUT_PP),
    Pin('COL16', Pin.OUT_PP),
    Pin('COL17', Pin.OUT_PP),
]
for c  in cols:
    c.high()

def deselect_all():
    for x in rows:
        x.high()

def select_row(num):
    deselect_all()
    rows[num].low()

## Run the main test pattern
print("Starting test pattern...")
i = 0
while True:
    select_row(i % len(rows))
    for c in cols:
        c.low()
        pyb.delay(100)
    pyb.delay(1000)
    for c in cols:
        c.high()
        pyb.delay(100)
    i += 1
