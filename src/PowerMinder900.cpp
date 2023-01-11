/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/jeff/powerwall900/PowerMinder900/src/PowerMinder900.ino"
void setmsgBuffer(uint8_t textSize);
void oledOn(void *args);
void motionDetected(void);
void fadeOn(unsigned int time,int increament, pin_t pinnum);
void fadeOff(unsigned int time,int decreament, pin_t pinnum);
void showBattery(const char *event ,int data);
void setup();
void loop();
#line 1 "/home/jeff/powerwall900/PowerMinder900/src/PowerMinder900.ino"
SYSTEM_THREAD(ENABLED);

#include "HttpClient.h"
#include "oled-wing-adafruit.h"
//THIS IS FOR RADIO ****************************************************************************
#include "RFM69-Particle.h"
#include "RFM69_ATC.h"
#include "RFM69registers.h"

int16_t NETWORKID = 100;  //the same on all nodes that talk to each other
int16_t NODEID = 2;       //make sure NODEID differs between nodes  
int16_t RECEIVER = 1;     //receiver's node ID

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY      RF69_915MHZ
#define ENCRYPTKEY     "" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW    true // set to 'true' if you are using an RFM69HCW module

//**************************
//Adjusted for Argon wirring  
#define RFM69_CS      D5
#define RFM69_IRQ     D6 
#define RFM69_IRQN    D6 //On Photon it is the same unlike Arduino
#define RFM69_RST     D4 

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);  //initialize radio 
/*
typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         temp;   //temperature maybe?
} Payload;

Payload theData;
*/
typedef struct {
  int         sysHour;       //hour of day
  int         gridStatus;    //grid 1 up 2 down
  int         batteryStatus; //battery 1 charging 2 discharging 3 idle 
  int         percentage;    //battery percentage
  int         highUsage;     //1 load in bounds 0 load out of bounds
} Payload;

Payload theData;


//END RADIO STUFF *******************************************************************************

#define PERCENT_STRING_SIZE 5
#define OLED_RESET D8
//OledWingAdafruit display;
OledWingAdafruit display;



unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;
char buffer[100];
char buffer2[100];
char buffer3[10];
char percentString[PERCENT_STRING_SIZE] = "00";
//buffers for OLED
char lineOne[20] = "WAITING ON";
char lineTwo[20] = "UPDATES...";
bool updating = FALSE; 


os_thread_t neot;
os_mutex_t mutex;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void setmsgBuffer(uint8_t textSize){
    display.setTextSize(textSize);
    display.setTextColor(WHITE);
    display.setCursor(0, 18);
    display.println(lineTwo);
    display.setCursor(0,0);
    display.println(lineOne);
    if (strcmp(lineOne, "WAITING ON")!=0){
        display.setCursor(90,0);
        if (theData.percentage == 100){
            display.setTextSize(1);
            //display.setCursor(80,18);
            snprintf(percentString, sizeof(percentString), "%s%s", percentString, "%");
            display.println(percentString);
        
        }
        else {
            snprintf(percentString, sizeof(percentString), "%s%s", percentString, "%");
            display.println(percentString);
        }
        
      
    }
    
}


//OLED THREAD
void oledOn(void *args){
    while(1){
        os_mutex_lock(mutex);
        updating = TRUE;
        //digitalWrite(D7,HIGH);
        setmsgBuffer(2);
        display.display();
        delay(30000);
        //digitalWrite(D7,LOW);
        display.clearDisplay();
        display.display();
        updating = FALSE;
    }
}
//MUTEX Controller from Interupt
void motionDetected(void){
    //ADD A CHECK FOR BOOLEON HERE TO SEE OF OLEDON IS RUNNING - ONLY UNLOCK IF NOT
    if (!updating){
        os_mutex_unlock(mutex);
    }
}


