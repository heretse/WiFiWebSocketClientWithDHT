
/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using a WiFi shield.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * WiFi shield attached

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */

#define READ_SD_CARD 1

#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

#define SD_CARD_PIN 4
#define WIFI_PIN   10
#define BUTTON_PIN  A0   

#include "DHT.h"
#define DHT_PIN A2
#define DHT_TYPE DHT22

File myFile;

char ssid[32] = "iSunGuest_2.4G"; // your network SSID
char pass[63] = "isunguest";      // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(10,0,1,2);  // numeric IP for WebSocket server (no DNS)
//int port = 8000;
//char server[] = "wot.city";    // name address for WebSocket server (using DNS)
char server[] = "winston-iot-dashboard.mybluemix.net";
int port = 80;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

DHT dht(DHT_PIN, DHT_TYPE);

int buttonState = 0; // variable for reading the pushbutton status

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(BUTTON_PIN, INPUT);
  
#if READ_SD_CARD
  // disable wifi while starting SD
  pinMode(WIFI_PIN, OUTPUT);
  digitalWrite(WIFI_PIN, HIGH);
  
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(SD_CARD_PIN)) {
    Serial.println(F("initialization failed!"));
    return;
  }
  Serial.println(F("initialization done."));

  if (SD.exists("wifi.cfg")) {
    Serial.println(F("wifi.cfg exists."));
    
    myFile = SD.open("wifi.cfg", FILE_READ);
    if (myFile) {
      Serial.println(F("wifi.cfg:"));

      int lineCnt = 0;
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        //Serial.write(myFile.read());
        String tmp = myFile.readStringUntil('\n');
        Serial.println(tmp);
        if (lineCnt == 0) {
          tmp.toCharArray(ssid, 32);
        } else if (lineCnt == 1) {
          tmp.toCharArray(pass, 63);
        }

        lineCnt++;
      }
      
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println(F("error opening test.txt"));
    }
  } else {
    Serial.println(F("wifi.cfg doesn't exist."));
  }
#endif

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present."));
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println(F("Please upgrade the firmware."));
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.print(ssid);
    Serial.print(" with password: ");
    Serial.println(pass);
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    if (strlen(pass) != 0) {
      Serial.println(F("Connect to WPA/WPA2 network."));
      status = WiFi.begin(ssid, pass);
    } else {
      Serial.println(F("Connect to open network."));
      status = WiFi.begin(ssid);
    }

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println(F("Connected to wifi"));
  printWifiStatus();

  Serial.println(F("\nStarting connection to server..."));
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println(F("connected to server."));
    String strHost = "Host: " + String(server);
    
    // Make a HTTP request
//    client.println("GET /object/563af4969903aade72000c25/send HTTP/1.1");
      client.println("GET /ws/send HTTP/1.1");
    client.println("Upgrade: WebSocket");
    client.println("Connection: Upgrade");
    client.println(strHost);
    client.println("Origin: WebSocketClient");
//    client.println("Sec-WebSocket-Extensions: permessage-deflate");
    client.println("Sec-WebSocket-Key: DMF3ByMTLq+cp7AyMN0qUA==");
    client.println("Sec-WebSocket-Version: 13");
    client.println();
    delay(500);
  }
  else {
    // you didn't get a connection to the server:
    Serial.println(F("connection failed."));
  }

  dht.begin();
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  monitor();

  // Read humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(2000);
    return;
  }

  String sendData = "{\"temperature\":" + String(t) + ", \"humidity\":" + String(h) + "}"; 
  Serial.println(sendData);
  send(sendData);

  delay(5000);
  
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    client.stop();
    WiFi.disconnect();
  }

  // if the server's disconnected, stop the client:
  if (!client.connected() || !Serial) {
    Serial.println();
    Serial.println(F("disconnecting from server."));
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void monitor() {
  char character = -1;
  
  if (client.available() && (character = client.read()) >= 0) {
    String data = "";
    
    bool endReached = false;
    while (!endReached) {
      character = client.read();
      endReached = (character == -1);
      if (!endReached) {
        data += character;
      }
    }

    Serial.println(data);
    client.flush();
  }
}

void send(String data) {
  
  client.print((char)129);
  client.print((char)data.length());
  client.print(data);
  
}
