#include "BitWriter.h"

BitWriter::BitWriter(uint8_t *buf, uint8_t bufSize) : buffer(buf), size(bufSize), bitPos(0) {
    memset(buffer, 0, size);
}

void BitWriter::writeBits(uint32_t value, uint8_t numBits) {
    value &= (1UL << numBits) - 1; // Mask numBits

    while (numBits > 0) {
        uint8_t byteIndex = bitPos / 8;
        if (byteIndex >= size)
            return; // No write behind buffer

        uint8_t bitIndex = bitPos % 8;
        uint8_t bitsAvailable = 8 - bitIndex;
        uint8_t bitsToWrite = (numBits < bitsAvailable) ? numBits : bitsAvailable;

        uint8_t chunk = value & ((1UL << bitsToWrite) - 1);
        buffer[byteIndex] |= chunk << bitIndex;

        value >>= bitsToWrite;
        numBits -= bitsToWrite;
        bitPos += bitsToWrite;
    }
}
