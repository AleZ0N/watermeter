#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <data.h>

#define TFT_MOSI 6
#define TFT_SCLK 4
#define TFT_CS   10
#define TFT_DC   2
#define TFT_RST  3

const char* ssid = "<SSID>";
const char* pass = "<password>";
const int levMin = 1150;
const int levMax = 340;
const int levMaxDiff = 80;

int localLED = 8;
int trig = 0;
int echo = 1;
int lastLevel = 0;
const int range = levMin - levMax;
String header;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
WiFiServer server(80);

void setup() {
  pinMode(localLED, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  tft.init(135, 240, SPI_MODE3); 
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1);

  WiFi.begin(ssid, pass);

  /*
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();*/

  tft.drawRGBBitmap(0, 85, bitmap_tank, 135, 150);
}

void loop() {  
  long t, s;
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  t = pulseIn(echo, HIGH);
  s = (t/2) * 0.343;

  char pers[20] = "  0 %";
  int level = 0;
  int perColor = ST77XX_GREEN;
  if (s > levMin) {
    level = 0;
    sprintf(pers, "  0 %%", level);
  }
  else if (s < levMax) {
    level = 100;
    sprintf(pers, "100 %%", level);
    if (s < levMax - levMaxDiff) {
      level = 101;
      sprintf(pers, "  !!!");
      perColor = ST77XX_RED;
    }
  }
  else {
    level = 100 - (s - levMax) * 100 / range;
    sprintf(pers, "%3i %%", level);
  }

  char dist[15] = " ???";
  sprintf(dist, " %i", s);

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setFont(&FreeSans9pt7b);

  tft.setCursor(5, 15);
  if (WiFi.status() == WL_CONNECTED) {
    server.begin();
    tft.drawRGBBitmap(100, 0, bitmap_wifi_on, 24, 24);
  }
  else {
    tft.drawRGBBitmap(100, 0, bitmap_wifi_off, 24, 24);
  }

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.fillRect(60, 25, 75, 30, ST77XX_BLACK);
  tft.setCursor(60, 45);
  tft.println(dist);
  switch (acount) {
    case 0: 
      tft.drawRGBBitmap(0, 30, bitmap_wave1, 60, 20);
      break;
    case 1: 
      tft.drawRGBBitmap(0, 30, bitmap_wave2, 60, 20);
      break;
    case 2: 
      tft.drawRGBBitmap(0, 30, bitmap_wave3, 60, 20);
      break;
    case 3: 
      tft.drawRGBBitmap(0, 30, bitmap_wave4, 60, 20);
      break;
    case 4: 
      tft.drawRGBBitmap(0, 30, bitmap_wave5, 60, 20);
      break;
  }
  acount = acount + 1;
  if (acount == 5) {
    acount = 0;
  }

  if (lastLevel != level) {
    tft.setFont(&FreeSansBold18pt7b);
    tft.setTextColor(perColor, ST77XX_BLACK);
    tft.fillRect(0, 55, 135, 50, ST77XX_BLACK);
    tft.setCursor(25, 95);
    tft.println(pers);

    tft.fillRect(25, 210, 87, -100, ST77XX_BLACK);
    if (level == 101) {
      tft.fillRect(25, 210, 87, -100, ST77XX_RED);
    }
    else if (level >= 0) {
      tft.fillRect(25, 210, 87, level * -1, ST77XX_BLUE);
    }
    lastLevel = level;
  }

  digitalWrite(localLED, HIGH);
  WiFiClient client = server.available();
  
  if (client) {
    String currentLine = "";
    digitalWrite(localLED, LOW);
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/plain");
            client.println("Connection: close");
            client.println();

            char resp[20];
            sprintf(resp, "%i", level);
            client.println(resp);
            client.println();
            break;
          }
          else { 
            currentLine = "";
          }
        } 
        else if (c != '\r') { 
          currentLine += c; 
        }
      }
    }
    header = "";
    client.stop();
    delay(1000);
    digitalWrite(localLED, HIGH);
  }

  delay(2000);
}
