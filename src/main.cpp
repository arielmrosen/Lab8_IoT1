#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLE_UART.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_DC    16
#define OLED_RESET 17
#define OLED_CS    5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);
bool wasConnected = false;
int messageCount = 0;
// Libraries
// Global variables
// Functions

void showOnOLED(String title, String message){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println(message);

  display.display();
}


void setup(){
  //Initialize
  Serial.begin(115200);
  Serial.println("Program Started");

  //OLED code
  display.begin(SSD1306_SWITCHCAPVCC);
  Serial.println("OLED Started");
  showOnOLED("Lab8: BLE", "Internet of Things");

  //BLE code
  bleBegin("ESP32_Ariel");
  Serial.println("BLE Started");
  showOnOLED("BLE Ready", "Waiting");

// OLED code
  // BLE code
}

void loop(){
  //Connection Status
  if (bleIsConnected() == true && wasConnected == false){
  wasConnected = true;
  Serial.println("Phone Connected");
  showOnOLED("BLE Status", "Connected");
}
if (bleIsConnected() == false && wasConnected == true){
  wasConnected = false;
  Serial.println("Phone Disconnected");
  showOnOLED("BLE Status", "Disconnected");
}

  //Send Messages
  if (bleMessageAvailable() == true){
  String message = bleReadMessage();

  messageCount = messageCount + 1;

  Serial.print("Received: ");
  Serial.println(message);

  String title = "Message " + String(messageCount);
  showOnOLED(title, message);

  bleSend("Shown on OLED");

}
}
