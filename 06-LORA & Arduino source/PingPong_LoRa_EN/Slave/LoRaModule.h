#ifndef LR31_MODULE_H
#define LR31_MODULE_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class LoRaModule
{
public:
  using TxDoneCallback    = void (*)();               // Transmission complete callback
  using RxDoneCallback    = void (*)(uint8_t*,uint16_t); // Reception complete callback
  using RxTimeoutCallback = void (*)();               // Reception timeout callback

  enum State : uint8_t { ST_IDLE, ST_RX, ST_RX_TIMEOUT };

  /* Constructor: rx/tx pins, default baud rate 9600 */
  LoRaModule(uint8_t rxPin, uint8_t txPin, uint32_t baud = 9600);

  /* The only function that needs to be called periodically */
  void poll();

  /* Send arbitrary data and enter wait-for-response state */
  void send(const uint8_t *buf, uint16_t len);

  /* Pure reception start: no transmission, only wait for expected frame, timeout in ms */
  void startReceive(uint32_t timeout = 0);

  /* Set "expected frame to receive", length=0 means any frame is considered success */
  void setExpect(const uint8_t *data, uint16_t len);

  /* Register callbacks */
  void onTxDone   (TxDoneCallback    cb) { _txDone    = cb; }
  void onRxDone   (RxDoneCallback    cb) { _rxDone    = cb; }
  void onRxTimeout(RxTimeoutCallback cb) { _rxTimeout = cb; }

  State state() const { return _state; }

private:
  SoftwareSerial _ss;

  TxDoneCallback    _txDone    = nullptr;
  RxDoneCallback    _rxDone    = nullptr;
  RxTimeoutCallback _rxTimeout = nullptr;

  /* State machine */
  State _state = ST_IDLE;
  uint32_t _ackDeadline = 0;

  /* Expected frame buffer */
  static constexpr uint8_t EXPECT_BUF_SZ = 32;
  uint8_t  _expect[EXPECT_BUF_SZ];
  uint16_t _expectLen = 0;          // 0 = any frame

  /* Ring receive buffer */
  static constexpr uint16_t RB_SIZE = 64;
  uint8_t  _rb[RB_SIZE];
  uint16_t _rbHead = 0, _rbTail = 0;

  /* Buffer operations */
  void _rbWrite(uint8_t b);
  void _rbClear();
  uint16_t _rbDataLen() const;
  bool _matchExpectInRb();   // Match expected frame
  void _changeState(State s);
};

#endif