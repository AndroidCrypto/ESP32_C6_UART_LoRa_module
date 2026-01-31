#include "LoRaModule.h"

LoRaModule lora(2, 3);   // RX=2, TX=3

uint8_t ping[] = {'P','i','n','g'}; // Host transmission data
uint8_t pong[] = {'P','o','n','g'}; // Slave response data

void onTxDone()
{
  Serial.println("Pong sent successfully");
  lora.startReceive(1000);            // Start listening for host Ping
}

void onRxDone(uint8_t *d, uint16_t len)
{
  Serial.println("Ping received, PingPong successful");
  lora.send(pong, sizeof(pong));   // Slave sends Pong response
}

void onTimeout()
{
  Serial.println("PingPong timeout, restarting listening");
  lora.startReceive(1000);         // Start listening with 1s timeout
}

void setup()
{
  /* Initialize debug serial port */
  Serial.begin(9600);

  /* Register callback functions for transmission complete, reception complete, and timeout events */
  lora.onTxDone(onTxDone);
  lora.onRxDone(onRxDone);
  lora.onRxTimeout(onTimeout);

  /* Configure module to expect 'Ping' from host */
  lora.setExpect(ping, sizeof(ping));
  Serial.println("Slave initialized successfully, starting PingPong test!");
  lora.startReceive(1000);           // Start listening with 1s timeout
}

void loop()
{
  /* State machine polling */
  lora.poll();
}