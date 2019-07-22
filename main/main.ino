\#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <YoutubeApi.h>
#include <MusicEngine.h>
#include <SNTPtime.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "RGB.h"



#define API_KEY "API"
#define CHANNEL_ID "ID"
const char ssid[] = "ssid";
const char password[] = "pass";

// ================================================ PIN DEFINITIONS ======================================

#define BUZ_PIN D5
#define NEO_PIN D4
#define POWER_PIN D1

#define NUMBER_OF_DIGITS 6
#define STARWARS "t112v127l12<dddg2>d2c<ba>g2d4c<ba>g2d4cc-c<a2d6dg2>d2c<ba>g2d4c<ba>g2d4cc-c<a2"

#define SUBSCRIBER_INTERVAL 30000
#define NTP_LOOP_INTERVAL 60000
#define DISP_LOOP_INTERVAL 500

#define MAX_BRIGHTNESS 50

strDateTime timeNow;
int lastHour = 0;

unsigned long entrySubscriberLoop, entryNTPLoop, entryDispLoop;

struct subscriberStruc {
  long last;
  long actual;
  long old[24];
} subscribers;

//Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 5, 1, NEO_PIN,
//                            NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
//                            NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
//                            NEO_GRB + NEO_KHZ800);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 6 , 1, NEO_PIN,
                            NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);


#define LED_PIN    D4
#define LED_COUNT 320
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 128, 0), matrix.Color(0, 0, 255), matrix.Color(0, 0 , 128)
};



MusicEngine music(BUZ_PIN);

WiFiClientSecure client;
YoutubeApi api(API_KEY, client);
SNTPtime NTPch("ch.pool.ntp.org");

