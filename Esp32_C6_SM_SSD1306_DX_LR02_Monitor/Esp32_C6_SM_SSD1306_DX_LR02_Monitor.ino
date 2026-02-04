/*
  Simple Test sketch to connect an OLED SSD1306 display and a
  LoRa DX-LR02-900T22D to an ESP32-C6 Super Mini device.
  The display is controlled by the I2C interface, the LoRa module
  uses the Serial/UART interface.

  Important note: you need to make the settings for the LoRa module
  manually BEFORE you start to run this sketch, as the module is, e.g.,
  using a frequency band or transmission power that is not allowed
  in your country.

  For assistance, see my tutorial 'Simple LoRa transmissions over 
  several kilometers using UART/Serial LoRa modules', published here:
  https://medium.com/@androidcrypto/simple-lora-transmissions-over-several-kilometers-using-uart-serial-lora-modules-6c5abbf464f2

  Second: this sketch is transmitting a small, 4 bytes long packet every
  14 seconds - that could lead to a spamming of the channel in use.
  Please activate the transmission for a short period of time only, please !

  For transmission uncomment '#define ENABLE_TRANSMISSION true' below
*/

/*
Wiring OLED SSD1306 - ESP32-C6 SM
GND - GND
VCC - 3.3V
SCL - GPIO 19
SDA - GPIO 20
*/

// ------------------------------------------------------------------
// LoRa module
#define LORA_SERIAL Serial1 // using the second UART interface
#define RXD2_PIN 17 // Note: the TXD pin of the LoRa module is connected to the RXD2 pin of the ESP32-C6 SM
#define TXD2_PIN 16 // Note: the RXD pin of the LoRa module is connected to the TXD2 pin of the ESP32-C6 SM

/*
Wiring LoRa module - ESP32-C6 SM
M0  - not connected
M1  - not connected
RXD - GPIO 16 (labled with 'TX'): yes, the RXD pin of the LoRa module is connected to the TX pin of the ESP32-C6 SM
TXD - GPIO 17 (labled with 'RX'): yes, the TXD pin of the LoRa module is connected to the RX pin of the ESP32-C6 SM
AUX - not connected
VCC - 3.3 pin
GND - GND pin
*/

const uint8_t RX_BUFFER_SIZE = 255;
uint8_t rxBuffer[RX_BUFFER_SIZE];
int rxBufferLength = 0;
bool isReceived = false;

const uint8_t TX_BUFFER_SIZE = 255;
uint8_t txBuffer[TX_BUFFER_SIZE];
int txBufferLength = 0;

//#define ENABLE_TRANSMISSION true

const long TRANSMISSION_INTERVAL_MILLIS = 14000;  // 14 seconds
long lastTransmissionMillis = 0;

uint8_t txCounter = 0;

// ------------------------------------------------------------------
// internal OLED display SSD1306 128 * 64 px
#define IS_OLED  // uncomment this if not included
#define OLED_I2C_ADDRESS 0x3C
#define OLED_I2C_SDA_PIN 20
#define OLED_I2C_SCL_PIN 19

#include "FONT_MONOSPACE_9.h"
// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>
#include "SSD1306.h"  // https://github.com/ThingPulse/esp8266-oled-ssd1306
SSD1306Wire display(OLED_I2C_ADDRESS, OLED_I2C_SDA_PIN, OLED_I2C_SCL_PIN);

String display1 = "";
String display2 = "";
String display3 = "";
String display4 = "";
String display5 = "";

char buf[4];

// ------------------------------------------------------------------
// Display helper

void displayData() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Monospaced_plain_9);
  display.drawString(0, 0, display1);
  display.drawString(0, 12, display2);
  display.drawString(0, 24, display3);
  display.drawString(0, 36, display4);
  display.drawString(0, 48, display5);
  display.display();
}

// ------------------------------------------------------------------
// LoRa helper methods

