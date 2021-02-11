import serial
import time

print("Start")
port = "COM3"  # This will be different for various devices and on windows it will probably be a COM port.
bluetooth = serial.Serial(port, 9600)  # Start communications with the bluetooth unit
print("Connected")
bluetooth.flushInput()  # This gives the bluetooth a little kick
for i in range(1):  # send 5 groups of data to the bluetooth
    print("Ping")
    bluetooth.write(b"opendoor1")  # These need to be bytes not unicode, plus a number
    input_data = bluetooth.readline()  # This reads the incoming data. In this particular example it will be the "Hello from Blue" line
    if input_data.decode() == 'okdoor1\r\n':
        print("Porte côté arduino ouverte")
        time.sleep(3)
        bluetooth.write(b"opendoor2")
        # input_data = bluetooth.readline()  # This reads the incoming data. In this particular example it will be the "Hello from Blue" line
        # if input_data.decode() == 'freedoor1\r\n':
        #     print("door1 is free")

    print(input_data.decode())  # These are bytes coming in so a decode is needed
    time.sleep(0.1)  # A pause between bursts
bluetooth.close()  # Otherwise the connection will remain open until a timeout which ties up the /dev/thingamabob
print("Done")
