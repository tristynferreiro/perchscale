"""_summary_ Records a video and saves it to the specified file location.

    Usage:
        ./record_video.py

    Version Date: 07-02-2024
"""

# Import libraries
from picamera import PiCamera
from time import sleep

# Instantiate camera object
camera = PiCamera()

# Begin recording video
camera.start_recording('/home/pi/Desktop/video.mp4')
sleep(5) # adjust based on how long the video should be.
camera.stop_recording()
