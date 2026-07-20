#include <Arduino.h>

#ifndef BIT_WRITER_H
    #define BIT_WRITER_H

class BitWriter {
    private:
        uint8_t *buffer;
        uint8_t size;
        uint16_t bitPos;

    public:
        /** Constructor */
        BitWriter(uint8_t *buf, uint8_t bufSize);

        /** Write numBits of value, LSB first to the buffer */
        void writeBits(uint32_t value, uint8_t numBits);
};

#endif