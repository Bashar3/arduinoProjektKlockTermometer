
#include  <Adafruit_NeoPixel.h>
#include  <Adafruit_TiCoServo.h>
#include  <RtcDS3231.h>
#include  <SoftwareSerial.h>  
#include  "SSD1306AsciiWire.h"
#include  "WiFiEsp.h"
#include  <Wire.h>

const byte neoPixels = 24;
const byte neoPin = 10;

char ssid[] = "";
char pass[] = "";

const char server[] = "thingspeak.com";
const char thingspeakAPIKey [] ="";
long postingInterval = 30;

SSD1306AsciiWire oled;

Adafruit_NeoPixel ring = Adafruit_NeoPixel(neoPixels, neoPin, NEO_GRB);

RtcDS3231<TwoWire> rtcModule(Wire);

WiFiEspClient client;

Adafruit_TiCoServo servo;

SoftwareSerial Serial1(6, 7);

int status = WL_IDLE_STATUS;

int hours;
int minutes;
int seconds;
float temp;

int oldMinute;
char lastSent[20];


void setup() {
  // put your setup code here, to run once:

  Wire.begin();

  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.print("Starting");

  Serial.begin(115200);
  servo.attach(9);

  rtcModule.Begin();

  ring.begin();
  ring.setBrightness(100);
  ring.show();

  oled.print(".");

  Serial1.begin(9600);

  WiFi.init(&Serial1);

  oled.print(".");

  if (WiFi.status() == WL_NO_SHIELD){

    Serial.println ("WiFi shield not present");

    while(true);
  }

  while (status != WL_CONNECTED){

    Serial.print("Attempting to connect to SSID");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);

    oled.print(".");
  }

  printWifiStatus();
  printOled();
  updateTemp();

}

void loop() {
  // put your main code here, to run repeatedly:

  updateTime();
  updateNeoRing();

  if (seconds%postingInterval == 0){

    updateTemp();
    sendThingspeak(temp);
  }

  client.flush();
  client.stop();
  delay(500);

}
// send data to thingspeak.com

void sendThingspeak(float value){

  if (client.connectSSL(server, 443)){

    Serial1.println("Connected to server.");
    client.println("Get /update?api_key=" + String(thingspeakAPIKey) +
    "&field1=" + String(value) + " HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println();
    Serial.println("Sent to server.");

    sprintf(lastSent, "Sent: %02d:%02d:%02d", hours, minutes, seconds);

    printOled();
    delay(1000);
    
  }
}

//update time
void updateTime(){

  oldMinute = minutes;
  RtcDateTime now = rtcModule.GetDateTime();

  hours   = now.Hour();
  minutes = now.Minute();
  seconds = now.Second();

  if (minutes != oldMinute){

    printOled();
 

  
}
updateNeoRing();
}
// print to display
void printOled(){

  oled.clear();
  oled.setFont(lcdnums14x24);
  oled.setCol(28);
  char timeString[6];
  sprintf(timeString, "%02d:%02d", hours,minutes);
  oled.println(timeString);
  

  oled.setFont(utf8font10x16);
  oled.setRow(6);
  oled.setCol(112);
  oled.print(int(temp));
  oled.write(176);

  oled.setCol(0);
  oled.print(lastSent);
}

void updateNeoRing(){
  
// balance to 24 pixels neoRing
  int neoMin = round(minutes/2.5-1);
  int neoHour = hours%12*2-1;

  int blue;
  int red;

  for (int i  = 0; i <= neoPixels; i++){

    if (i <= neoMin){

      blue = 255;
    }else {
      blue = 0;
    }

    if (i <= neoHour){

      red = 255;
    }else {

      red = 0;
    }
    
    ring.setPixelColor(i, ring.Color(red, 0, blue));
  }

  ring.show();

  
}

//update tempereture
void updateTemp(){

  RtcTemperature rtcTemp = rtcModule.GetTemperature();
  temp = rtcTemp.AsFloatDegC();

  int tempPointer = map(temp, 18, 28, 30, 140);

  servo.write(tempPointer);
}

void printWifiStatus(){

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Adress: ");
  Serial.println(ip);
}
