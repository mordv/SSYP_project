#include <Arduino.h>

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <ESP8266mDNS.h>

#define COLOR_ORDER GRB
#define CHIPSET WS2811
#define NUM_LEDS 256
#define BRIGHTNESS 255
#define LED_PIN 12
#define arraySize 16
#define kMatrixSerpentineLayout true

DynamicJsonDocument doc(256);

CRGB leds[NUM_LEDS];
CHSV paleBlue(160, 128, 255);

unsigned long int timing = 0;
int applePosition[2] = {0};
unsigned long int toxicTiming = 0;
int apple_const = -1;
int pos = 0;
int y = 0;
int x = 0;
int a[arraySize][arraySize] = {0};

byte direction = 0;
uint8_t type = 0;
int snake_size = 4;

const char *ssid = "MATRIX";
const char *password = "ssyp2023";
const char *mdnsName = "matrix";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 16;

File fsUploadFile;

bool handleFileRead(String path);
void handleFileUpload();

#define kMatrixSerpentineLayout true

uint32_t x_noise, y_noise, v_time, hue_time, hxy;

uint8_t octaves = 1;
uint8_t hue_octaves = 3;

int xscale = 57771;
int yscale = 57771;

int hue_scale = 1;

int time_speed = 11;
int hue_speed = 1;

int x_speed = 331;
int y_speed = 1111;

void increaseSize();
void handle_ledUpon();
void handle_ledDownon();
void handle_ledRighton();
void handle_ledLefton();
void handle_ledChangeMode();
void display();
void initSnake();
void spawnToxicApple();

void decreaseCells()
{
  for (int i = 0; i < arraySize; i++)
  {
    for (int j = 0; j < arraySize; j++)
    {
      if (a[i][j] > 0)
      {
        a[i][j] -= 1;
      }
    }
  }
}

void twoDimensionalPosition(int n)
{
  int array[2] = {0};
  array[0] = floor(n / 16);
  if (array[0] % 2 == 1)
  {
    array[1] = 16 * (array[0] + 1) - n - 1;
  }
  else
  {
    array[1] = n - 16 * (array[0]);
  }
  for (int i = 0; i < 2; i++)
  {
    applePosition[i] = array[i];
  }
}

void twoDimensionalPositionad(int n)
{
  int array[2] = {0};
  array[0] = floor(n / 16);
  if (array[0] % 2 == 1)
  {
    array[1] = 16 * (array[0] + 1) - n - 1;
  }
  else
  {
    array[1] = n - 16 * (array[0]);
  }
  for (int i = 0; i < 2; i++)
  {
    applePosition[i] = array[i];
  }
}

void rainbowColor(int n)
{
  leds[n] = CHSV(random(255), 255, 255);
}

void spawnApple()
{ // назвать spawnApples
  bool fl = true;
  while (fl)
  {
    int place = random(256);
    if (leds[place] == 0)
    {
      leds[place] = CRGB::Green;
      fl = false;
      twoDimensionalPosition(place);
      a[applePosition[0]][applePosition[1]] = apple_const;
    }
  }
}
void resetDisplay()
{
  for (int i = 0; i < arraySize; i++)
  {
    for (int j = 0; j < arraySize; j++)
    {
      a[i][j] = 0;
    }
  }
  snake_size = 4;
  x = 0;
  y = 0;
}
void spawnHead()
{
  if (direction == 1)
  {
    x = (x + 1) % arraySize;
  }
  else if (direction == 0)
  {
    y = (y + 1) % arraySize;
  }
  else if (direction == 3)
  {
    y = (y - 1) % arraySize;
    if (y < 0)
    {
      y = 15;
    }
  }
  else
  {
    x = (x - 1) % arraySize;
    if (x < 0)
    {
      x = 15;
    }
  }
  if (a[x][y] == -1)
  {
    increaseSize();
    spawnApple();
  }
  else if (a[x][y] > 0)
  {
    resetDisplay();
    delay(100);
    initSnake();
  }
  else if (a[x][y] == -2)
  {
    decreaseCells();
    snake_size -= 1;
    if (snake_size <= 0)
    {
      resetDisplay();
      initSnake();
      clearDisplay();
    }
  }
  a[x][y] = snake_size;
}

