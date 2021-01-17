#include <SoftwareSerial.h>


// hc05 pins
#define BT_RX_PIN 10
#define BT_TX_PIN 11
#define baudrate 38400

// leds pin
#define LED_MASTER_PIN 12
#define LED_SLAVE_PIN 13
#define LED_DETECT_PERSON_PIN 14

// touch sensor pin
#define TOUCH_SENSOR_PIN 15

// motor pin
#define MOTOR_PWM_PIN 2
#define MOTOR_PIN1 3
#define MOTOR_PIN2 4

// init hc05 vars
String msg;
SoftwareSerial hc05(rxPin, txPin);

// reserved var (airlock processing)
bool reservation_asked = false; // ask if the card has ask

// card's possible states on the airlock
int reserved_UNKNOWN = 0;
int reserved_MASTER = 1;
int reserved_SLAVE = 2;

// card's current state on the airlock
int reserved_state = 0;


void setup() {
    // enable bluetooth hc05 in config mode
    // pinMode(9,OUTPUT); 
    // digitalWrite(9,HIGH);


    // init card bluetooth pins (one pin in auto one in out)
    pinMode(BT_RX_PIN, INPUT);
    pinMode(BT_TX_PIN, OUTPUT);

    // init leds pins in output mode
    pinMode(LED_MASTER_PIN, OUTPUT);
    pinMode(LED_SLAVE_PIN, OUTPUT);
    pinMode(LED_DETECT_PERSON_PIN, OUTPUT);

    // init touch sensor pins in input mode
    pinMode(TOUCH_SENSOR_PIN, INPUT);

    // init motor pins in output mode
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    pinMode(MOTOR_PIN1, OUTPUT);
    pinMode(MOTOR_PIN2, OUTPUT);

    // set hc05 rate
    hc05.begin(baudrate);

    // enable serial
    Serial.begin(9600);
    Serial.println("Command: ");
}


void loop() {
    // reduce CPU usage
    delay(100);

    // white serial input from computer
    while(Serial.available()) {
        String msg = Serial.readString();

        switch(msg) {
            // if the computer has detect someone, it send a message to the other card to become airlock's master
            case "detect\r\n":
                Serial.println("Potential-master: Detect someone");

                // if the card is in unknown state and that a reservation hasn't already been asked, we ask a reservation to the other card
                if(reserved_state == reserved_UNKNOWN && reservation_asked == false) {
                    setReservationAskState(true);
                    sendHC05(hc05, "reserve");
                }
                
                break;
            default:
                Serial.println("Type unknow");
                break;
        }
    }


    // read and process HC05 messages
    msg = readHC05(hc05);
    if(msg != "") {
        switch(msg) {
            case "reserve\r\n":
                Serial.println("Potential-slave: Reservation asked by the other door.");

                // check that airlock isn't reserved and that the actual card didn't asked it. If that the case, we answer to the other card that it can become the airlock's master (and this card become the slave)
                if(reserved == reserved_UNKNOWN && reservation_asked == false) {
                    setReservedState(reserved_SLAVE);
                    sendHC05(hc05, "reservation_ok");

                    Serial.println("Potential-slave: Airlock isn't reserved, the card become slave.");
                } else {  // otherwise we decline the reservation ask
                    sendHC05(hc05, "reservation_notok");
                
                    Serial.println("Potential-slave: Airlock is already reserved.");
                }
                
                break;
            case "reservation_ok\r\n":
                // if the reservation has been accepted, the card become master
                Serial.println("Master: Airlock reservation accepted. Opening door...");

                setReservedState(reserved_MASTER);
                setReservationAskState(false);

                // process master opening and closing his door
                processOpenAndCloseDoor();

                // ask to the other door to make the same process with the second door
                sendHC05(hc05, "opendoor");
                
                break;

            case "reservation_notok\r\n":
                // reservation has been refused, the card go back in unknown state
                Serial.println("Potential-master: Airlock reservation unaccepted.");

                setReservedState(reserved_UNKNOWN);
                setReservationAskState(false);
    
                break;

            case "opendoor\r\n":
                Serial.println("Slave: Opening door...");

                // process slave opening and closing his door
                processOpenAndCloseDoor();

                // tell to the master that the door process is finished
                sendHC05(hc05, "opendoor_done");

                break;

            case "opendoor_done\r\n":
                // when the slave door process is finished, we release the reservations state to make both cards back to unknown state
                Serial.println("Master: Slave finished his process. Releasing the airlock.");

                setReservedState(reserved_UNKNOWN);
                sendHC05(hc05, "unreserve");

                break;
            
            case "unreserve\r\n":
                // Master finished process and tell to the card that it releases the reservation
                Serial.println("Slave: Airlock unreserved.");

                setReservedState(reserved_UNKNOWN);

                break;

            default:
                Serial.println("Type unknow");
                break;
        }
    }
}



