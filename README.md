# WiFiWebSocketClientWithDHT
A web socket client with DHT sensor connected by WiFi Shield

## Usage
### Setup your Wifi SSID and password in "wifi.cfg" file saved into a SD caard.

#### The first line is Wifi SSID and the second line is Wifi password.

```txt
iSunGuest_2.4G
iSunGuest
```

### You can choose the specificied Wifi settings read from wifi.cfg or by the following hard code with the READ_SD_CARD value.

```ino
#define SD_CARD_PIN 4

...

#define READ_SD_CARD 1

...

File myFile;

char ssid[32] = "AirPort Time Capsule"; // your network SSID
char pass[63] = "0229647065";      // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                  // your network key Index number (needed only for WEP)

...

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
```

### Define your DHT sensor's digital pin and type.

```ino
...

#include "DHT.h"
#define DHT_PIN A2
#define DHT_TYPE DHT22

...
```
### Define an PIN when it is HIGH to force disconnect the WebSocket and Wifi connections.  

```ino
#define BUTTON_PIN  A0

...

int buttonState = 0; // variable for reading the pushbutton status

...

buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    client.stop();
    WiFi.disconnect();
  }

```