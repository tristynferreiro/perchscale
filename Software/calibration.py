import RPi.GPIO as gpio
import time
from hx711 import HX711

gpio.setmode(gpio.BCM)

hx = HX711(dout_pin=5,pd_sck_pin=6)

""" Calibration script """ 

# Ask user for known weight of calibration object
print("Starting calibration....")
known_weight = float(input("What is the weight of the object used (in grams) for calibration?"))

# Take the reading
hx.zero()
print("Place the object on the scale")
time.sleep(3)
reading = hx.get_raw_data_mean(1) - hx.get_current_offset()

# Calculate calibration factor
calibration_factor = known_weight/reading

# Save the calibration factor to file
file_path = 'calibration_factor.txt'  # Replace with the desired path for the new file

# Open the file in write mode ('w')
with open(file_path, 'w') as file:
    content = str(calibration_factor)
    file.write(content)

print("Calibration successful."+str(calibration_factor))


