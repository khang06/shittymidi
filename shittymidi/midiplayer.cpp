#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include "midiplayer.h"

MidiPlayer::MidiPlayer(MidiFile* file) {
    loaded_file = file;
    tick_length = 500 / file->division; // 500 ms per quarter note
}

void MidiPlayer::NoteEvent(bool status, uint8_t velocity, uint8_t note, uint8_t channel) {
    // status = 0 = note off
    // status = 1 = note on
    DWORD message;

    message = ((((((channel) & 0x0F | (status ? 9 : 8) << 4) & 0xFF) | ((note & 0xFF) << 8)) & 0xFFFF) | ((velocity & 0xFF) << 16));
    midiOutShortMsg(midi_out_handle, message);
}

// this entire function is basically just what TMIDI does, but very slimmed down
void MidiPlayer::ProcessCommand(MidiTrack& track) {
    uint8_t cmd = track.ReadByte();
    uint32_t len;
    DWORD bruh;

    // running mode (whatever that is...)
    if (cmd < 128) {
        track.SkipBytes(-1);
        if (!track.last_cmd_initialized)
            throw "Tried to use uninitialized last_cmd!";
        cmd = track.last_cmd;
    }
    else {
        track.last_cmd_initialized = true;
        track.last_cmd = cmd;
    }

    // meta-events, used for showing MIDI info and stuff
    // since this MIDI player isn't exactly fully featured, most of these will be ignored
    if (cmd == 0xFF) {
        cmd = track.ReadByte();
        len = track.ReadVlq();

        switch (cmd) {
            case 0x00: // Sequence Number
                track.SkipBytes(2);
                break;
            default:
                std::cout << "Unhandled Meta Event " << std::hex << (uint32_t)cmd << std::dec << std::endl;
            case 0x01: // Text Event
            case 0x02: // Copyright Notice
            case 0x03: // Sequence/Track Name
            case 0x04: // Instrument Name
            case 0x05: // Lyric
            case 0x06: // Marker
            case 0x07: // Cue Point
            case 0x7F: // Sequencer-specific Meta-event
                track.SkipBytes(len);
                break;
            case 0x20: // MIDI Channel Prefix
                track.SkipBytes(1);
                break;
            case 0x21: // MIDI Port
                track.SkipBytes(1);
                break;
            case 0x2F: // End of Track
                track.enabled = false;
                break;
            case 0x51: // Set Tempo (in microseconds per quarter note)
                tick_length = ((((uint32_t)track.ReadByte() << 16) + ((uint32_t)track.ReadByte() << 8) + ((uint32_t)track.ReadByte())) / loaded_file->division) / (double)1000.0;
                //std::cout << tick_length << std::endl;
                break;
            case 0x54: // SMPTE Offset
                track.SkipBytes(5);
                break;
            case 0x58: // Time Signature
                track.SkipBytes(4);
                break;
            case 0x59: // Key Signature
                track.SkipBytes(2);
                break;
        }
        //std::cout << "Meta Event " << std::hex << (uint32_t)cmd << std::dec << std::endl;
    }
    else {
        switch (cmd >> 4) {
            case 0x08: // Note Off
                NoteEvent(false, track.ReadByte(), track.ReadByte(), cmd & 0xF);
                break;
            case 0x09: // Note On
                NoteEvent(true, track.ReadByte(), track.ReadByte(), cmd & 0xF);
                played_notes++;
                break;
            case 0x0A: // Polyphonic Key Pressure (Aftertouch)
            case 0x0B: // Control Change
            case 0x0E: // Pitch Wheel
                bruh = ((cmd & 0xFF) | ((track.ReadByte() & 0xFF) << 8)) & 0xFFFF | ((track.ReadByte() & 0xFF) << 16);
                midiOutShortMsg(midi_out_handle, bruh);
                break;
            case 0x0C: // Program Change
            case 0x0D: // Channel after-touch 
                midiOutShortMsg(midi_out_handle, ((cmd & 0xFF) | ((track.ReadByte() & 0xFF) << 8)));
                break;
            case 0x0F: // System Message
                std::cout << "System Message is UNIMPLEMENTED!!!" << std::endl;
                std::cout << (cmd & 0x0F) << std::endl;
                // this command does such a wide range of things that doing anything here would still break something
                break;
            default:
                std::cout << "Unhandled MIDI event " << (cmd >> 4) << std::endl;
        }
        //std::cout << "MIDI Event " << std::hex << (uint32_t)(cmd >> 4) << std::dec << std::endl;
    }
}

// i have spent HOURS debugging the timing bullshit even though this is completely based on someone else's implementation
void MidiPlayer::Play() {
    if (!midi_out_handle)
        throw "MIDI Out not initialized!";

    double cur_time;
    double next_trigger;
    bool read_first_event = true;
    bool tracks_still_enabled = true;
    playing = true;
    while (true) {
        timeBeginPeriod(1);
        cur_time = timeGetTime();
        timeEndPeriod(1);
        next_trigger = cur_time + 1000;

        for (MidiTrack& track : loaded_file->tracks) {
            if (read_first_event) {
                track.cur_event_len = track.ReadVlq() * tick_length;
                track.cur_event_end = cur_time + track.cur_event_len;
            }
            if (!track.enabled)
                continue;
            while (cur_time >= track.cur_event_end) {
                ProcessCommand(track);
                if (!track.enabled)
                    break;
                track.cur_event_len = track.ReadVlq() * tick_length;
                track.cur_event_end += track.cur_event_len;
            }
            if (!track.enabled)
                continue;
            if (track.cur_event_end < next_trigger)
                next_trigger = track.cur_event_end;
        }
        read_first_event = false;
        //cur_time = GetTickCount64();
        timeBeginPeriod(1);
        cur_time = timeGetTime();
        timeEndPeriod(1);
        while (cur_time < next_trigger) {
            // who cares if value could be lost? nobody's going to have almost 25 days between MIDI events
            //std::cout << "Sleeping for " << (uint32_t)next_trigger - cur_time << std::endl;
            Sleep((uint32_t)next_trigger - cur_time);
            //cur_time = GetTickCount64();
            timeBeginPeriod(1);
            cur_time = timeGetTime();
            timeEndPeriod(1);
        }

        // check if all tracks got disabled
        tracks_still_enabled = false;
        for (MidiTrack& track : loaded_file->tracks) {
            if (track.enabled)
                tracks_still_enabled = true;
        }
        if (!tracks_still_enabled)
            break;
    }
    playing = false;
}