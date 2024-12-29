/*
  ESP32-CAM Image Analysis with OpenAI API and OLED Display

  This code captures an image using the ESP32-CAM module, processes it, 
  and sends it to OpenAI's GPT-4o API for analysis. The API's response is 
  displayed on an OLED screen. The code also provides audio feedback using 
  a buzzer and includes features like Wi-Fi connectivity, image encoding, 
  and scrolling text display.

  Tested with:
  - Arduino IDE version 2.3.2
  - ESP32 boards package version 3.0.0
  - Adafruit GFX library version 1.11.11
  - Adafruit SSD1306 library version 2.5.13
  - ArduinoJson library version 7.1.0
  - Base64 library (default version with ESP32 boards package)

  Make sure to install these libraries and configure your environment 
  as specified above before running the code.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include "esp_camera.h"
#include <Adafruit_GFX.h>  
#include <Wire.h>
#include <Adafruit_SSD1306.h> 
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "SSID";
const char* password = "PASS";

// OpenAI API key
const String apiKey = "YOUR API KEY";

// Question to be Asked about the image
String Question = "Just let me know only the number of the car";

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SCL 14
#define OLED_SDA 15
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin definitions for ESP32-CAM AI-Thinker module
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define BUTTON_PIN 13
#define BUZZER_PIN 2  // Buzzer connected to GPIO2

void displayCenteredText(const String& text, int textSize = 1) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);

  int maxLineLength = 16;  // Assuming 16 characters fit per line at textSize 1
  String lineBuffer = "";
  String wordBuffer = "";
  int16_t x1, y1;
  uint16_t textWidth, textHeight;

  // Calculate line height
  display.getTextBounds("A", 0, 0, &x1, &y1, &textWidth, &textHeight);
  int lineHeight = textHeight + 2;

  // Calculate the total number of lines needed
  int lineCount = 0;
  for (size_t i = 0; i <= text.length(); i++) {
    char c = text.charAt(i);
    if (c == ' ' || c == '\n' || c == '\0') {
      if (lineBuffer.length() + wordBuffer.length() > maxLineLength) {
        lineCount++;
        lineBuffer = wordBuffer;
      } else {
        lineBuffer += (lineBuffer.isEmpty() ? "" : " ") + wordBuffer;
      }
      wordBuffer = "";

      if (c == '\n') {
        lineCount++;
        lineBuffer = "";
      }
    } else {
      wordBuffer += c;
    }
  }
  if (!lineBuffer.isEmpty()) lineCount++;  // Count the last line

  // Calculate the vertical offset to center the block of text
  int totalTextHeight = lineCount * lineHeight;
  int yOffset = (SCREEN_HEIGHT - totalTextHeight) / 2;

  // Render the text line by line, vertically centered
  int yPos = yOffset;
  lineBuffer = "";
  wordBuffer = "";
  for (size_t i = 0; i <= text.length(); i++) {
    char c = text.charAt(i);
    if (c == ' ' || c == '\n' || c == '\0') {
      if (lineBuffer.length() + wordBuffer.length() > maxLineLength) {
        // Render the current line
        display.setCursor((SCREEN_WIDTH - lineBuffer.length() * textWidth) / 2, yPos);
        display.print(lineBuffer);
        yPos += lineHeight;
        lineBuffer = wordBuffer;
      } else {
        lineBuffer += (lineBuffer.isEmpty() ? "" : " ") + wordBuffer;
      }
      wordBuffer = "";

      if (c == '\n' || c == '\0') {
        display.setCursor((SCREEN_WIDTH - lineBuffer.length() * textWidth) / 2, yPos);
        display.print(lineBuffer);
        yPos += lineHeight;
        lineBuffer = "";
      }
    } else {
      wordBuffer += c;
    }
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);  // Set Buzzer pin as output

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }

  // Display the project title on power-on
  displayCenteredText("AI VISION Project\nby techiesms", 2);
  delay(3000);  // Hold the title screen for 3 seconds

  displayCenteredText("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  displayCenteredText("WiFi Connected!");
  delay(2000);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    displayCenteredText("Camera Init Failed");
    return;
  }

  displayCenteredText("Camera Initialized");
  delay(2000);

  displayCenteredText("Press button to capture");
}

// Remaining code remains unchanged...

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed! Capturing image...");
    displayCenteredText("Capturing...");
    captureAndAnalyzeImage();
    delay(1000);  // Small delay to debounce button press
  }
}
