#include <Arduino.h>

#define PPM_PIN 2
#define NUM_PPM_CHANNELS 8
#define NUM_CRSF_CHANNELS 16

volatile uint32_t lastTime = 0;
volatile int currentChannel = 0;
volatile uint16_t ppmValues[NUM_CRSF_CHANNELS];

const uint8_t CRSF_SYNC_POS = 0;
const uint8_t CRSF_LEN_POS = 1;
const uint8_t CRSF_TYPE_POS = 2;
const uint8_t CRSF_PAYLOAD_POS = 3;
const uint8_t CRSF_CRC_POS = 25;

const uint8_t CRSF_SYNC = 0xEE;
const uint8_t CRSF_LEN = 0x18;
const uint8_t CRSF_TYPE = 0x16;

const uint8_t CRSF_PAYLOAD_SIZE = 22;
const uint8_t CRSF_FRAME_SIZE = 26;

const uint16_t CRSF_MIN_US = 988;
const uint16_t CRSF_MAX_US = 2012;
const uint16_t CRSF_MIN_VALUE = 172;
const uint16_t CRSF_MAX_VALUE = 1811;

const uint16_t PPM_NEUTRAL = 1500;

const uint8_t REFRESH_RATE = 70; // Hz

uint8_t crsfFrame[CRSF_FRAME_SIZE];

HardwareSerial CRSFSerial(1);

void ppmISR()
{
  uint32_t now = micros();
  uint32_t duration = now - lastTime;
  lastTime = now;

  if (duration > 4000)
  {
    currentChannel = 0;
  }
  else
  {
    if (currentChannel < NUM_PPM_CHANNELS)
    {
      ppmValues[currentChannel] = duration;
      currentChannel++;
    }
  }
}

void encodeChannels()
{
  uint16_t channels[NUM_CRSF_CHANNELS];
  for (int i = 0; i < NUM_CRSF_CHANNELS; i++) {
      uint16_t val = map(ppmValues[i], CRSF_MIN_US, CRSF_MAX_US,CRSF_MIN_VALUE, CRSF_MAX_VALUE);
      val = min(val, CRSF_MAX_VALUE);
      val = max(val, CRSF_MIN_VALUE);
      Serial.print(val);
      Serial.print(" ");
      channels[i] = val;
  }

  // clear payload
  memset(&crsfFrame[CRSF_PAYLOAD_POS], 0, CRSF_PAYLOAD_SIZE);

  int bitOffset = 0;
  for (int i = 0; i < NUM_CRSF_CHANNELS; i++)
  {
    uint16_t chan = channels[i] & 0x07FF; // 11 bits
    int byteIndex = bitOffset / 8;
    int bitIndex = bitOffset % 8;

    crsfFrame[CRSF_PAYLOAD_POS + byteIndex] |= (chan << bitIndex) & 0xFF;
    crsfFrame[CRSF_PAYLOAD_POS + byteIndex + 1] |= (chan >> (8 - bitIndex)) & 0xFF;

    if (bitIndex > 5)
    {
      crsfFrame[CRSF_PAYLOAD_POS + byteIndex + 2] |= (chan >> (16 - bitIndex)) & 0xFF;
    }
    bitOffset += 11;
  }
}

uint8_t crc8(const uint8_t *data, uint8_t length)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; i++)
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0xD5;
      else
        crc <<= 1;
    }
  }
  return crc;
}

void sendCRSFFrame()
{
  crsfFrame[CRSF_SYNC_POS] = CRSF_SYNC;
  crsfFrame[CRSF_LEN_POS] = CRSF_PAYLOAD_SIZE + 2;
  crsfFrame[CRSF_TYPE_POS] = CRSF_TYPE;
  encodeChannels();
  crsfFrame[CRSF_CRC_POS] = crc8(&crsfFrame[2], CRSF_PAYLOAD_SIZE + 1);

  Serial1.write(crsfFrame, CRSF_FRAME_SIZE);
}

unsigned long lastSend = 0;

void setup()
{
  for (int i = 0; i < NUM_CRSF_CHANNELS; ++i) {
    ppmValues[i] = PPM_NEUTRAL;
  }

  Serial1.begin(420000, SERIAL_8N1, RX, TX, true);
  pinMode(PPM_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PPM_PIN), ppmISR, RISING);
}

void loop()
{
  unsigned long now = millis();
  if (now - lastSend > 1000 / REFRESH_RATE)
  {
    sendCRSFFrame();
    lastSend = now;
  }
}