#pragma once
#include <array>
#include <string>
#include <Windows.h>
#include <mmsystem.h>
#include "midifile.h"

#ifdef USE_OPENGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gl_util.h"
#include "piano.h"
#endif

/*
typedef struct {
    std::array<uint8_t, 128> note_velocities;
    std::array<int16_t, 128> controllers;
} midi_channel_t;
*/

class MidiPlayer {
public:
    MidiPlayer(MidiFile* file);
    ~MidiPlayer();
    void Play(bool analyzing, volatile bool& done);

    MidiFile* loaded_file = nullptr;
    HMIDIOUT midi_out_handle = NULL;
    //std::array<midi_channel_t, 16> channels;
    double tick_length = 0;
    uint32_t start_time = 0;
    uint64_t played_notes = 0;
    uint64_t total_notes = 0;
    bool playing = false;
    // a better implementation would be to implement a queue with a std::vector, but that would cause race conditions
    bool message_pending = false;
    char message[0x100];
    bool analyzing = true;

private:
    void ProcessCommand(MidiTrack& track, double emulated_time, int track_idx);
    void NoteEvent(bool status, uint8_t note, uint8_t velocity, uint8_t channel, int track);
    void SendMessageToConsole(const char* format, ...);
    void midiOutShortMsgWrapper(HMIDIOUT hmo, DWORD dwMsg);
    std::vector<uint32_t> translated_midi;
#ifdef USE_OPENGL
    GLFWwindow* window = nullptr;
    Piano piano;
#endif
};