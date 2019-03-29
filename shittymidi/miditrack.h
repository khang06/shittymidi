#pragma once
#include <cstdint>

class MidiTrack {
public:
    uint8_t ReadByte();
    uint32_t Read32Bits();
    void SkipBytes(int32_t bytes);
    uint32_t ReadVlq();

    uint32_t length;
    uint8_t* data;
    uint32_t pos;
    bool last_cmd_initialized;
    uint8_t last_cmd; // used for running mode, i honestly have no idea how it works
    bool enabled = true;
    double cur_event_len;
    double cur_event_end;
    uint64_t offset;
};