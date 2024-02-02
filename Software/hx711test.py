import RPi.GPIO as gpio
from hx711 import HX711

gpio.setmode(gpio.BCM)

hx = HX711(dout_pin=5,pd_sck_pin=6)

hx.zero()

while True:
	reading = hx.get_data_mean()
	print(reading) 
