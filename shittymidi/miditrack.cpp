#include <cstdint>
#include <iostream>
#include "miditrack.h"
#include "util.h"

uint8_t MidiTrack::ReadByte() {
    if (pos + 1 > length)
        throw "Tried to read out of bounds!";
    pos++;
    return data[pos - 1];
}

uint32_t MidiTrack::Read32Bits() {
    return ReadByte() << 24 | ReadByte() << 16 | ReadByte() << 8 | ReadByte();
    //return ReadByte() | ReadByte() << 8 | ReadByte() << 16 | ReadByte() << 24;
}

void MidiTrack::SkipBytes(int32_t bytes) {
    if (pos + bytes > length || pos + bytes < 0)
        throw "Tried to skip out of bounds!";
    pos = pos + bytes;
}

/*
    VLQ = Variable Length Quantity
    A VLQ variable is made of 7-bit values.
    The 8th bit of every value is set to 1,
    except for the last value, where it's set to 0.
    0x0FFFFFFF is the largest allowed VLQ.
    For example, 0x69420 is 00000110 10010100 00100000 in binary.
    The VLQ representation would look something like 10000110 11001010 00010000.
*/
// this implementation is from python-midi
uint32_t MidiTrack::ReadVlq() {
    uint32_t value = 0;
    uint8_t temp;
    bool read_next_byte = true;

    while (read_next_byte) {
        temp = ReadByte();
        if (!(temp & 0x80))
            read_next_byte = false;
        temp &= 0x7F;
        value <<= 7;
        value += temp;
    }
    return value;
}