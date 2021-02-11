#include "SoftwareSerial.h"
SoftwareSerial serial_connection(2, 3);//Create a serial connection with TX and RX on these pins
SoftwareSerial serial_master(10, 11);

#define BUFFER_SIZE 64//This will prevent buffer overruns.
char inData[BUFFER_SIZE];//This is a character buffer where the data sent by the python script will go.
char inChar=-1;//Initialie the first character as nothing
int i=0;//Arduinos are not the most capable chips in the world so I just create the looping variable once

int state = 0;

const int motor_pwm = 6;
const int motor_in_1 = 5; 
const int motor_in_2 = 4;

const long interval = 100; 
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

bool check_bluetooth = false;

void setup()
{
  Serial.begin(9600);//Initialize communications to the serial monitor in the Arduino IDE
  serial_connection.begin(9600);//Initialize communications with the bluetooth module
  serial_connection.println("Ready!!!");//Send something to just start comms. This will never be seen.

  serial_master.begin(9600);
  serial_master.println("Ready!!!");
  // Init du moteur
  pinMode(motor_pwm,OUTPUT); 
  pinMode(motor_in_1,OUTPUT); 
  pinMode(motor_in_2,OUTPUT);
  
  pinMode(LED_BUILTIN, OUTPUT);

  // Serial.println("Started");//Tell the serial monitor that the sketch has started.

}
void loop()
{
  currentMillis = millis();

    // instead of using delay and to avoid bouncing
  if (currentMillis - previousMillis >= interval) {
    // save the last time it happened
    previousMillis = currentMillis;
    check_bluetooth = !check_bluetooth;

    if (check_bluetooth) 
    {
      manageDataSlaveHC06();
    } 
    else 
    {
      manageDataMasterHC05();
    }
    
  }
}

void manageDataMasterHC05(){
  /*
  serial_master.listen();
  if(serial_master.available() > 0)
  {
    state = serial_master.read();
  }
  if(state == '1')
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);    
  }
  *>=/
  /*
  serial_master.listen();
  //This will prevent bufferoverrun errors
  byte byte_count=serial_master.available();//This gets the number of bytes that were sent by the python script
  if(byte_count)//If there are any bytes then deal with them
  {
    int first_bytes=byte_count;//initialize the number of bytes that we might handle. 
    int remaining_bytes=0;//Initialize the bytes that we may have to burn off to prevent a buffer overrun
    if(first_bytes>=BUFFER_SIZE-1)//If the incoming byte count is more than our buffer...
    {
      remaining_bytes=byte_count-(BUFFER_SIZE-1);//Reduce the bytes that we plan on handleing to below the buffer size
    }
    for(i=0;i<first_bytes;i++)//Handle the number of incoming bytes
    {
      inChar=serial_master.read();//Read one byte
      inData[i]=inChar;//Put it into a character string(array)
    }
    inData[i]='\0';//This ends the character array with a null character. This signals the end of a string
    if(String(inData) == "okdoor2")//This could be any motor start string we choose from the python script
    {
      serial_connection.listen();
      serial_connection.println("freedoor1");
    }
    else 
    {
      serial_connection.listen();
      serial_connection.println("cannot process data");
    }
    
    for(i=0;i<remaining_bytes;i++)//This burns off any remaining bytes that the buffer can't handle.
    {
      inChar=serial_connection.read();
    }

  }
  */
}

void manageDataSlaveHC06(){
  serial_connection.listen();
  //This will prevent bufferoverrun errors
  byte byte_count=serial_connection.available();//This gets the number of bytes that were sent by the python script
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
      inChar=serial_connection.read();//Read one byte
      inData[i]=inChar;//Put it into a character string(array)
    }
    inData[i]='\0';//This ends the character array with a null character. This signals the end of a string
    if(String(inData) == "opendoor1")//This could be any motor start string we choose from the python script
    {
      openDoor1();
      serial_connection.listen();
      serial_connection.println("okdoor1");
    }
    else if(String(inData) == "opendoor2")
    {
      closeDoor1();
      serial_connection.listen();
      serial_connection.print("closed_door");
      
      serial_master.listen();
      serial_master.print("opendoor2");
      serial_connection.listen();
    }
    else 
    {
      serial_connection.println("cannot process data");
    }
    
    for(i=0;i<remaining_bytes;i++)//This burns off any remaining bytes that the buffer can't handle.
    {
      inChar=serial_connection.read();
    }
  }
}


void openDoor1(){
  // Serial.println("********* Start Motor *********");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,LOW); analogWrite(motor_pwm,255);
  delay(2000);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,HIGH); 
  delay(200);
}


void closeDoor1()
{
  // Serial.println("********* STOP Motor *********");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(motor_in_1,LOW); digitalWrite(motor_in_2,HIGH); 
  delay(2000);
  
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(motor_in_1,HIGH); digitalWrite(motor_in_2,HIGH); 
  delay(200);
}