void moveSnake()
{
  decreaseCells();
  spawnHead();
}

void initSnake()
{
  for (int i = 0; i < snake_size; ++i)
  {
    a[0][i] = i + 1;
  }
  x = 0;
  y = snake_size - 1;
  direction = 1;
  spawnApple();
}

void increaseSize()
{
  for (int i = 0; i < arraySize; i++)
  {
    for (int j = 0; j < arraySize; j++)
    {
      if (a[i][j] > 0)
      {
        a[i][j] += 1;
      }
    }
  }
  snake_size += 1;
}

void snake()
{
  if (millis() - timing >= 100)
  {

    timing = millis();
    moveSnake();
    if (millis() - toxicTiming >= 30000)
    {
      spawnToxicApple();
      toxicTiming = millis();
    }
    display();
    FastLED.show();
  }
  if (millis() - timing >= 10000)
  {
    resetDisplay();
    delay(100);
  }
}

void noise()
{
  fill_2dnoise16(leds, kMatrixWidth, kMatrixHeight, kMatrixSerpentineLayout,
                 octaves, x_noise, xscale, y_noise, yscale, v_time,
                 hue_octaves, hxy, hue_scale, hxy, hue_scale, hue_time, false);

  FastLED.show();

  x_noise += x_speed;
  y_noise += y_speed;
  v_time += time_speed;
  hue_time += hue_speed;
}

