#include <Arduino.h>

#define PPM_PIN 2 // Where the PPM Signal comes in
#define NUM_PPM_CHANNELS 8
#define NUM_CRSF_CHANNELS 16
#define INVERT_SIGNAL false // true for output to a JR-Module (ESP32 boards only), false when using an RX as TX

volatile uint16_t ppmValues[NUM_PPM_CHANNELS];

const uint8_t CRSF_SYNC_POS = 0;
const uint8_t CRSF_LEN_POS = 1;
const uint8_t CRSF_TYPE_POS = 2;
const uint8_t CRSF_PAYLOAD_POS = 3;
const uint8_t CRSF_CRC_POS = 25;

const uint8_t CRSF_PAYLOAD_SIZE = 22;
const uint8_t CRSF_FRAME_SIZE = CRSF_PAYLOAD_SIZE + 4;

const uint8_t CRSF_SYNC = 0xC8; // EdgeTX: 0xEE CRSF: 0xC8
const uint8_t CRSF_LEN = CRSF_PAYLOAD_SIZE + 2;
const uint8_t CRSF_TYPE = 0x16; // CRSF Frametype RC channels packed

const uint16_t CRSF_MIN_US = 988;
const uint16_t CRSF_MAX_US = 2012;
const uint16_t CRSF_MIN_VALUE = 172;
const uint16_t CRSF_MAX_VALUE = 1811;
const uint16_t CRSF_NEUTRAL = 992;
const uint16_t CRSF_REFRESH_RATE = 10; // ms
const uint16_t BLINK_RATE = 200; //ms

const uint16_t PPM_SYNC = 3000; // us
const uint16_t PPM_TIMEOUT = 500; // ms
const uint16_t PPM_NEUTRAL = 1500;

static uint8_t crsfFrame[CRSF_FRAME_SIZE];
static volatile long lastPPMPulse = 0;

void ppmISR() {
    static uint8_t currentChannel = 0;

    uint32_t now = micros();
    uint32_t duration = now - lastPPMPulse;
    lastPPMPulse = now;

    if (duration > PPM_SYNC) {
        currentChannel = 0;
    } else {
        if (currentChannel < NUM_PPM_CHANNELS) {
            ppmValues[currentChannel] = duration;
            currentChannel++;
        }
    }
}

void encodeChannels() {
    uint16_t channels[NUM_CRSF_CHANNELS];
    for (int i = 0; i < NUM_CRSF_CHANNELS; i++) {
        uint16_t val;
        if (i < NUM_PPM_CHANNELS)
            val = map(ppmValues[i], CRSF_MIN_US, CRSF_MAX_US, CRSF_MIN_VALUE, CRSF_MAX_VALUE);
        else
            val = CRSF_NEUTRAL;
        channels[i] = max(min(val, CRSF_MAX_VALUE), CRSF_MIN_VALUE); // limit crsf min..crsf max
    }

    // clear payload
    memset(&crsfFrame[CRSF_PAYLOAD_POS], 0, CRSF_PAYLOAD_SIZE);

    int bitOffset = 0;
    for (int i = 0; i < NUM_CRSF_CHANNELS; i++) {
        uint16_t chan = channels[i] & 0x07FF; // 11 bits
        int byteIndex = bitOffset / 8;
        int bitIndex = bitOffset % 8;

        crsfFrame[CRSF_PAYLOAD_POS + byteIndex] |= (chan << bitIndex) & 0xFF;
        crsfFrame[CRSF_PAYLOAD_POS + byteIndex + 1] |= (chan >> (8 - bitIndex)) & 0xFF;

        if (bitIndex > 5) {
            crsfFrame[CRSF_PAYLOAD_POS + byteIndex + 2] |= (chan >> (16 - bitIndex)) & 0xFF;
        }
        bitOffset += 11;
    }
}

uint8_t crc8(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0xD5;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void sendCRSFFrame() {
    crsfFrame[CRSF_SYNC_POS] = CRSF_SYNC;
    crsfFrame[CRSF_LEN_POS] = CRSF_PAYLOAD_SIZE + 2;
    crsfFrame[CRSF_TYPE_POS] = CRSF_TYPE;
    encodeChannels();
    crsfFrame[CRSF_CRC_POS] = crc8(&crsfFrame[CRSF_TYPE_POS], CRSF_PAYLOAD_SIZE + 1);
    CRSF_SERIAL.write(crsfFrame, CRSF_FRAME_SIZE);
}

void setup() {
    pinMode(LED_PORT, OUTPUT);
    for (int i = 0; i < NUM_PPM_CHANNELS; ++i) {
        ppmValues[i] = PPM_NEUTRAL;
    }

#ifdef ESP32
    CRSF_SERIAL.begin(CRSF_BAUD_RATE, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN, INVERT_SIGNAL);
#else
    #if INVERT_SIGNAL
        #error "INVERT_SIGNAL=true Not supported on that board."
    #endif
    CRSF_SERIAL.begin(CRSF_BAUD_RATE);
#endif
    pinMode(PPM_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PPM_PIN), ppmISR, RISING);
}

void loop() {
    static uint32_t lastSend = 0;
    static uint32_t lastBlink = 0;
    static bool blinkState = false;

    uint32_t now = millis();
    noInterrupts();
    uint32_t lastPPMMillis = lastPPMPulse / 1000;
    interrupts();
    uint32_t ppmAge = now - lastPPMMillis;
    if ((ppmAge < PPM_TIMEOUT) && (now - lastSend > CRSF_REFRESH_RATE)) {
        lastSend = now;
        sendCRSFFrame();
        if (now - lastBlink > BLINK_RATE) {
            lastBlink = now;
            digitalWrite(LED_PORT, blinkState ? HIGH : LOW);
            blinkState = !blinkState;
        }
    } // else timeout => send nothing leads to failsafe
}