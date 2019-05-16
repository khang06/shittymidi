#pragma once
#include <array>
#include <string>
#include <Windows.h>
#include <mmsystem.h>
#include "midifile.h"

/*
typedef struct {
    std::array<uint8_t, 128> note_velocities;
    std::array<int16_t, 128> controllers;
} midi_channel_t;
*/

class MidiPlayer {
public:
    MidiPlayer(MidiFile* file);
    void Play(bool analyzing = false);

    MidiFile* loaded_file = nullptr;
    HMIDIOUT midi_out_handle = NULL;
    //std::array<midi_channel_t, 16> channels;
    double tick_length = 0;
    uint32_t start_time = 0;
    uint64_t played_notes = 0;
    bool playing = false;
    // a better implementation would be to implement a queue with a std::vector, but that would cause race conditions
    bool message_pending = false;
    char message[0x100];
    bool analyzing = true;

private:
    void ProcessCommand(MidiTrack& track, double emulated_time);
    void NoteEvent(bool status, uint8_t note, uint8_t velocity, uint8_t channel);
    void SendMessageToConsole(const char* format, ...);
    void midiOutShortMsgWrapper(HMIDIOUT hmo, DWORD dwMsg);
    std::vector<uint32_t> translated_midi;
};