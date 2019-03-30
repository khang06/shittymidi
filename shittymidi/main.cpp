#include <iostream>
#include <fstream>
#include <cassert>
#include <thread>
#include <Windows.h>
#include <mmsystem.h>
#include "midifile.h"
#include "midiplayer.h"

void GetMidiOutDevices(std::vector<std::wstring>& vector) {
    MIDIOUTCAPS device_caps;
    MMRESULT getdevcaps_status;

    uint32_t midi_out_num = midiOutGetNumDevs();
    for (uint32_t i = 0; i < midi_out_num; i++) {
        getdevcaps_status = midiOutGetDevCaps(i, &device_caps, sizeof(MIDIOUTCAPS));
        if (getdevcaps_status != MMSYSERR_NOERROR)
            continue;
        std::wstring device_name(device_caps.szPname);
        vector.push_back(device_name);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " [input midi]" << std::endl;
        return 1;
    }
    try {
        std::cout << "Loading " << argv[1] << "..." << std::endl;
        MidiFile* midi = new MidiFile(argv[1]);
        MidiPlayer* player = new MidiPlayer(midi);
        uint32_t chosen_device = 0;
        uint64_t last_note_check = 0;
        uint64_t notes_at_last_check = 0;
        uint64_t notes_since_last_check = 0;
        size_t previous_string_size = 0;
        size_t current_string_size = 0;
        char message[0x100]; // overkill?

        std::cout << "Header Chunk:" << std::endl;
        std::cout << "    Length: " << midi->header_len << std::endl;
        std::cout << "    Format: " << midi->format << std::endl;
        std::cout << "    Tracks: " << midi->track_count << std::endl;
        std::cout << "    Division (or ticks per quarter note): " << midi->division << std::endl;

        // cool C++ stuff that i'm only using to look cool and can very well be a regular for loop
        uint32_t i = 0;
        for (const MidiTrack& track : midi->tracks) {
            std::cout << "Track " << i << ":" << std::endl;
            std::cout << "    Offset: 0x" << std::hex << track.offset << std::dec << std::endl;
            std::cout << "    Length: " << track.length << std::endl;
            i++;
        }
        
        // set up MIDI output stuff
        std::vector<std::wstring> midi_out_devices;
        GetMidiOutDevices(midi_out_devices);
        i = 0;
        for (std::wstring& device_name : midi_out_devices) {
            std::wcout << "MIDI out device " << i << ": " << device_name << std::endl;
            i++;
        }
        std::cout << "Type the number of the MIDI out device you want." << std::endl;
        std::cin >> chosen_device;
        if (chosen_device > i)
            throw "Invalid device!";
        std::wcout << "Opening " << midi_out_devices.at(chosen_device) << "..." << std::endl;
        if (midiOutOpen(&player->midi_out_handle, chosen_device, NULL, 0, NULL) != MMSYSERR_NOERROR)
            throw "Failed to open device!";

        // start playing!
        std::thread midi_thread(&MidiPlayer::Play, player);
        std::cout << "Played 0 notes | 0 n/s";
        while (player->playing) {
            if (GetTickCount64() > last_note_check + 1000) {
                // slight race condition here but who cares
                notes_since_last_check = player->played_notes - notes_at_last_check;
                notes_at_last_check = player->played_notes;
                last_note_check = GetTickCount64();
            }
            // stringstreams are slow, let's do it c-style!
            // any unused space will be replaced with a space to prevent overlapping if the previous message was longer than the current
            // it only needs to be done once per above case
            snprintf(message, 0x100, "\rPlayed %llu notes | %llu n/s", player->played_notes, notes_since_last_check);
            current_string_size = strnlen(message, 0x100);
            if (previous_string_size > current_string_size) {
                memset(message + current_string_size, ' ', previous_string_size - current_string_size);
                message[previous_string_size + 1] = '\0';
            }
            previous_string_size = current_string_size;
            printf("%s", message);
        }

        // wait for the thread to exit
        midi_thread.join();

        delete midi;
        delete player;
    }
    catch (const std::exception &e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}