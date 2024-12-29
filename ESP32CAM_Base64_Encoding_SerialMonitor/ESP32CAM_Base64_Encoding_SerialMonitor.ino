/*
   This code demonstrates how to use the ESP32-CAM module to capture an image, encode it in Base64 format, 
   and send the Base64 string to the Serial Monitor. The code is designed to work with the ESP32-CAM AI-Thinker 
   module and includes the necessary configurations and pin definitions for this specific hardware.

   Key Functionalities:
   1. Initialize the ESP32-CAM hardware and configure its pins for camera operation.
   2. Capture an image using the camera.
   3. Encode the captured image into a Base64 string for easy transfer or storage.
   4. Print the Base64-encoded image string to the Serial Monitor.

   Components:
   - ESP32-CAM AI-Thinker module
   - Arduino IDE or compatible environment
   
   Version Check:
   - ESP32 Boards Package Version - 3.0.0
   - Arduino IDE Version - 2.3.2
   Note:
   - Ensure the ESP32-CAM module is properly powered, as it requires sufficient current to operate the camera.
   - The Base64 string output can be used for web-based applications or for transmitting the image over networks.
*/



#include <WiFi.h>         // Include WiFi library for ESP32
#include <Base64.h>       // For Base64 encoding
#include "esp_camera.h"   // Include ESP32 camera library

// Pin definitions for the ESP32-CAM AI-Thinker module
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

void setup() {
  Serial.begin(115200);

  // Initialize camera
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
  config.frame_size = FRAMESIZE_VGA;  // or FRAMESIZE_VGA
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Camera init
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  // Capture and encode the image to Base64
  captureAndPrintBase64Image();
}

void captureAndPrintBase64Image() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Encode the image buffer to Base64
  String base64Image = base64::encode((const uint8_t*)fb->buf, fb->len);

  // Print the Base64 string to the Serial Monitor
  Serial.println("\n[Base64 Encoded Image]:");
  Serial.println(base64Image);

  esp_camera_fb_return(fb);  // Return the frame buffer
}

void loop() {
  // Nothing to loop in this example
}