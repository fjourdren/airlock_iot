#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600);
  pinMode(9,OUTPUT); digitalWrite(9,HIGH);
  Serial.println("Enter AT commands:");
  mySerial.begin(9600);
  //38400 or 9600
  /*
  AT+UART?
  AT+UART=9600,0,0
  AT+ROLE=1 or 0 (Master :67 or Slave :08)
  AT+CMODE=0 on master to connect to a fixed address
  AT+BIND= ADDR slave module using ',' instead of ':'
  
  */
}

void loop()
{
  if (mySerial.available())  
  Serial.write(mySerial.read());
  
  if (Serial.available())  
  mySerial.write(Serial.read());
}

//+ADDR:98d3:31:f51c08
