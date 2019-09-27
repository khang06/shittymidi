#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <cstring>
#include "midifile.h"
#include "util.h"

#define CPP_STYLE_FILES

MidiFile::MidiFile(const std::string& input) {
#ifdef CPP_STYLE_FILES
    std::ifstream stream(input, std::ios::in | std::ios::binary);
#else
    FILE* file = fopen(input.c_str(), "rb");
#endif
    char header_magic[4];
    // will be used in the track load loop
    uint32_t cur_track = 0;
    char track_magic[4];
    uint32_t track_len;

#ifdef CPP_STYLE_FILES
    if (!stream.good())
        throw "Failed to open file!";
#else
    if (!file)
        throw "Failed to open file!";
#endif

    // read the main header
#ifdef CPP_STYLE_FILES
    stream.read(header_magic, 4);
#else
    fread(header_magic, 4, 1, file);
#endif
    if (memcmp(header_magic, "MThd", 4))
        throw "Doesn't start with MThd!";
#ifdef CPP_STYLE_FILES
    stream.read(reinterpret_cast<char*>(&header_len), 4);
    stream.read(reinterpret_cast<char*>(&format), 2);
    stream.read(reinterpret_cast<char*>(&track_count), 2);
    stream.read(reinterpret_cast<char*>(&division), 2);
#else
    fread(&header_len, 4, 1, file);
    fread(&format, 2, 1, file);
    fread(&track_count, 2, 1, file);
    fread(&division, 2, 1, file);
#endif

    // swap endianness of the main header
    // it's not like Windows can run on big endian
    header_len = SwapEndian<uint32_t>(header_len);
    format = SwapEndian<uint16_t>(format);
    track_count = SwapEndian<uint16_t>(track_count);
    division = SwapEndian<uint16_t>(division);

    // read tracks
    uint64_t pos;
#ifdef CPP_STYLE_FILES
    while (!stream.eof()) {
        stream.read(reinterpret_cast<char*>(&track_magic), 4);
        pos = stream.tellg();
#else
    while (!feof(file)) {
        fread(&track_magic, 4, 1, file);
        pos = _ftelli64(file);
#endif
        
        // awful workaround
        if (pos == UINT64_MAX)
            continue;
        // another awful workaround
        if (memcmp(track_magic, "MTrk", 4)) {
            std::cout << "There should be a track at 0x" << std::hex << pos - 4 << std::dec << "..." << std::endl;
            continue;
        }
        tracks.resize(tracks.size() + 1);
        auto& track = tracks.at(cur_track);
#ifdef CPP_STYLE_FILES
        stream.read(reinterpret_cast<char*>(&track_len), 4);
#else
        fread(&track_len, 4, 1, file);
#endif
        track.length = SwapEndian<uint32_t>(track_len);
        track.data = new uint8_t[track.length];
        track.offset = pos;
#ifdef CPP_STYLE_FILES
        stream.read((char*)track.data, track.length);
#else
        fread(track.data, track.length, 1, file);
#endif
        cur_track++;
        if (cur_track == track_count)
            break;
    }
}