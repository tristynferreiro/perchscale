import smbus
import time
from datetime import datetime, timedelta
import SDL_DS3231
from os.path import exists

### Function to create a file object with the input function name
def createFile(filename):
    time_file = open(filename,"w")
    return time_file

### Function to write to an existing file. If not exist, create
def writeToFile(filename):
    if exists(filename):
        time_file = open(filename,"w")
        return time_file
    createFile(filename)
    writeToFile(filename)

# Set file to write time data to
file_time = writeToFile("time.txt")
# Set the number of values to take (Testing line)
vals = int (input("How many readings should be taken?: "))
# Every how many seconds (Testing line)
sleep_time = int (input("Every how many seconds?: "))
i=0

while i<=vals:
    # Read the RTC I2C address using library
    ds3231 = SDL_DS3231.SDL_DS3231(1,0x68)
    t = ds3231.read_datetime() # Get UTC time
    t = t + timedelta(hours=2) # Convert to SAST
    print(t)
    # Convert to string of desired format
    t_str = t.strftime("%m/%d/%Y, %H:%M:%S")
    # Write to file
    file_time.write(t_str)
    time.sleep(sleep_time)
    i =i+1
# Close file
file_time.close()
