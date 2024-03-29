import RPi.GPIO as gpio
import time
from hx711 import HX711

gpio.setmode(gpio.BCM)

hx = HX711(dout_pin=5,pd_sck_pin=6)

hx.zero()


while True:
	reading = hx.get_raw_data_mean(1) - hx.get_current_offset()
	#reading = hx.get_data_mean(1)
	print(reading) 
	#time.sleep(2)



	
