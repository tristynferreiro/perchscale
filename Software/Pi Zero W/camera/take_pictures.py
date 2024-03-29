"""_summary_ Takes pictures and saves them to the specified file location.

    Usage:
        ./take_pictures.py

    Version Date: 07-02-2024
"""

# Import libraries
from picamera import PiCamera
from time import sleep

# Instantiate camera object
camera = PiCamera()

# Capture the images
# camera.start_preview()
for image_number in range(2):
    sleep(2) # Change according to how much time camera needs to adjust to ambient light levels
    camera.capture('/home/pi/Desktop/image%s.jpg',image_number)
# camera.stop_preview()