void fadeOn(unsigned int time,int increament, pin_t pinnum){
    for (byte value = 0 ; value < 255; value+=increament){
        analogWrite(pinnum, value);
        delay(time/(255/5));
    }
}
void fadeOff(unsigned int time,int decreament, pin_t pinnum){
    for (byte value = 255; value >0; value-=decreament){
        analogWrite(pinnum, value);
        delay(time/(255/5));
    }
}

//void showBattery(const char *event ,const char *data) {
void showBattery(const char *event ,int data) {
    if (data == 1){
        //GREEN LED
        //turn off red- can't be green and red!
        analogWrite(A2,LOW);
        fadeOn(2000,3,A3);
        fadeOff(2000,3,A3);
        analogWrite(A3,LOW);
        strcpy(lineOne,  "BATTERY");
        strcpy(lineTwo, "CHARGING");
      
    }
    else if (data == 2){
        //RED LED
        fadeOn(2000,3,A2);
        fadeOff(2000,3,A2);
        analogWrite(A2,LOW);
        strcpy(lineOne, "BATTERY");
        strcpy(lineTwo, "DISCHARGE");
    }
    else{
        //BLUE LED
         //turn off red- can't be blue and red!
        analogWrite(A2,LOW);
        fadeOn(2000,3,A1);
        fadeOff(2000,3,A1);
        analogWrite(A1,LOW);
        strcpy(lineOne, "BATTERY");
        strcpy(lineTwo, "IDLE");
    }
}

void setup() {
    //SETUP FOR RADIO
    //******************************************************************************************
    Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
    //Particle.publish("RFM69 TX Startup setup","Completed",360,PRIVATE);
    //Particle.publish("WiFi signal",String(WiFi.RSSI()),360,PRIVATE);
    Serial.println("RFM69 Based Transmitter");
  
    // Hard Reset the RFM module - Optional
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);
  
    // Initialize radio
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
        if (IS_RFM69HCW) {
            radio.setHighPower();    // Only for RFM69HCW & HW!
        }

  delay(10000); //give time for user to connect serial monitor if necessary
  if (!radio.initialize(FREQUENCY,NODEID,NETWORKID))
  {
     Serial.println("init failed");
  }
  else
  {
    Serial.println("init OK");
  }
  
    // To improve distance set a lower bit rate. Most libraries use 55.55 kbps as default
    // See https://lowpowerlab.com/forum/moteino/rfm69hw-bit-rate-settings/msg1979/#msg1979
    // Here we will set it to 9.6 kbps instead 
    radio.writeReg(0x03,0x0D); //set bit rate to 9k6
    radio.writeReg(0x04,0x05);
  
    radio.setPowerLevel(15); // power output ranges from 0 (5dBm) to 31 (20dBm)
                          // Note at 20dBm the radio sources up to 130 mA! 
                         // Selecting a power level between 10 and 15 will use ~30-44 mA which is generally more compatible with Photon power sources
                        // As reference, power level of 10 transmits successfully at least 300 feet with 0% packet loss right through a home, sufficient for most use
    
    //radio.encrypt(ENCRYPTKEY);
    radio.encrypt(0);
  
    Serial.print("\nListening at ");
    Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(" MHz");
    //*****************************************************************************************
    //END SETUP FOR RADIO

    display.setup(); 
    display.clearDisplay();
    display.display();

    //BLUE LED
     pinMode(A1, OUTPUT);
    //RED LED
    pinMode(A2, OUTPUT);
    //GREEN LED
    pinMode(A3, OUTPUT);

    //RUN THROUGH LED COLORS
//Fire Up the Green LED
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("GREEN-3");
    display.display();
    analogWrite(A3,250);
    delay(2000);
    analogWrite(A3,LOW);
    display.clearDisplay();
    display.display();
//Fire up the Blue LED
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("BLUE-2");
    display.display();
    analogWrite(A1,250);
    delay(2000);
    analogWrite(A1,LOW);
    display.clearDisplay();
    display.display();
