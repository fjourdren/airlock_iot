#include "SoftwareSerial.h"
SoftwareSerial serial_slave(10, 11);//Create a serial connection with TX and RX on these pins
#define BUFFER_SIZE 64//This will prevent buffer overruns.
char inData[BUFFER_SIZE];//This is a character buffer where the data sent by the python script will go.
char inChar=-1;//Initialie the first character as nothing
int count=0;//This is the number of lines sent in from the python script
int i=0;//Arduinos are not the most capable chips in the world so I just create the looping variable once

const int motor_pwm = 6;
const int motor_in_1 = 5; 
const int motor_in_2 = 4;

const long interval = 100; 
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// defines pins numbers
const int trigPin = 3;
const int echoPin = 9;

const int ledPin = 13;

long duration;
int distance;

bool check_bluetooth = false;
bool door_ready = false;

void setup()
{
  Serial.begin(9600);//Initialize communications to the serial monitor in the Arduino IDE
  serial_slave.begin(9600);
  
  // Init du moteur
  pinMode(motor_pwm,OUTPUT); 
  pinMode(motor_in_1,OUTPUT); 
  pinMode(motor_in_2,OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  Serial.println("Started");//Tell the serial monitor that the sketch has started.

}
void loop()
{
  currentMillis = millis();

  // instead of using delay and to avoid bouncing
  if (currentMillis - previousMillis >= interval) {
    // save the last time it happened
    previousMillis = currentMillis;
    check_bluetooth = !check_bluetooth;
    // if the LED is off turn it on and vice-versa:
    if (check_bluetooth) 
    {
      manageDataBluetooth();
    } 
    else 
    {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(echoPin, HIGH);
      // Calculating the distance
      distance= duration*0.034/2;
      // Prints the distance on the Serial Monitor
      Serial.print("Distance: ");
      Serial.println(distance);
      if(door_ready)
      {
        digitalWrite(ledPin, HIGH);
        if(distance > 5 && distance < 40){
          door_ready = false;
          closeDoor();
          serial_slave.print(okdoor2);
        }
        
      }
    }
  }
  
}

void manageDataBluetooth(){
   byte byte_count=serial_slave.available();//This gets the number of bytes that were sent by the python script
  if(byte_count)//If there are any bytes then deal with them
  {
    Serial.println("Incoming Data");//Signal to the monitor that something is happening
    int first_bytes=byte_count;//initialize the number of bytes that we might handle. 
    int remaining_bytes=0;//Initialize the bytes that we may have to burn off to prevent a buffer overrun
    if(first_bytes>=BUFFER_SIZE-1)//If the incoming byte count is more than our buffer...
    {
      remaining_bytes=byte_count-(BUFFER_SIZE-1);//Reduce the bytes that we plan on handleing to below the buffer size
    }
    for(i=0;i<first_bytes;i++)//Handle the number of incoming bytes
    {
      inChar=serial_slave.read();//Read one byte
      inData[i]=inChar;//Put it into a character string(array)
    }
    inData[i]='\0';//This ends the character array with a null character. This signals the end of a string
    Serial.println(String(inData));
    if(String(inData).startsWith("opendoor"))
    {
      Serial.println("OPENDOOR");
      openDoor();
      door_ready = true;
    }
    for(i=0;i<remaining_bytes;i++)//This burns off any remaining bytes that the buffer can't handle.
    {
      inChar=serial_slave.read();
    }
  }
}

void openDoor(){
      Serial.println("********* Start Motor *********");
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,LOW); analogWrite(motor_pwm,255);
      delay(2000);
      
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,HIGH); 
      delay(200);
}


void closeDoor()
{
  Serial.println("********* STOP Motor *********");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(motor_in_1,LOW); digitalWrite(motor_in_2,HIGH); 
  delay(2000);
  
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,HIGH); 
  delay(200);
}
