import cv2
import serial

import logging as log
import datetime as dt
from time import sleep

face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
# eye_cascade = cv2.CascadeClassifier('haarcascade_eye.xml')
# https://github.com/Db1998/Facial-Distance
# https://github.com/LessonStudio/Arduino_Bluetooth

log.basicConfig(filename='webcam.log', level=log.INFO)

video_capture = cv2.VideoCapture(0)
anterior = 0
dst = 0.
text_dst = ''
data_can_be_sent = True
door1_open = False

print("Start")
port = "COM3"  # This will be different for various devices and on windows it will probably be a COM port.
bluetooth = serial.Serial(port, 9600)  # Start communications with the bluetooth unit
print("Connected")
bluetooth.flushInput()  # This gives the bluetooth a little kick

while True:
    if not video_capture.isOpened():
        print('Unable to load camera.')
        sleep(5)
        pass

    # Capture frame-by-frame
    ret, frame = video_capture.read()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # minNeighbors --> if it is in neighborhood of other rectangles then it is ok
    # minSize / maxSize
    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30)
    )

    # Draw a rectangle around the faces
    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        # dst = 6421 / w
        dst = 6421 * 1.5 / w
        text_dst = '%.2f' % dst
        font = cv2.FONT_HERSHEY_SIMPLEX
        cv2.putText(frame, str(text_dst), (x, y - 10), font, 1, (0, 50, 250), 1, cv2.LINE_AA)

    if anterior != len(faces):
        anterior = len(faces)
        log.info("faces: " + str(len(faces)) + " at " + str(dt.datetime.now()) + " | distance: " + str(dst))

    # if the data can be sent and there is exactly one face : send it
    if len(faces) == 1 and dst <= 40:
        log.info("data is sent to the bluetooth card")
        data_can_be_sent = False
        bluetooth.write(b"opendoor1")  # These need to be bytes not unicode, plus a number
        input_data = bluetooth.readline()  # This reads the incoming data. In this particular example it will be the "Hello from Blue" line
        log.info(input_data.decode())
        if input_data.decode() == 'okdoor1\r\n':
            log.info("okdoor1")
            door1_open = True
    if door1_open and len(faces) != 1:
        bluetooth.write(b"opendoor2")
        door1_open = False

    # Display the resulting frame
    cv2.imshow('Video', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    # Display the resulting frame
    cv2.imshow('Video', frame)

# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()
bluetooth.close()
log.info("Done")