// ================================================ SETUP ================================================
void setup() {
  Serial.begin(115200);
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // Attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi N/W : ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  pinMode(POWER_PIN, INPUT);
  Serial.println("Start");

  matrix.begin();
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW);
  matrix.show();
  //  displayText("P R A J");
  pinMode(A0, INPUT);

  while (!NTPch.setSNTPtime()) Serial.print("."); // set internal clock
  entryNTPLoop = millis() + NTP_LOOP_INTERVAL;
  entrySubscriberLoop = millis() + SUBSCRIBER_INTERVAL;
  //  beepUp();
  beepUp();
  while (subscribers.actual == 0) {
    api.getChannelStatistics(CHANNEL_ID);
    subscribers.actual = api.channelStats.subscriberCount;
  }
  for (int i = 0; i < 24; i++) {
    if (subscribers.old[i] == 0) subscribers.old[i] = subscribers.actual - 245;
  }
  debugPrintSubs();
  Serial.println("Setup done");
}
uint8_t counter = 0;
void loop() {
  int buttonstate = digitalRead(D0);
  delay(120);
  if (buttonstate == HIGH) {
    counter++;
    counter %= 4;
    Serial.print(counter);
    delay(10000);
  }
  switch (counter) {
    case 1:
      displayText("PRAJ");
      for (int f = 0; f <= 7; f++) {
        matrix.drawPixel(0, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(1, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(2, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(3, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(4, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(5, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(6, f, matrix.Color(50, 0, 0));
        matrix.drawPixel(7, f, matrix.Color(50, 0, 0));
      }
      matrix.drawPixel(0, 0, matrix.Color(0, 0, 0));
      matrix.drawPixel(7, 0, matrix.Color(0, 0, 0));
      matrix.drawPixel(0, 7, matrix.Color(0, 0, 0));
      matrix.drawPixel(7, 7, matrix.Color(0, 0, 0));
      drawLogo();
      break;
    case 2:
            looper();
      break;
    case 3:
      // Fill along the length of the strip in various colors...
            colorWipe(strip.Color(2,   0,   0), 5); // Red
            colorWipe(strip.Color(  0, 2,   0), 5); // Green
            colorWipe(strip.Color(  0,   0, 2), 5); // Blue
      // Do a theater marquee effect in various colors...
            theaterChase(strip.Color(12, 12, 12), 5); // White, half brightness
            theaterChase(strip.Color(12,   0,   0), 5); // Red, half brightness
            theaterChase(strip.Color(  0,   0, 12), 5); // Blue, half brightness
            rainbow();             // Flowing rainbow cycle along the whole strip
            theaterChaseRainbow(10); // Rainbow-enhanced theaterChase variant
      break;
  }
}

void looper() {
  if (millis() - entryDispLoop > DISP_LOOP_INTERVAL) {
    entryDispLoop = millis();
    displayNeo(subscribers.actual, subscribers.actual - subscribers.old[timeNow.hour]);
  }

  if (millis() - entryNTPLoop > NTP_LOOP_INTERVAL) {
    //   Serial.println("NTP Loop");
    entryNTPLoop = millis();
    timeNow = NTPch.getTime(1.0, 1); // get time from internal clock
    NTPch.printDateTime(timeNow);
    if (timeNow.hour != lastHour ) {
      Serial.println("New hour!!!!!!!");
      subscribers.old[lastHour] = subscribers.actual;
      subscribers.last = subscribers.actual;
      debugPrintSubs();
      lastHour = timeNow.hour;
    }
  }

  if (millis() - entrySubscriberLoop > SUBSCRIBER_INTERVAL) {
    //   Serial.println("Subscriber Loop");
    entrySubscriberLoop = millis();
    updateSubs();
  }
}

void updateSubs() {
  if (api.getChannelStatistics(CHANNEL_ID))
  {
    // get subscribers from YouTube
    //      Serial.println("Get Subs");
    subscribers.actual = api.channelStats.subscriberCount;
    displayNeo(subscribers.actual, subscribers.actual - subscribers.old[timeNow.hour]);
    Serial.print("Subs ");
    Serial.print(subscribers.actual);
    Serial.print(" yesterday ");
    Serial.println(subscribers.old[timeNow.hour]);
    if (subscribers.last > 0) {
      if (subscribers.actual > subscribers.last ) {
        matrix.setTextColor(colors[1]);
        beepUp();
        if (subscribers.actual % 10 <= subscribers.last % 10) for (int ii = 0; ii < 1; ii++) beepUp();
        if (subscribers.actual % 100 <= subscribers.last % 100) starwars();
        if (subscribers.actual % 1000 <= subscribers.last % 1000) for (int ii = 0; ii < 2; ii++) starwars();
      }
      else {
        if (subscribers.actual < subscribers.last ) {
          beepDown();
          matrix.setTextColor(colors[0]);
        }
      }
    }
    matrix.show();
    subscribers.last = subscribers.actual;
    //     debugPrint();
  }
}

void displayNeo(int subs, int variance ) {
  // Serial.println("Display");

  matrix.fillScreen(0);
  int bright = measureLight();
  if (bright > -1) {
    matrix.setBrightness(bright);
    matrix.setTextColor(colors[3]);
    matrix.setCursor(0, 0);
    matrix.print(String(subs));

    // Show arrow
    char arrow = (variance <= 0) ? 0x1F : 0x1E;
    if (variance > 0) matrix.setTextColor(colors[1]);
    else matrix.setTextColor(colors[0]);
    matrix.setCursor(33, 0);
    matrix.print(arrow);

    // show variance bar
    int h = map(variance, 0, 400, 0, 8);
    h = (h > 8) ? 8 : h;
    if (h > 0) matrix.fillRect(42, 8 - h,  1, h , colors[3]);
  }
  matrix.show();
}

void beepUp() {
  music.play("T80 L40 O4 CDEFGHAB>CDEFGHAB");
  while (music.getIsPlaying() == 1) yield();
  delay(500);
}

void beepDown() {
  music.play("T1000 L40 O5 BAGFEDC<BAGFEDC<BAGFEDC");
  while (music.getIsPlaying() == 1) yield();
  delay(500);
}

void starwars() {
  music.play(STARWARS);
  while (music.getIsPlaying()) yield();
  delay(500);
}

int measureLight() {
  int brightness = map(analogRead(A0), 360, 800, MAX_BRIGHTNESS, 0);
  brightness = (brightness > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : brightness;  // clip value
  brightness = (brightness < 20) ? 0 : brightness;
  // brightness = MAX_BRIGHTNESS;
  return brightness;
}

void debugPrint() {
  Serial.println("---------Stats---------");
  Serial.print("Subscriber Count: ");
  Serial.println(subscribers.actual);
  Serial.print("Variance: ");
  Serial.println(subscribers.actual - subscribers.old[timeNow.hour]);
  Serial.print("LastSubs: ");
  Serial.println(subscribers.last);
  Serial.print("View Count: ");
  Serial.println(api.channelStats.viewCount);
  Serial.print("Comment Count: ");
  Serial.println(api.channelStats.commentCount);
  Serial.print("Video Count: ");
  Serial.println(api.channelStats.videoCount);
  // Probably not needed :)
  Serial.print("hiddenSubscriberCount: ");
  Serial.println(subscribers.actual);
  Serial.println("------------------------");
}
void debugPrintSubs() {
  for (int i = 0; i < 24; i++) {
    Serial.print(i);
    Serial.print(" old ");
    Serial.println(subscribers.old[i]);
  }
}
void displayText(String tt) {
  matrix.setCursor(13, 0);
  matrix.setTextWrap(false);
  matrix.setBrightness(30);
  matrix.fillScreen(0);
  matrix.setTextColor(colors[0]);
  matrix.print(tt);
  matrix.show();
}

void rainbow() {
  strip.begin();
  strip.show();
  strip.setBrightness(3);
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(1);  // Pause for a moment
  }
}
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++) { // Repeat 30 times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 10; a++) { // Repeat 10 times...
    for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
void drawLogo() {
  // This 8x8 array represents the LED matrix pixels.
  // A value of 1 means we’ll fade the pixel to white
  int logo[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 8; column++) {
      if (logo[row][column] == 1) {
        fadePixel(column, row, red, white, 3, 0);
      }
    }
  }
}
void fadePixel(int x, int y, RGB startColor, RGB endColor, int steps, int wait) {
  for (int i = 0; i <= steps; i++)
  {
    int newR = startColor.r + (endColor.r - startColor.r) * i / steps;
    int newG = startColor.g + (endColor.g - startColor.g) * i / steps;
    int newB = startColor.b + (endColor.b - startColor.b) * i / steps;

    matrix.drawPixel(x, y, matrix.Color(newR, newG, newB));
    matrix.show();
    delay(wait);
  }
}
void crossFade(RGB startColor, RGB endColor, int steps, int wait) {
  for (int i = 0; i <= steps; i++)
  {
    int newR = startColor.r + (endColor.r - startColor.r) * i / steps;
    int newG = startColor.g + (endColor.g - startColor.g) * i / steps;
    int newB = startColor.b + (endColor.b - startColor.b) * i / steps;

    matrix.fillScreen(matrix.Color(newR, newG, newB));
    matrix.show();
    delay(wait);
  }
}
