#include <iostream>
#include <fstream>
#include <cassert>
#include <thread>
#include <Windows.h>
#include <mmsystem.h>
#include "midifile.h"
#include "midiplayer.h"
#include "kdmapisupport.h"

#ifdef USE_OPENGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gl_util.h"
#include "piano.h"
#endif

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
        uint64_t player_message_len;

        std::cout << "Header Chunk:" << std::endl;
        std::cout << "    Length: " << midi->header_len << std::endl;
        std::cout << "    Format: " << midi->format << std::endl;
        std::cout << "    Tracks: " << midi->track_count << std::endl;
        std::cout << "    Division (or ticks per quarter note): " << midi->division << std::endl;

        // set up MIDI output stuff
        if (!InitKDMApi())
            throw "Couldn't initialize KDMAPI!";

        // analyze midi...
        volatile bool analysis_done = false;
        std::thread analyze_thread(&MidiPlayer::Play, player, true, std::ref(analysis_done));
        std::cout << "Analyzing..." << std::endl;
        while (!analysis_done) {
#ifdef USE_OPENGL
            // TODO
#endif
        }
        player->start_time = 0;
        player->total_notes = player->played_notes;
        player->played_notes = 0;
        for (auto& track : player->loaded_file->tracks) {
            track.enabled = true;
            track.pos = 0;
        }

        // start playing!
#ifdef USE_OPENGL
        // midi player itself will handle rendering if opengl
        player->Play(false, std::ref(analysis_done));
#else
        std::thread midi_thread(&MidiPlayer::Play, player, false, std::ref(analysis_done));
        std::cout << "Played 0 notes | 0 n/s";
        player->playing = true;
#endif
#ifndef USE_OPENGL
         while (player->playing) {
            if (GetTickCount64() > last_note_check + 1000) {
                // slight race condition here but who cares
                notes_since_last_check = player->played_notes - notes_at_last_check;
                notes_at_last_check = player->played_notes;
                last_note_check = GetTickCount64();
            }
            if (player->message_pending) {
                // it's fine to overwrite the message buffer now because it'll be overwritten right after this anyway
                player_message_len = strnlen(player->message, 0x100);
                strncpy_s(message, player->message, player_message_len);
                if (previous_string_size > player_message_len) {
                    memset(message + player_message_len, ' ', previous_string_size - player_message_len);
                    message[previous_string_size + 1] = '\0';
                }
                printf("\r%s\n", message);
				player->message_pending = false;
            }
            // stringstreams are slow, let's do it c-style!
            // any unused space will be replaced with a space to prevent overlapping if the previous message was longer than the current
            // it only needs to be done once per above case
            snprintf(message, 0x100, "\rPlayed %llu/%llu notes | %llu n/s", player->played_notes, player->total_notes, notes_since_last_check);
            current_string_size = strnlen(message, 0xF8);
            if (previous_string_size > current_string_size) {
                memset(message + current_string_size, ' ', previous_string_size - current_string_size);
                message[previous_string_size + 1] = '\0';
            }
            previous_string_size = current_string_size;
            printf("%s", message);
        }

        // wait for the thread to exit
        midi_thread.join();
#endif

        delete midi;
        delete player;
    }
    catch (const std::exception &e) {
        std::cout << std::endl << "An exception occurred: " << e.what() << std::endl;
    }
    return 0;
}