void printRxBuffer(uint8_t length) {
  Serial.printf("rxBuffer (%2d): ", length);
  for (uint8_t i = 0; i < length; i++) {
    Serial.printf("%02X ", rxBuffer[i]);
  }
  Serial.println();
}

void printRxBufferChar(uint8_t length) {
  Serial.printf("rxBuffer (%2d): ", length);
  for (uint8_t i = 0; i < length; i++) {
    Serial.print((char)rxBuffer[i]);
  }
  Serial.println();
}

void printTxBuffer(uint8_t length) {
  Serial.printf("txBuffer (%2d): ", length);
  for (uint8_t i = 0; i < length; i++) {
    Serial.printf("%02X ", txBuffer[i]);
  }
  Serial.println();
}

// this is transmitting the data in txBuffer with dataLength
bool transmitData(uint8_t dataLength) {
  txBufferLength = dataLength;
  LORA_SERIAL.write(txBuffer, txBufferLength);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("ESP32-C6 Super Mini OLED SSD1306 display test"));

  display.init();
  //display.flipScreenVertically();

  // Just a test for the display and font I'm using
  display1 = "12345678901234567890123456";
  display1 = "ESP32-C6 Super Mini";
  display2 = "OLED Display SSD1306 I2C";
  display3 = "DX-SMART DX-LR02-900T22D";
  display4 = "LoRa module 863-915 MHz";
  display5 = "with UART interface";
  displayData();

  // Connect to LoRa device
  LORA_SERIAL.begin(9600, SERIAL_8N1, RXD2_PIN, TXD2_PIN);

  // set a timeout for receiving data
  // default is LORA_SERIAL timeout is 1000 ms
  const uint16_t RX_TIMEOUT_MILLIS = 10;
  LORA_SERIAL.setTimeout(RX_TIMEOUT_MILLIS);
  Serial.printf("LORA_SERIAL timeout is %d ms\n", LORA_SERIAL.getTimeout());
  delay(5000);

  lastTransmissionMillis = millis();
}

void loop() {

  // this is the receiver part
  // read messages from the device
  while (LORA_SERIAL.available() > 0) {
    rxBufferLength = LORA_SERIAL.readBytes(rxBuffer, sizeof(rxBuffer));
    if (rxBufferLength > 0) {
      isReceived = true;
    }
  }

  if (isReceived) {
    isReceived = false;
    // here is the place to work with the data, I'm just printing the received packet
    // HEX and UTF-8 encodings and displaying HEX encoding
    printRxBuffer(rxBufferLength);
    printRxBufferChar(rxBufferLength);
    display.clear();
    display.display();
    display.resetDisplay();
    // This is a very slow but simple way of displaying data, better use 'drawString()'
    display.println("<-- Received " + String(rxBufferLength) + " bytes");
    for (uint8_t i = 0; i < rxBufferLength; i++) {
      sprintf(buf, "%02X ", rxBuffer[i]);
      display.print(buf);
      if ((i + 1) % 8 == 0) display.println();
    }
    display.println();
  }

#ifdef ENABLE_TRANSMISSION
  // this is the transmission part
  if (millis() - lastTransmissionMillis > TRANSMISSION_INTERVAL_MILLIS) {
    txCounter++;

    // just four bytes
    const uint8_t txLength = 4;
    txBuffer[0] = txCounter;
    txBuffer[1] = 0x62;
    txBuffer[2] = 0x63;
    txBuffer[3] = 0x64;

    if (transmitData(txLength)) {
      printTxBuffer(txLength);
      display.clear();
      display.display();
      display.resetDisplay();
      display.println("--> Transmitted " + String(txLength) + " bytes");
      for (uint8_t i = 0; i < txLength; i++) {
        sprintf(buf, "%02X ", txBuffer[i]);
        display.print(buf);
        if ((i + 1) % 8 == 0) display.println();
      }
      display.println();
    }
    lastTransmissionMillis = millis();
  }
#endif  
}
