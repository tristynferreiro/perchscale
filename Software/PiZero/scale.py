import RPi.GPIO as gpio
import time
from hx711 import HX711

def save_to_file(counter, value, filename='readings.txt'):
    with open(filename, 'a') as file:
        file.write(f"{counter}, {value}\n")

gpio.setmode(gpio.BCM)

hx = HX711(dout_pin=5,pd_sck_pin=6)

hx.zero()

# Open the file in read mode ('r')
with open('Calibration_factor.txt', 'r') as file:
    # Read the entire content of the file into a string
    file_content = file.read()

# Now, 'file_content' contains the content of the file
# Do whatever you need to do with the content
print("Content of the file:", file_content)

# If the content is numeric, you may want to convert it to a specific type
# For example, if it's an integer
try:
    Calibration_factor = float(file_content)
    print("Converted value to a float:", Calibration_factor)
except ValueError:
    print("The content is not a valid float.")

counter = 0

while True:
    reading = hx.get_raw_data_mean(1) - hx.get_current_offset()
    calibrated_reading = reading*Calibration_factor
    print("Scale reading: ", reading)
    print("Scale reading with calibration: ", calibrated_reading)
    save_to_file(counter, calibrated_reading) 
    counter += 1
	#time.sleep(2)






	