/** state changed (easier to manage leds) **/
void setReservationAskState(bool newValue) {
    reservation_asked = newValue;

    if(reservation_asked) {
        changeLEDState(LED_DETECT_PERSON_PIN, HIGH);
    } else {
        changeLEDState(LED_DETECT_PERSON_PIN, LOW);
    }
}

void setReservedState(int newValue) {
    reserved_state = newValue;

    switch(reserved_state) {
        case 0:
            changeLEDState(LED_MASTER_PIN, LOW);
            changeLEDState(LED_SLAVE_PIN, LOW);
            break;

        case 1:
            changeLEDState(LED_MASTER_PIN, HIGH);
            changeLEDState(LED_SLAVE_PIN, LOW);
            break;

        case 2:
            changeLEDState(LED_MASTER_PIN, LOW);
            changeLEDState(LED_SLAVE_PIN, HIGH);
            break;
    
        default:
            Serial.println("Reserve type unknow");
            break;
    }
}


/** Motor utils **/
void setMotorSpeed(int speed, bool reverse) { // speed between 0 and 255
    if(speed >= 0 && speed <= 255) {
        analogWrite(MOTOR_PWM_PIN, speed); // controle the new motor speed

        if(speed == 0) {
            // turn off the motor
            digitalWrite(MOTOR_PIN1, LOW);
            digitalWrite(MOTOR_PIN1, LOW);
        } else {
            if(!reverse) {
                // run motor clockwise
                digitalWrite(MOTOR_PIN1, HIGH);
                digitalWrite(MOTOR_PIN1, LOW);
            } else {
                // run motor anti-clockwise
                digitalWrite(MOTOR_PIN1, LOW);
                digitalWrite(MOTOR_PIN1, HIGH);
            }
        }
    }
}


/** Door utils **/
void processOpenAndCloseDoor() {
    openDoor();
    closeDoor();
}

void openDoor() {
    setMotorSpeed(255, false); // start the motor without reverse
    delay(10000); // open door while 10s (this tips allow us to use 2 touch sensors less, even if for a real usage, these sensor would be a better solution)
    setMotorSpeed(0, false); // stop the motor
}

void closeDoor() {
    // if door isn't close, we start to close it
    if(!isClose()) {
        setMotorSpeed(255, true); // start motor in reverse mode

        // while the closing touch sensor isn't activate, we continue to close the door
        while(!isClose()) {
            delay(100);
        }
    }

    // stop the motor
    setMotorSpeed(0, true);
}

bool isClose() {
    // read touch sensor
    return digitalRead(KEY) == HIGH;
}


/* LED utils **/
void changeLEDState(int led, int newValue) {
    // set the LED with the ledState of the variable
    digitalWrite(led, newValue);
}


/*** HC05 card utils ***/
void sendHC05(SoftwareSerial card, String msg) {
    // send a message to the HC05 card through a SoftwareSerial
    card.println(msg);
}

String readHC05(SoftwareSerial card) {
    // read HC05 input from a SoftwareSerial
    String msg = "";
    while (card.available()) {
        delay(10);
        if (card.available() > 0) {
            char c = card.read();
            msg += c;
        }
    }

    return msg;
}
