#pragma once
#include <string>
#include <vector>
#include "miditrack.h"

class MidiFile {
public:
    MidiFile(const std::string& input);

    uint32_t header_len;
    uint16_t format;
    uint16_t track_count;
    uint16_t division;
    std::vector<MidiTrack> tracks;
};