void setup()
{
  Serial.begin(9600);

  FastLED.setBrightness(BRIGHTNESS);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // Upping WiFi server
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.persistent(false);
  delay(100);
  initSnake();

  ArduinoOTA.setHostname("matrix");

  ArduinoOTA.onStart([]()
                     { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();

  Serial.println("OTA ready");

  SPIFFS.begin();

  MDNS.begin(mdnsName);

  server.on("/ledUpon", handle_ledUpon);

  server.on("/ledDownon", handle_ledDownon);

  server.on("/ledRighton", handle_ledRighton);

  server.on("/ledNoise", handle_ledNoise);

  server.on("/ledLefton", handle_ledLefton);

  server.on("/ledChange_mode", handle_ledChangeMode);

  server.on("/ledChangeMode", handle_ledChangeMode);

  server.on("/ledNetworkColoring", handle_ledNetworkColoring);

  server.on("/upload", HTTP_GET, []()
            { server.send(200, "text/html", "<!DOCTYPE html><html><head> <meta http-equiv='Content-Type' content='text/html; charset=utf-8'> <title>Загрузка</title></head><body> <form method='post' enctype='multipart/form-data'><input type='file' name='name'><input class='button' type='submit'  value='Upload'></form></body></html>"); });

  server.on(
      "/upload", HTTP_POST, // if the client posts to the upload page
      []()
      { server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
      handleFileUpload       // Receive and save the file
  );

  server.onNotFound([]() {             // If the client requests any URI
    if (!handleFileRead(server.uri())) // send it if it exists
      server.send(404, "text/plain", "404: Not Found");
  });

  server.begin();
}

int cordsTransformation(int x, int y)
{
  return (x * arraySize) + (x % 2 ? arraySize - y - 1 : y);
}

void display()
{
  for (int i = 0; i < arraySize; i++)
  {
    for (int j = 0; j < arraySize; j++)
    {
      int val = a[i][j];

      int n = cordsTransformation(i, j);
      if (a[i][j] > 0 && a[i][j] != apple_const && a[i][j] != -2)
      {
        pos = i * j;
        int y1 = pos / 16;
        rainbowColor(n);
        // Serial.println(j);
      }
      else if (leds[n] != CRGB::Green && a[i][j] != -2)
      {
        leds[n] = 0;
      }
    }
  }
}

void spawnToxicApple()
{
  bool fl = true;
  while (fl)
  {
    int place = random(256);
    if (leds[place] == 0)
    {
      leds[place] = CHSV(128, 255, 255);
      fl = false;
      twoDimensionalPosition(place);
      a[applePosition[0]][applePosition[1]] = -2;
    }
  }
}

void printArray()
{
  for (int i = 0; i < arraySize; i++)
  {
    for (int j = 0; j < arraySize; j++)
    {
      Serial.print(a[i][j]);
      Serial.print(',');
    }
    Serial.println();
  }
  Serial.println();
}

void loop()
{
  server.handleClient();
  ArduinoOTA.handle();

  if (type == 0)
  {
    snake();
  }
  else if (type == 2)
  {
    noise();
  }
}

void ok()
{
  server.send(200, "text/html", "ok");
}

void clearDisplay()
{
  for (int i = 0; i <= 255; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void handle_ledNoise()
{
  deserializeJson(doc, server.arg("plain"));
  if(doc["octaves"]) { octaves = doc["octaves"].as<uint8_t>(); }
  if(doc["hue_octaves"]) { hue_octaves = doc["hue_octaves"].as<uint8_t>(); }
  if(doc["hue_speed"]) { hue_speed = doc["hue_speed"].as<uint8_t>(); }
  if(doc["time_speed"]) { time_speed = doc["time_speed"].as<uint16_t>(); }
  if(doc["xscale"]) { xscale = doc["xscale"].as<uint32_t>(); }
  if(doc["yscale"]) { yscale = doc["yscale"].as<uint32_t>(); }
  if(doc["hue_scale"]) { hue_scale = doc["hue_scale"].as<uint16_t>(); }
  if(doc["x_speed"]) { x_speed = doc["x_speed"].as<uint16_t>(); }
  if(doc["y_speed"]) { y_speed = doc["y_speed"].as<uint16_t>(); }
  ok();
}

void handle_ledUpon()
{

  if (direction != 3 && direction != 4)
  {
    direction = 0;
    server.send(200, "text/html", "ok");
  }

  ok();
}

void handle_ledDownon()
{
  if (direction != 0 && direction != 4)
  {
    direction = 3;
  }
  ok();
}

void handle_ledRighton()
{
  if (direction != 1 && direction != 4)
  {
    direction = 2;
    server.send(200, "text/html", "ok");
  }
  ok();
}

void handle_ledLefton()
{
  if (direction != 2 && direction != 4)
  {
    direction = 1;
    server.send(200, "text/html", "ok");
  }
  ok();
}

void handle_ledNetworkColoring()
{
  deserializeJson(doc, server.arg("plain"));
  auto xCords = doc["xCords"].as<uint8_t>();
  auto yCords = doc["yCords"].as<uint8_t>();
  auto r = doc["r"].as<uint8_t>();
  auto g = doc["g"].as<uint8_t>();
  auto b = doc["b"].as<uint8_t>();
  uint8_t n1 = cordsTransformation(xCords, yCords);
  leds[n1].r = r;
  leds[n1].g = g;
  leds[n1].b = b;
  FastLED.show();
  ok();
}

void handle_ledChangeMode()
{
  deserializeJson(doc, server.arg("plain"));
  auto modeType = doc["mode"].as<uint8_t>();
  if (modeType == 0)
  {
    type = 0;
    resetDisplay();
    initSnake();
    timing = millis();
  }
  else if (modeType == 1)
  {
    type = 1;
    resetDisplay();
    display();
    clearDisplay();
  }
  else
  {
    type = 2;
    resetDisplay();
    display();
    clearDisplay();
  }
  ok();
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html"; // If a folder is requested, send the index file
  String contentType = "";
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {                                                     // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                      // If there's a compressed version available
      path += ".gz";                                    // Use the compressed verion
    File file = SPIFFS.open(path, "r");                 // Open the file
    size_t sent = server.streamFile(file, contentType); // Send it to the client
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

void handleFileUpload()
{
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      fsUploadFile.close();
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      server.sendHeader("Location", "/");
      server.send(303);
    }
    else
    {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}
