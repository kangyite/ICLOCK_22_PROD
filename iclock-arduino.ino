//IOTClock_01 = Principal's Office (Admin)
//IOTClock_02 = Vice Principal's Office (Admin)
//IOTClock_03 = CEC Room (Others)
//IOTClock_04 = Yite's Class (S1)
//IOTClock_05 = Tzi Hwee's Class (S2)

//------------------------------CHANGE BEFORE EVERY COMMIT--------------------
#define FIRMWARE_VER "IoT_Clock_22_Prod" //Firmware Name                      |
//----------------------------------------------------------------------------

char clockID [3]; 
//---------------------WS2812 FAST LED LIBRARY-------------------
#include <FastLED.h> 
#define NUM_LEDS 16
#define LED_DATA_PIN 17
#define STRIP_TYPE WS2812B
#define COLOR_ORDER GRB
uint8_t LEDBrightness = 50;
bool rainbowDir = 1;
uint8_t hueRed = 0;
uint8_t hueOrange = 32;
uint8_t hueYellow = 64;
uint8_t hueGreen = 96;
uint8_t hueAqua = 128;
uint8_t hueBlue = 160;
uint8_t huePurple = 192;
uint8_t huePink = 224;
uint8_t gHue = hueGreen; // rotating "base color" used by many of the patterns
CRGB leds[NUM_LEDS];

//---------------------PAROLA DISPLAY----------------------------
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8

#define CLK_PIN       SCK  // D18
#define DATA_PIN      MOSI // D23, DIN on LED 
#define CS_PIN        SS   // D5 

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global message buffers shared by Serial and Scrolling functions
uint8_t scrollSpeed = 20;    //***** SCROLLING SPEED,  BIGGER =  FASTER
textEffect_t scrollEffect = PA_SCROLL_RIGHT; //***** SCROLLING DIRECTION = RIGHT SHOULD BE NORMAL
textPosition_t scrollAlign = PA_LEFT; 
uint16_t scrollPause = 0; // in milliseconds
//uint16_t scrollPause = 1000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  100
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Hello! This is a test message :( Please show" };
//char curMessages[BUF_SIZE] = ""
bool newMessageAvailable = true;
uint8_t currentMessageLoopCount = 0;
uint8_t messageLoopCount = 3; //*****How many times you want each message to be shown

//Effects that can be used
textEffect_t scrollEffectList[] = {
  PA_NO_EFFECT, PA_PRINT, PA_SCROLL_UP, PA_SCROLL_DOWN,
  PA_SCROLL_LEFT, PA_SCROLL_RIGHT, PA_MESH, PA_SCROLL_DOWN_RIGHT
};

int brightnessValue = 0;


//----------------------Wifi and OTA --------------------------
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

char wifiInfo [][2][65] = {
  {"Enitang", "eni03579"},
  {"Papaya", "steam123"},
  {"CEC", "CEC_2018"},
  {"CLPHS_Admin", "smclp8283655"},
  {"OFF-ADMIN_WIFI", "clp8283655"},
  {"IOT1", "12345hello"}
};
char wifiBuffer [65];
int wifiRSSI;
char SSIDBUF [20];
int leaveLoop = 0;

//-------------------NTP TIME----------------------
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800;
const int   daylightOffset_sec = 0;
char timeBuffer [15];
char prevTimeBuffer [15];
char dateBuffer [15];
char timeDispBuffer [15];
time_t unixTime;

//-------------------DHT SENSOR--------------------
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 19     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);
float TEMP = 0.0;
float HUM = 0.0;
char THBUFFER[13];

//-------------------BRIGHTNESS SENSOR--------------
#include <Smoothed.h>
Smoothed <float> TEMT6000;
#define SENSOR_PIN 33
float LUX;
int readCount = 0;

//--------------------Firebase and WIFI ----------------------
#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST "https://clphs-iot.firebaseio.com/"
#define FIREBASE_AUTH "PHquDi2a7VmSZl3sYUM2G2h1iK7QnyF231RESOcx"
FirebaseData firebaseData;
FirebaseData firebaseData2;
bool sendHistoryFlag = 0;

//----------------- FIREBASE ADDRESSES ------------------------
char clocks [] = "/clocks/IOTClock_";
char global [] = "/global";
String path = "/global/message";

char AddressGlobalPeriod[] = "/global/periodInfo";

//Placeholder address names for this particular clock-------------
char AddressTEMP [40];
char AddressHISTORY [75];


//Epoch time estimate 1,598,884,458
//Unsigned long max   4,294,967,295

//Storage for firebase get variables
char AUDIENCE[50];
char MESSAGE [100];
char MESSAGEID [25]; //"zMHYM6RaLTUNkpk0tYXA"
unsigned long MESSAGETIME; //"1598878200"
unsigned long  WEATHEREPOCHTIME; //"1598878200"

char clockName [30]; //For OTA Update

bool SENSOREXIST = 1;
char currentMessageID [25] = "";

char category [15]; 
char periodInfo[25][5];

//-------------------DF Player Mini-----------------
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>
SoftwareSerial mySerial(26, 25); //On DF Player, 3=TX, 13=RX
DFPlayerMini_Fast myMP3;
uint8_t MP3BusyPIN = 22;
uint8_t volume = 30; //*****SPEAKER VOLUME, Maximum is 30
/*
  0 Chord
  1 Circles
  2 Soft Full
  3 Upbeat 1
  4 Upbeat 2
  5 Upbeat 2
  6 Chime
*/
//-------------------ArduinoJson--------------------
#include <ArduinoJson.h>

