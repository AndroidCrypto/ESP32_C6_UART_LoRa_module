#include "LoRaModule.h"  // Include LoRa module library

LoRaModule lora(2, 3);   // Initialize LoRa module with RX pin=2, TX pin=3

uint8_t ping[] = {'P','i','n','g'}; // Host transmission data
uint8_t pong[] = {'P','o','n','g'}; // Slave response data

// Callback function when transmission completes
void onTxDone()
{
  Serial.println("Ping sent successfully");
  lora.startReceive(1000);            // Start listening with 1s timeout
}

// Callback function when expected data is received
void onRxDone(uint8_t *d, uint16_t len)
{
  Serial.println("Pong received, PingPong successful");
  lora.send(ping, sizeof(ping));   // Retransmit
}

// Callback function when reception times out
void onTimeout()
{
  Serial.println("PingPong timeout, Retransmitting");
  lora.send(ping, sizeof(ping));   // Retransmit
} 

// Initialization function
void setup()
{
  /* Initialize debug serial port */
  Serial.begin(9600);

  /* Register callback functions for transmission complete, reception complete, and timeout events */
  lora.onTxDone(onTxDone);
  lora.onRxDone(onRxDone);
  lora.onRxTimeout(onTimeout);

  /* Configure module to expect 'Pong' response */
  lora.setExpect(pong, sizeof(pong));
  Serial.println("Host initialized successfully, starting PingPong test!");
  lora.send(ping, sizeof(ping));   // Send initial Ping
}

// Main program loop
void loop()
{
  /* State machine polling */
  lora.poll();
}