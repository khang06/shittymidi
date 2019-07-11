#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <cstring>
#include "midifile.h"
#include "util.h"

MidiFile::MidiFile(const std::string& input) {
    std::ifstream stream(input, std::ios::in | std::ios::binary);
    char header_magic[4];
    // will be used in the track load loop
    uint32_t cur_track = 0;
    char track_magic[4];
    uint32_t track_len;

    if (!stream.good())
        throw "Failed to open file!";

    // read the main header
    stream.read(header_magic, 4);
    if (memcmp(header_magic, "MThd", 4))
        throw "Doesn't start with MThd!";
    stream.read(reinterpret_cast<char*>(&header_len), 4);
    stream.read(reinterpret_cast<char*>(&format), 2);
    stream.read(reinterpret_cast<char*>(&track_count), 2);
    stream.read(reinterpret_cast<char*>(&division), 2);

    // swap endianness of the main header
    // it's not like Windows can run on big endian
    header_len = SwapEndian<uint32_t>(header_len);
    format = SwapEndian<uint16_t>(format);
    track_count = SwapEndian<uint16_t>(track_count);
    division = SwapEndian<uint16_t>(division);

    // read tracks
    uint64_t pos;
    while (!stream.eof()) {
        stream.read(reinterpret_cast<char*>(&track_magic), 4);
        pos = stream.tellg();
        // awful workaround
        if (pos == UINT64_MAX)
            continue;
        if (memcmp(track_magic, "MTrk", 4)) {
            std::cout << "There should be a track at 0x" << std::hex << pos - 4 << std::dec << "..." << std::endl;
            continue;
        }
        tracks.resize(tracks.size() + 1);
        auto& track = tracks.at(cur_track);
        stream.read(reinterpret_cast<char*>(&track_len), 4);
        track.length = SwapEndian<uint32_t>(track_len);
        track.data = new uint8_t[track.length];
        track.offset = pos;
        stream.read((char*)track.data, track.length);
        cur_track++;
    }
}