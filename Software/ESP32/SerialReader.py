import serial

ser = serial.Serial('COM7', baudrate=9600)  # Adjust 'COMx' and baudrate

try:
    with open('output.txt', 'w') as file:
        while True:
            # Read a line from the serial port
            line = ser.readline().decode().strip()
            
            # Write the line to the file
            file.write(line + '\n')
            print(line)

except KeyboardInterrupt:
    print("Serial reading terminated by user.")
    ser.close()