//-------------------SPIFFS------------------------
#include "SPIFFS.h"

//-------------------Update-------------------------
#include <Update.h>
#include <HTTPClient.h>

//-------------------TIMER--------------------------

#define RUN_LED_pin   2
enum {
  TIM_0,
  TIM_1,
  TIM_2,
  TIM_3,
  TIM_3A,
  TIM_4,
  TIM_W
};

#define TIMOUT  0
#define TIMSET  1

#define TIME_100_msec  100
#define TIME_500_msec  500
#define TIME_1p0_sec   1000

unsigned long time0_data, time1_data, time2_data, time3_data, time3A_data, time4_data, timeW_data;
unsigned long time0_set, time1_set, time2_set, time3_set, time3A_set, time4_set, timeW_set;
byte tim0, tim1, tim2, tim3, tim3A, tim4, timW;
int seq0_num, seq1_num, seq2_num, seq3_num, seq4_num, seqW_num;

#define UNINITIATED  0
#define WAITINGFORWIFI  1
#define WIFIREADY  2
#define INITIATED  3
uint8_t initiated = UNINITIATED;

//===================FUNCTION DEFINITIONS=================================
//
//char secondChange(void);
//void setBrightness(int);
//void readSensors(void);
//void initiate(void);
//void getFirebaseWeather(void);
//void sendToFirebase(void);
//void checkMessage(void);
//void loopMessage(uint8_t);

//===================================================================

void printResult(FirebaseData &data);
void printResult(StreamData &data);

void setup() {
  tim0 = tim1 = tim2 = tim3 = tim3A = tim4 = timW = 0;
  seq0_num = seq1_num = seq2_num = seq3_num = seq4_num = seqW_num = 0;
  Serial.begin(115200);
  mySerial.begin(9600);
  myMP3.begin(mySerial);
  pinMode(MP3BusyPIN, INPUT);
  Serial.println("DF Player Setting Volume to Max");
  myMP3.volume(30);
  delay(20);


  pinMode(RUN_LED_pin, OUTPUT);

  P.begin();
  P.setZoneEffect(0, !P.getZoneEffect(0, PA_FLIP_LR), PA_FLIP_LR);
  P.setZoneEffect(0, !P.getZoneEffect(0, PA_FLIP_UD), PA_FLIP_UD);
  P.displayReset();
  P.setIntensity(brightnessValue);  /// Brightness setting

  FastLED.addLeds<STRIP_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(LEDBrightness); //Can be put inside void Loop
}
//================================Setup Ends======================================

void loop() {
  if (initiated != INITIATED) {
    initiate();
  }
//  Serial.println(digitalRead(MP3BusyPIN)); // can use
  sequence0();  //
  //Serial.println(seq1_num);
  sequence1();  //
  sequence2();  //
  sequence3();  //
  sequence4();
  sequenceW();
}

void sequence0(void) {
  switch (seq0_num)
  {
    case 0:
      if (timer_milli_sec(TIM_0) == TIMOUT)
      {
        set_timer_milli_sec(TIM_0, TIME_500_msec);
        digitalWrite(RUN_LED_pin, HIGH);
        seq0_num = 10;
      }
      break;
    case 10:
      if (timer_milli_sec(TIM_0) == TIMOUT)
      {
        set_timer_milli_sec(TIM_0, TIME_500_msec);
        ArduinoOTA.handle();
        digitalWrite(RUN_LED_pin, LOW);
        seq0_num = 0;
      }
      break;
    case 20:
      break;
    default:
      break;
  }
}