//Fire up the Red LED
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("RED-1");
    display.display();
    analogWrite(A2,250);
    delay(2000);
    analogWrite(A2,LOW);
    display.clearDisplay();
    display.display();

    //Thread create
    os_thread_create(&neot,NULL,OS_THREAD_PRIORITY_DEFAULT,oledOn,NULL,2048);
    os_mutex_create(&mutex);
    
    //ONBOARD LED 
    //pinMode(D7,OUTPUT);
    //INPUT FROM MOTION SENSOR
    pinMode(D3, INPUT);
    attachInterrupt(D3, motionDetected, RISING);
 


    //NEW TEST GET ALL
    request.hostname = "bay4pi.mars.local";
    request.port = 80;
    request.path = "/get_all";

}

void loop() {
      if (nextTime > millis()) {
        return;
    }
    
    //NEW TEST PRINT RETURN FROM WEB CALL
    http.get(request,response,headers);
    response.body.toCharArray(buffer,sizeof(buffer));
    Serial.println("\nResponse from Server: ");
    Serial.print(buffer);

    //Crappy code follows
    char* token = strtok(buffer, ",");
    //type int
    theData.sysHour = atoi(token);
    token = strtok(NULL,",");
    theData.gridStatus = atoi(token);
    token = strtok(NULL,","); 
    theData.batteryStatus = atoi(token);
    token = strtok(NULL, ","); 
    strcpy(percentString,token);
    theData.percentage = atoi(token);
    //itoa(theData.percentage, percentString, 10);
    token = strtok(NULL,",");
    theData.highUsage = atoi(token);
    
    
    //THIS IS WHERE LED LOGIC GETS CONTROLLED
    //Not really doing much here but calling the showBattery function which takes the buffer and does what it needs
    if (theData.gridStatus == 1){
        showBattery("fireCannon", theData.batteryStatus);
    }
    else if (theData.gridStatus == 0 && theData.percentage != 0){
        //RED LED PANIC ON THE STREETS OF LONDON
        analogWrite(A2, HIGH);
        strcpy(lineOne, "GRID");
        strcpy(lineTwo, "DOWN");
    }
    else{
       //Battery in Transition do something here
       analogWrite(A2, LOW);
    }
     
    // THIS IS THE RADIO TRANSMIT SECION *****************************************************
     
     
    Serial.print("\n."); //THis gives us a neat visual indication of time between messages received
    //Serial.print("test");

    char radiopacket[20] = "Hello M0 #";
    itoa(packetnum++, radiopacket+13, 10);
    Serial.print(packetnum);
    Serial.print("Sending "); Serial.println(radiopacket);
  
    //send the message
    //radio.send(RECEIVER, radiopacket, strlen(radiopacket));//target node Id, message as string or byte array, message length
    //Serial.println("OK, sent without retrying"); //Why send without retrying? If the RX is down for some reason, the TX will remain in a state of high power
    //which can drain your battery or overwhelm your power source. 

    // Uncomment below and comment out above if you want to send with retry (which will look for an ACK message or resends)
    /*if (radio.sendWithRetry(RECEIVER, radiopacket, strlen(radiopacket))) { //target node Id, message as string or byte array, message length
    Serial.println("OK, sent with retry");
    }
    */
    if (radio.sendWithRetry(RECEIVER,(const void*)(&theData), sizeof(theData))) { //target node Id, message as string or byte array, message length
    Serial.println("OK, sent with retry");
    }    
    else
    {
      Serial.println("Failed to send");  
    }
    //send message to Particle console if not using serial
    //String TXMessage = "[NODE: " + String(NODEID) + "]  " + String(radiopacket) + " ";
    //Particle.publish("Message transmitted",TXMessage,360,PRIVATE);    
    //Serial.println("Message transmitted " + TXMessage);    
    radio.receiveDone(); //put radio in RX mode
    
    //END RADIO TRANSMIT SECTION **************************************************************


    //nextTime = millis() + 2000;
    nextTime = millis() + 10000;
    
}