void sequence1(void) { //MAX7219 (Hence Clock time display, Firebase send/get Sequences

  switch (seq1_num) {
    case 0:
      break;

    case 10:
      if (timer_milli_sec(TIM_1) == TIMOUT) {
        set_timer_milli_sec(TIM_1, 50);
        //Rest and get time
        if (secondChange() == 1) {
          seq1_num = 20;
        }
      }
      break;

    case 20: //Root node, check if it's the start of a minute
      if (timer_milli_sec(TIM_1) == TIMOUT) {
        int secondTens = timeBuffer[6] - '0';
        int secondOnes = timeBuffer[7] - '0';
        int seconds = secondTens * 10 + secondOnes;
        //Serial.println(seconds);

        if (seconds == 0) {
          seq1_num = 40;
        }
        else {
          Serial.println(timeDispBuffer);
          //P.print(timeDispBuffer);
          P.displayText(timeDispBuffer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
          if (P.displayAnimate()) P.displayReset();
          seq1_num = 30;
        }
      }
      break;

    case 30:
      if (SENSOREXIST == 1) {
        readSensors();
        setBrightness(LUX);
      }
      seq1_num = 10;
      break;

    case 40: //Every Minute, at 0th Second, do a bell chime if its new period, else display date and use that time to checkout wifi status
      set_timer_milli_sec(TIM_1, 4900);
      P.displayText(dateBuffer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      if (P.displayAnimate()) P.displayReset();
      
//      //Check if its a period
//      //strftime(timeBuffer, 15, "%I:%M:%S %p", &timeinfo);
//      //0 1 : 3 4 : 6 7 PM
        for(int i = 0; i < 25; i++){
          if(timeBuffer[0] == periodInfo[i][0]){ //1st number of hour
            if(timeBuffer[1] == periodInfo[i][1]){ //2nd number of hour
              if(timeBuffer[3] == periodInfo[i][2]){ //1st number of minute
                if(timeBuffer[4] == periodInfo[i][3]){ //2nd number of minute
                  myMP3.play(2);
                  Serial.print("Bell Time now: "); Serial.println(periodInfo[i]);
                  P.displayText("NEXT PERIOD", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
                  if (P.displayAnimate()) P.displayReset();
                }
              }
            }
          }          
        }

//      //Check if its the start of an hour or half hour
//      //strftime(timeBuffer, 15, "%I:%M:%S %p", &timeinfo);
//      //0 1 : 3 4 : 6 7
          if(timeBuffer[3] == '0' || timeBuffer[3] == '2' || timeBuffer[3] == '4'){ //1st number of minute
            if(timeBuffer[4] == '0'){ //2nd number of minute
//              myMP3.play(2);
              Serial.print("Start of an hour now now: "); 
              sendHistoryFlag = 1;              
          }
        }

      if (WiFi.status() != WL_CONNECTED) { //Stuff that if no Internet
        //        Serial.println("UNSYNCED");
        //        P.displayText("UNSYNCED", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
        //        if (P.displayAnimate()) P.displayReset();

        Serial.println("No known wifi networks around, start scanning");
        int n = WiFi.scanNetworks();
        Serial.println("scan done");
        if (n == 0) {
          Serial.println("no networks found");
        }
        else {
          Serial.print(n);
          Serial.println(" networks found");
          for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
          }
        }
        Serial.println("");
      }

      else { //Stuff that if there is Internet
        //        Serial.println("SYNC");
        //        P.displayText("SYNC", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
        //        if (P.displayAnimate()) P.displayReset();

        wifiRSSI = WiFi.RSSI();
        Serial.print("Currently connected to: ");
        Serial.print(WiFi.SSID());
        Serial.print(" | Wifi Strength: "); 
        Serial.println(wifiRSSI);

        while (secondChange() == 1) {
          configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
          Serial.println("Chillin waiting for edge of second");
          delay(1);
          int NTPRetryCount = 0;
          NTPRetryCount++;
          if (NTPRetryCount == 10000) { //In case it never changes :O
            NTPRetryCount = 0;
            break;
          }
        }
      }
      seq1_num = 50;
      break;

    case 50: //Display Temperature and Humidity
      if (timer_milli_sec(TIM_1) == TIMOUT) {
        set_timer_milli_sec(TIM_1, 4000);
        Serial.print("Temperature: "); Serial.print(TEMP); Serial.print("°C ");
        Serial.print("Humidity: "); Serial.print(HUM); Serial.println("%");
        Serial.print("LUX: "); Serial.print(LUX); Serial.println("");
        static char TEMPSTR[5];
        dtostrf(TEMP, 5, 1, TEMPSTR);
        static char HUMSTR[4];
        dtostrf(HUM, 3, 1, HUMSTR);
        THBUFFER[0]  = ' ';
        THBUFFER[1]  = TEMPSTR[1];
        THBUFFER[2]  = TEMPSTR[2];
        THBUFFER[3]  = TEMPSTR[3];
        THBUFFER[4]  = TEMPSTR[4];
        THBUFFER[5]  = 'C';
        THBUFFER[6]  = ' ';
        THBUFFER[7]  = ' ';
        THBUFFER[8]  = HUMSTR[0];
        THBUFFER[9]  = HUMSTR[1];
        THBUFFER[10]  = '%';
        THBUFFER[11] = ' ';
        //        THBUFFER[12] = '%';
        //        for(int i = 0; i < 12; i++){
        //          Serial.println(THBUFFER[i]);
        //        }
        P.displayText(THBUFFER, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
        if (P.displayAnimate()) P.displayReset();
        //        P.print(THBUFFER);

        set_timer_milli_sec(TIM_1, 7000);
        if(SENSOREXIST)sendToFirebase();
        checkForUpdates();
        seq1_num = 60;
      }
      break;

    case 60:        
//      Serial.println("Checking Message");
//      if(checkMessage() == 1){
//        Serial.println("Resetting display and starting to display the message");
//        P.displayReset(); //Readys the display
//        seq1_num = 70;
//      }
//      else{
        seq1_num = 80;      
//      }
      break;

    case 70:
      if (timer_milli_sec(TIM_1) == TIMOUT) {
        set_timer_milli_sec(TIM_1, 100);
        loopMessage();
        seq1_num = 80;
      }
      break;

    case 80:
      if (timer_milli_sec(TIM_1) == TIMOUT) {
        Serial.println("message display ended");
        seq1_num = 10;
      }
      break;
      
    default:
      break;
  }
}

void sequence2(void) { //RGB Sequence
  switch (seq2_num)
  {
    case 0:
      if (timer_milli_sec(TIM_2) == TIMOUT) {
        loopRedTheme();
        set_timer_milli_sec(TIM_2, 200);
      }
      break;
    case 10:
      if (timer_milli_sec(TIM_2) == TIMOUT) {
        if (WiFi.status() != WL_CONNECTED) { //not connected
          loopRedTheme();
        }
        else {//connected
          loopGreenTheme();
        }
        set_timer_milli_sec(TIM_2, 200);
      }
      break;
    case 20:
      break;
    case 30:
      break;
    default:
      break;
  }
}

void sequence3(void) { //EMPTY
  switch (seq3_num) {
    case 0:
      break;
    case 10:
      break;
    case 20:
      break;
    default:
      break;
  }
}

void sequence4(void) { //EMPTY
  switch (seq4_num)
  {
    case 0:
      break;
    case 10:
      break;
    case 20:
      break;
    default:
      break;
  }
}

uint8_t networksFound = 0;
uint8_t networkIndex = 0;
uint8_t KnownNI[10]; //Index of Available Networks in the current scan list, will try first 10 strongest wifis of knwon credentials
uint8_t KnownNII = 0; //Index in the list of AvailNI
uint8_t maxKnownNII = 0; //Index in the list of AvailNI
uint8_t wifiIndex = 0;
uint8_t retryCount = 0;

void sequenceW(void) {
  switch (seqW_num)
  {
    case 0:
      break;
    case 10:
      if (timer_milli_sec(TIM_W) == TIMOUT) {
        set_timer_milli_sec(TIM_W, 2000);
        if (WiFi.status() == WL_CONNECTED) { //Check Wifi status, if already connected just report
          //Serial.print("Connected to "); Serial.println(WiFi.SSID());
          //initiated = WIFIREADY;
        }
        else { //If not connected, start scan and attempt to connect
          Serial.println("Not connected, scanning for nearby networks");
          networksFound = WiFi.scanNetworks(); // WiFi.scanNetworks will return the number of networks found
          if (networksFound == 0) {
            Serial.println("No networks found");
          }
          else {
            Serial.print(networksFound); Serial.println(" Networks Found: ");
            networkIndex = 0; //i
            KnownNII = 0;
            clearKnownNI();
            seqW_num = 20;
          }
        }
      }
      break;

    case 20:
      if (networkIndex < networksFound) { //Look at current network, print out data
        Serial.print(networkIndex + 1); Serial.print(": "); //1:
        WiFi.SSID(networkIndex).toCharArray(SSIDBUF, 50); //saengch
        Serial.print(SSIDBUF); Serial.print(" ("); Serial.print(WiFi.RSSI(networkIndex)); Serial.print(")"); //(-44)
        Serial.print((WiFi.encryptionType(networkIndex) == WIFI_AUTH_OPEN) ? " " : "*"); //*

        for (int j = 0; j < (sizeof(wifiInfo) / sizeof(wifiInfo[0])); j++) { //Reference against known networks, print "Known" if thats true
          if (strcmp(SSIDBUF, wifiInfo[j][0]) == 0) { //If exists in our Known list of networks
            Serial.print(" -Known Network");
            KnownNI[KnownNII] = j; //Store for later attempts to connect
            KnownNII++;
          }
        }
        Serial.println("");
        networkIndex++;
      }
      else {
        Serial.println("Scan complete");
        //
        //        Serial.print("Their IDs on our wifi list");
        //        for(int i = 0; i < KnownNII; i++){
        //          Serial.print(KnownNI[i]); Serial.print(" ");
        //        }
        //        Serial.println("");
        seqW_num = 30;
        maxKnownNII = KnownNII;
        KnownNII = 0;
      }
      break;

    case 30: //Nexus for attempting wifi networks, once for every network
      if (KnownNII < maxKnownNII) {
        Serial.print("Connecting to "); Serial.print(wifiInfo[KnownNI[KnownNII]][0]);
        WiFi.begin(wifiInfo[KnownNI[KnownNII]][0], wifiInfo[KnownNI[KnownNII]][1]);
        KnownNII++;
        retryCount = 0;
        seqW_num = 40;
      }
      else {
        Serial.println("All known WIFIs refused us");
        //Parola display No Known Wifis Available
        seqW_num = 10;
      }
      break;

    case 40: //10 attempts for each network
      if (timer_milli_sec(TIM_W) == TIMOUT) {// Every second,
        set_timer_milli_sec(TIM_W, 1000);
        if (WiFi.status() != WL_CONNECTED) {
          if (retryCount < 10) { //Haven't retried enough yet
            Serial.print(".");
            retryCount++;
          }
          else {//Retried enough, moving on
            Serial.println("No response for 10 secs, moving on");
            retryCount = 0;
            seqW_num = 30;
          }
        }
        if (WiFi.status() == WL_CONNECTED) { //Exit loop upon connection if that was our goal in the first place
          //Serial.println("Connected!");
          //initiated = WIFIREADY;
          myMP3.play(4);
          retryCount = 0;
          seqW_num = 10;
        }
      }
      break;
    default:
      break;
  }
}

void clearKnownNI(void) {
  for (int i = 0; i < 10; i++) {
    KnownNI[i] = 0;
  }
}

char secondChange() { //Updates Buffer on the first instance where the second has changed.
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { // Error, cannot obtain time
    Serial.println("Failed to obtain time, wifi status is: ");
    return 2;
  }

  //  Serial.println(&timeinfo, "%A, %B %d %Y %I:%M:%S %p"); //Print time
  strftime(dateBuffer, 15, "%d-%m-%Y", &timeinfo);
  strftime(timeBuffer, 15, "%I:%M:%S %p", &timeinfo);
  if (timeBuffer[7] != prevTimeBuffer[7]) { //Edge of the second
    
    unixTime = mktime(&timeinfo); // get unix time for maths

    prevTimeBuffer[7] = timeBuffer[7]; //record current read for comparison in next loop

    //This part is to change 04:30:15 PM -> 4:30:15 PM
    if (timeBuffer[0] == '0') { //if first digit is 0 aka hour is single digit
      for (int i = 0; i < 14; i++) { //Copy but shift entire array forwards by 1
        timeDispBuffer[i] = timeBuffer[i + 1];
      }
    }
    else {
      for (int i = 0; i < 14; i++) { //Else just copy
        timeDispBuffer[i] = timeBuffer[i];
      }
    }
    return 1;
  }
  else { //Not edge of the second
    return 0;
  }
}

void updateFirmware(String download_url) {
  // Start pulling down the firmware binary.
   P.displayText("OFA UPDATE", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  if (P.displayAnimate()) P.displayReset();
  HTTPClient http;
  http.begin(download_url);
  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("HTTP failed, error: %s\n", 
       http.errorToString(httpCode).c_str());
    return;
  }
  // Check that we have enough space for the new binary.
  int contentLen = http.getSize();
  Serial.printf("Content-Length: %d\n", contentLen);
  bool canBegin = Update.begin(contentLen);
  if (!canBegin) {
    Serial.println("Not enough space to begin OTA");
    return;
  }
  // Write the HTTP stream to the Update library.
  WiFiClient* client = http.getStreamPtr();
  size_t written = Update.writeStream(*client);
  Serial.printf("OTA: %d/%d bytes written.\n", written, contentLen);
  if (written != contentLen) {
    Serial.println("Wrote partial binary. Giving up.");
    return;
  }
  if (!Update.end()) {
    Serial.println("Error from Update.end(): " + 
      String(Update.getError()));
    return;
  }
  if (Update.isFinished()) {
    Serial.println("Update successfully completed. Rebooting."); 
    // This line is specific to the ESP32 platform:
    ESP.restart();
  } else {
    Serial.println("Error from Update.isFinished(): " + 
      String(Update.getError()));
    return;
  }
}


//-----------------GET SENSOR DATA AND UPLOAD------------------
void readSensors() {
  sensors_event_t event;
  // Get Temperature
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    //Serial.println(F("Error reading temperature!"));
  }
  else {
    //Print Temperature
    TEMP = event.temperature;
    //Serial.print(F("Temperature: ")); Serial.print(TEMP); Serial.println(F("°C"));
  }

  // Get Humidity
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    //Serial.println(F("Error reading humidity!"));
  }
  else {
    //Print Humidity
    HUM = event.relative_humidity;
    //Serial.print(F("Humidity: ")); Serial.print(HUM); Serial.println(F("%"));
  }

  // Get Brightness
  TEMT6000.add(analogRead(SENSOR_PIN)); // Read a sensor value from analogue pin 0

  LUX = TEMT6000.get() * 0.9765625 * 4;
//  LUX = 0;
  Serial.print("Averaged: "); Serial.print(LUX); // Output the smoothed sensor value to the serial
}

void sendToFirebase(void) { //---------------------Send data to firebase-----------------------
  int ITEMP = TEMP;
  int IHUM = HUM;
  int ILUX = LUX;
  unsigned long startTime = millis();
  //For some unknown reason, you need to re-generate all these unique Firebase Addresses from time to time otherwise they get messed up
  //I think its an issue with C pointers and arrays. there is a overflowing 'om' that gets picked up 
  Serial.println("Re-creating addresses: ");
  sprintf(AddressTEMP, "%s%s", clocks, clockID);
  Serial.println(AddressTEMP);
  sprintf(AddressHISTORY, "%s%s", "/history/IOTClock_", clockID);
  Serial.println(AddressHISTORY);
  //Creating Json Object & Storing update data
  StaticJsonDocument<256> clockData;
  clockData["TEMP"] = ITEMP;
  clockData["HUM"] = IHUM;
  clockData["LUX"] = ILUX;
  clockData["SSID"]= WiFi.SSID();
  clockData["RSSI"] = wifiRSSI;
  clockData["LR"] = unixTime;
  clockData["FIRMWARE"] = FIRMWARE_VER;
  clockData["online"] = true;
  //ArduinoJSON to FirebaseJson
  String clockDataOutput = "";
  serializeJson(clockData,clockDataOutput);
  FirebaseJson updateData;
  updateData.setJsonData(clockDataOutput);

  //Updating Clock Node
  if (!Firebase.updateNode(firebaseData,AddressTEMP,updateData)){
    Serial.print("Error in Updating Data, "); Serial.println(firebaseData.errorReason());
  }
  
  //Updating History
  if(sendHistoryFlag == 1){   
    FirebaseJson json1;  
    FirebaseData firebaseData1;
    
    json1.set("TEMP", ITEMP);
    json1.set("HUM", IHUM);
    json1.set("LUX", ILUX);
    json1.set("RSSI", wifiRSSI);
    if (Firebase.pushJSON(firebaseData1, AddressHISTORY, json1)) {  //Push(append) to temperature history
      Serial.print("Sending JSON to firebase at hash: ");Serial.println(firebaseData1.dataPath() + "/" + firebaseData1.pushName());
      sendHistoryFlag = 0;
    } else {
      Serial.println(firebaseData1.errorReason());
    }
  }

  Serial.print("Firebase Set Data Complete, time taken: "); Serial.println(millis() - startTime);
}

void checkForUpdates(void){
  unsigned long startTime = millis();
  char AddressLTFIRMWARE[40];
  sprintf(AddressLTFIRMWARE, "%s%s", AddressTEMP, "/LTFIRMWARE");
  Serial.println(AddressLTFIRMWARE);
  Serial.println("[FIREBASE] Checking For Updates");
  if(!Firebase.get(firebaseData,AddressLTFIRMWARE)){ //Geting config at /clocks/{clock_name}/
    Serial.println("[IMPORTANT] Check for Update Failed");
    Serial.println(firebaseData.errorReason());
  } else {
    String LTFIRMWARE = firebaseData.stringData(); //FirebaseJson to ArduinoJson Conversion
    Serial.print("Latest Version: ");Serial.println(LTFIRMWARE);
    if(LTFIRMWARE == FIRMWARE_VER){
      Serial.println("[UPDATER] Firmware is Up To Date. No Update Needed");
    } else {
      Serial.println("[UPDATER] Firmware is Out Of Date. Initiating Update");
      String AddressFIRMWAREPATH = "/firmware/";
      AddressFIRMWAREPATH.concat(LTFIRMWARE);
      if(!Firebase.get(firebaseData,AddressFIRMWAREPATH)){
        Serial.println("[IMPORTANT] Get Firmware Details Failed");
        Serial.println(firebaseData.errorReason());
      } else {
        FirebaseJson *json = firebaseData.jsonObjectPtr(); //FirebaseJson to ArduinoJson Conversion
        String jsonStr;
        json->toString(jsonStr, true);
        StaticJsonDocument<256> firmware;
        deserializeJson(firmware,jsonStr);
        if(!firmware["url"].isNull()){
          Serial.print("Firmware URL: ");Serial.println(firmware["url"].as<String>());
          updateFirmware(firmware["url"].as<String>());
        } else {
          Serial.print("Lol. Got LTFIRMWARE no URL, how to download? Skipping");
        }
      }
    }
  }
  Serial.print("Checking For Updates Complete, time taken: "); Serial.println(millis() - startTime);
}

void loopMessage(void) {      
//  P.displayReset(); //Readys the display 
  myMP3.play(1);
  P.print("");
  currentMessageLoopCount = 0;
  Serial.print("currentMessageLoopCount upon initiation: ");  Serial.println(currentMessageLoopCount);
  while(currentMessageLoopCount <= messageLoopCount) {
    if (P.displayAnimate()) { // = 1 when animation has completed
      P.displayReset(); //Readys the display
      P.displayText(newMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
      Serial.print("currentMessageLoopCount in the while loop: ");  Serial.println(currentMessageLoopCount);
      currentMessageLoopCount++;
    }
  }
  Serial.print("Message has displayed "); Serial.print(currentMessageLoopCount); Serial.println(" times");
  currentMessageLoopCount = 0;
  newMessageAvailable = false;
}


void setBrightness(int lux) {
  if (lux >= 0 && lux < 50)     brightnessValue = 0;
  if (lux >= 50 && lux < 100)   brightnessValue = 1;
  if (lux >= 100 && lux < 150)   brightnessValue = 2;
  if (lux >= 150 && lux < 200)   brightnessValue = 3;
  if (lux >= 200 && lux < 400)   brightnessValue = 4;
  if (lux >= 400 && lux < 600)   brightnessValue = 5;
  if (lux >= 600 && lux < 800)  brightnessValue = 6;
  if (lux >= 800 && lux < 1000) brightnessValue = 7;
  if (lux >= 1000 && lux < 1400) brightnessValue = 8;
  if (lux >= 1400)               brightnessValue = 9;
  P.setIntensity(brightnessValue);  /// Brightness setting

  LEDBrightness = 50 + brightnessValue * 20;
  FastLED.setBrightness(LEDBrightness); //Can be put inside FOR Loop
}

void streamCallback(StreamData data)
{
  Serial.println("[MESSAGE STREAM] Stream Data1 available...");
  Serial.println("STREAM PATH: " + data.streamPath());
  FirebaseJson *json = data.jsonObjectPtr();
  String jsonStr;
  json->toString(jsonStr, true);
  StaticJsonDocument<1024> message;
  deserializeJson(message,jsonStr);
  serializeJsonPretty(message, Serial);
  Serial.println();
  //-----------Check for Message Stream Firing multiple times--------------
  if(String(currentMessageID) == message["messageId"].as<String>()){
    Serial.println("Message has been displayed before. Skipping.");
    return;
  }
  //-----------Check for Message Stream Firing at start--------------
  if(unixTime != 0 && message["messageTime"].as<signed long>() != 0){
    Serial.println("Both unixTime and messageTime valid, checking validity."); 
    if((unixTime - message["messageTime"].as<signed long>()) > 1800){
      Serial.println("Message outdated. Skipping...");
      return; 
    }
  }
  //-----------Check for If Message Stream is intended for this clock--------------
  JsonArray arr = message["audience"].as<JsonArray>();
  boolean isIntendedAudience = false;
  for (JsonVariant value : arr) {
    if(value.as<String>()==String(category)){
      isIntendedAudience = true;
    }
  }
  if(!isIntendedAudience){
    Serial.println("Message sent is not for this clock category, skipping!");
    return;
  }
  
  String  messageText = message["messageText"];
  strcpy(newMessage, messageText.c_str()); //Copy message content
  Serial.print("Copying new message: "); Serial.println(newMessage);
  Serial.println("----------------------------------");
  newMessageAvailable = true;
  strcpy(curMessage, newMessage);
  Serial.println("Resetting display and starting to display the message");
  P.displayReset(); //Readys the display
  seq1_num = 70;
  Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void initiate(){
  if (initiated == UNINITIATED) {
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
    File configFile = SPIFFS.open("/config.json","r");
    if (!configFile) {
      Serial.println("Failed to open config file");
      return;
    }
    StaticJsonDocument<256> jsonBuffer;
    deserializeJson(jsonBuffer,configFile);
    configFile.close();
    if(jsonBuffer["clockID"].isNull()){
      P.displayText("CONFIG REQ", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      if (P.displayAnimate()) P.displayReset();
      Serial.println("Config Is not set");
      Serial.println("FIRST TIME SETUP INITIATED");
      Serial.println("WELCOME TO iCLOCK! *Make sure your Serial Interface has NL line endings!");
      Serial.println("THIS ACTION IS IRREVERSABLE! CHECK BEFORE ENTERING!");
      Serial.print("ENTER THIS CLOCK\'S ID: ");
      String input = "";
      boolean waitInput = true;
      while(waitInput){
        if(Serial.available()){
            char lastChar = Serial.read();
          if(lastChar != '\n')input += lastChar;
          else waitInput = false;
        }
      }
      Serial.println(input);
      jsonBuffer["clockID"] = input;
      configFile = SPIFFS.open("/config.json","w");
      if(!configFile){
        Serial.println(F("Failed to open config.json"));
      }
      if(serializeJson(jsonBuffer,configFile)==0)Serial.println(F("Failed to write to file"));
      configFile.close();
      Serial.println("Writing to config.json complete!, First Time setup finish");
    }
      jsonBuffer["clockID"].as<String>().toCharArray(clockID,3);
    Serial.print("THIS CLOCK ID IS: ");
    Serial.println(clockID);
    
    //Speaker Volume
    Serial.print("Initiated Speaker, Volume Set To "); Serial.println(volume);
    myMP3.volume(volume);

    //DHT Sensor
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.print("Temperature Sensor Initiated - ");

    //Brightness sensor
    pinMode(36, INPUT);
    if (SENSOREXIST == 1) {
      TEMT6000.begin(SMOOTHED_AVERAGE, 20);
      for (int i = 0; i < 20; i++) {
        TEMT6000.add(analogRead(SENSOR_PIN)); // Read a sensor value from analogue pin 0
      }
      setBrightness(LUX);
    }
    else P.setIntensity(2);  // Brightness setting
    Serial.println("Brightness Initiated");

    Serial.println("Main Display Initiated, Displaying {Connecting}");
    P.displayText("CONNECTING", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    //P.displayText("CONNECTING WIFI", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    if (P.displayAnimate()) P.displayReset();
    delay(1);
    Serial.println("Components without WIFI Initiated");
    initiated = WAITINGFORWIFI;
    seqW_num = 10;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");

    if (P.displayAnimate()) P.displayReset();
    P.displayText("SETTING UP", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    //  if(initiated == WIFIREADY){
    Serial.print("Connecting to Firebase - ");
    //Firebase Setup
    Firebase.setMaxRetry(firebaseData, 5);
    Firebase.setMaxErrorQueue(firebaseData, 5);
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Firebase.setStreamTaskStackSize(10000);


    Serial.println("Re-creating addresses: ");
    sprintf(AddressTEMP, "%s%s", clocks, clockID);
    Serial.println(AddressTEMP);
    Serial.println("[FIREBASE] Getting Firebase Configs");
    while(!Firebase.get(firebaseData,AddressTEMP)){ //Geting config at /clocks/{clock_name}/
      Serial.println("[IMPORTANT] Update with Firebase Config Failed");
      Serial.println(firebaseData.errorReason());
      Serial.println("Retrying");
    } 
    FirebaseJson *json = firebaseData.jsonObjectPtr(); //FirebaseJson to ArduinoJson Conversion
    String jsonStr;
    json->toString(jsonStr, true);
    StaticJsonDocument<256> config;
    deserializeJson(config,jsonStr);
    config["category"].as<String>().toCharArray(category,15); //as<String>().toCharArray(_,_) seems like the easiest way to convert strings to char array
    Serial.println("Updated Firebase Configs");
    Serial.print("Category: ");
    Serial.println(category);
    
    Serial.println("[FIREBASE] Getting Period Info");
    while(!Firebase.get(firebaseData,AddressGlobalPeriod)){ //Get period info at /global/periodInfo
      Serial.println("[IMPORTANT] Update with Firebase Peroid Info Failed");
      Serial.println(firebaseData.errorReason());
      Serial.println("Retrying");
    } 
    FirebaseJsonArray *json2 = firebaseData.jsonArrayPtr(); //FirebaseJson to ArduinoJson Conversion
    String jsonStr2;
    json2->toString(jsonStr2, true);
    StaticJsonDocument<256> periods;
    deserializeJson(periods,jsonStr2);
    JsonArray arr = periods.as<JsonArray>();
    Serial.println("Updated Firebase Period Info");
    for(int i = 0; i < arr.size(); i++){ //mapping firebase array to local array
        arr[i].as<String>().toCharArray(periodInfo[i],5);
        Serial.println(periodInfo[i]);
    }
    Serial.println("-------------------------------------------------------");

    //Setting up OTA
    sprintf(clockName, "%s%s%s", FIREBASE_HOST, clocks, clockID);
    Serial.print("Setting up OTA - ");
    ArduinoOTA.setHostname(clockName);
    ArduinoOTA.setPassword("692`");
    ArduinoOTA.setPort(8266);
    ArduinoOTA.onStart([]() {
      P.displayText("OTA UPDATE", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      if (P.displayAnimate()) P.displayReset();

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
    Serial.print("OTA Ready, my IP: ");
    Serial.println(WiFi.localIP());
    //Config NTP
    Serial.print("Configuring Time ");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    int NTPRetryCount = 0;
    while (secondChange() == 0) {
      //Serial.println("Chillin waiting for edge of second");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      delay(1);
      NTPRetryCount++;
      if (NTPRetryCount == 10000) {
        Serial.println("Failed to find edge of second, miliSecond might not be accurate");
        break;
      }
    }
    struct tm timeinfo;
    Serial.print("Final Check - ");
    if (!getLocalTime(&timeinfo)) { // Error, cannot obtain time
      Serial.println("Failed, will reinitiate components that require WIFI");
      //    blacklistStrike++;
      //    if(blacklistStrike == blacklistThreshold){
      //      blacklist[blacklistMax] = wifiInfo[knownNI];
      //    }
    }
    else {
      if (!Firebase.beginStream(firebaseData2, path))
      {
        Serial.println("------------------------------------");
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }
    
      Firebase.setStreamCallback(firebaseData2, streamCallback, streamTimeoutCallback);
      initiated = INITIATED;
      Serial.println("Passed, initiating all sequences");
      seq1_num = 10;
      seq2_num = 10;
      seq3_num = 10;
      seq4_num = 10;
    }
  }
}

void loopRedTheme(void) {
  fill_rainbow(leds, NUM_LEDS, gHue, 2);
  if (rainbowDir == 1) { //FW
    gHue++;
    if (gHue == hueOrange) { //upper limit
      rainbowDir = 0;
      //Serial.println("Currently Yellow, turning the other way now");
    }
  }
  else if (rainbowDir == 0) { //BW
    gHue--;
    if (gHue == huePink + 20) { //lower limit //Either larger than hue pink or smaller than hue red
      rainbowDir = 1;
      //Serial.println("Currently pink, turning the other way now");
    }
  }
  FastLED.show();
}

void loopGreenTheme(void) {
  fill_rainbow(leds, NUM_LEDS, gHue, 3);
  if (rainbowDir == 1) { //FW
    gHue++;
    if (gHue == hueAqua) { //upper limit
      rainbowDir = 0;
      //Serial.println("Currently Aqua, turning the other way now");
    }
  }
  else if (rainbowDir == 0) { //BW
    gHue--;
    if (gHue == hueYellow + 20) { //lower limit
      rainbowDir = 1;
      //Serial.println("Currently Yellow, turning the other way now");
    }
  }
  FastLED.show();
}


void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_FLOAT)
        Serial.println(jsonData.floatValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
               jsonData.typeNum == FirebaseJson::JSON_NULL ||
               jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
}

void printResult(StreamData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string" || data.dataType() == "null")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson *json = data.jsonObjectPtr();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json->toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json->iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json->iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json->iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray *arr = data.jsonArrayPtr();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr->toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();

    for (size_t i = 0; i < arr->size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData *jsonData = data.jsonDataPtr();
      //Get the result data from FirebaseJsonArray object
      arr->get(*jsonData, i);
      if (jsonData->typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData->boolValue ? "true" : "false");
      else if (jsonData->typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData->intValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_FLOAT)
        Serial.println(jsonData->floatValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData->doubleValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_STRING ||
               jsonData->typeNum == FirebaseJson::JSON_NULL ||
               jsonData->typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData->typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData->stringValue);
    }
  }
}

//== Timer Function ====  ===========================================================
byte set_timer_milli_sec(byte timer_number, unsigned long time_set_value) {
  switch (timer_number)
  {
    case TIM_0:   time0_data = millis();  time0_set = time_set_value; tim0 = TIMSET;  return 1; break;
    case TIM_1:   time1_data = millis();  time1_set = time_set_value; tim1 = TIMSET;  return 1; break;
    case TIM_2:   time2_data = millis();  time2_set = time_set_value; tim2 = TIMSET;  return 1; break;
    case TIM_3:   time3_data = millis();  time3_set = time_set_value; tim3 = TIMSET;  return 1; break;
    case TIM_3A:  time3A_data = millis(); time3A_set = time_set_value; tim3A = TIMSET;  return 1; break;
    case TIM_4:   time4_data = millis();  time4_set = time_set_value; tim4 = TIMSET;  return 1; break;
    case TIM_W:   timeW_data = millis();  timeW_set = time_set_value; timW = TIMSET;  return 1; break;
    default:  return 0; break;
  }
}

byte timer_milli_sec(byte timer_number) {
  switch (timer_number)
  {
    case TIM_0:  if (tim0) {
        if ((millis() - time0_data) >= time0_set) tim0 = TIMOUT;
      } return tim0;  break;
    case TIM_1:  if (tim1) {
        if ((millis() - time1_data) >= time1_set) tim1 = TIMOUT;
      } return tim1;  break;
    case TIM_2:  if (tim2) {
        if ((millis() - time2_data) >= time2_set) tim2 = TIMOUT;
      } return tim2;  break;
    case TIM_3:  if (tim3) {
        if ((millis() - time3_data) >= time3_set) tim3 = TIMOUT;
      } return tim3;  break;
    case TIM_3A: if (tim3A) {
        if ((millis() - time3A_data) >= time3A_set)tim3A = TIMOUT;
      } return tim3A; break;
    case TIM_4:  if (tim4) {
        if ((millis() - time4_data) >= time4_set) tim4 = TIMOUT;
      } return tim4;  break;
    case TIM_W:  if (timW) {
        if ((millis() - timeW_data) >= timeW_set) timW = TIMOUT;
      } return timW;  break;
    default:  return 1; break;
  }
}
