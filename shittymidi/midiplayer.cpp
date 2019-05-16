#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cstdarg>
#include <iomanip>
#include <chrono>
#include "midiplayer.h"

MidiPlayer::MidiPlayer(MidiFile* file) {
    loaded_file = file;
    tick_length = 500 / file->division; // 500 ms per quarter note
}

// this exists so the read functions still get executed
void MidiPlayer::midiOutShortMsgWrapper(HMIDIOUT hmo, DWORD dwMsg) {
    if (!analyzing)
        midiOutShortMsg(hmo, dwMsg);
}

// main thread will handle the rest, this is to prevent interfering with note counter
void MidiPlayer::SendMessageToConsole(const char* format, ...) {
    // no queue, so the message won't be sent if one's pending ¯\_(ツ)_/¯
    if (message_pending)
        return;
    va_list args;
    va_start(args, format);
    vsnprintf(message, 0x100, format, args);
	OutputDebugStringA(message);
    //message_pending = true;
}

void MidiPlayer::NoteEvent(bool status, uint8_t note, uint8_t velocity, uint8_t channel) {
    // status = 0 = note off
    // status = 1 = note on
    DWORD message;

    message = ((((((channel) & 0x0F | (status ? 9 : 8) << 4) & 0xFF) | ((velocity & 0xFF) << 8)) & 0xFFFF) | ((note & 0xFF) << 16));
    midiOutShortMsgWrapper(midi_out_handle, message);
}

// this entire function is basically just what TMIDI does, but very slimmed down
void MidiPlayer::ProcessCommand(MidiTrack& track, double emulated_time) {
    uint8_t cmd = track.ReadByte();
    uint32_t len;
    double old_tick_length;
    double cur_time = emulated_time;

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
                old_tick_length = tick_length;
                tick_length = ((((uint32_t)track.ReadByte() << 16) + ((uint32_t)track.ReadByte() << 8) + ((uint32_t)track.ReadByte())) / loaded_file->division) / (double)1000.0;
                if (!analyzing) {
                    timeBeginPeriod(1);
                    cur_time = timeGetTime();
                    timeEndPeriod(1);
                }
                cur_time += 0.000001; // timing hack for Septette for the Dead Princess 14.9 million.mid, if this isn't in place, the tempo only gets set once and the midi plays too slowly
                for (MidiTrack& track : loaded_file->tracks) {
                    //if (!track.enabled)
                    //    continue;
                    track.cur_event_len = ((track.cur_event_end - cur_time) / old_tick_length) * tick_length;
                    track.cur_event_end = cur_time + track.cur_event_len;
                }
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
            default:
                //std::cout << "Unhandled Meta Event " << std::hex << (uint32_t)cmd << std::dec << std::endl;
                SendMessageToConsole("Unhandled Meta Event 0x%X", cmd);
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
                midiOutShortMsgWrapper(midi_out_handle, ((cmd & 0xFF) | ((track.ReadByte() & 0xFF) << 8)) & 0xFFFF | ((track.ReadByte() & 0xFF) << 16));
                break;
            case 0x0C: // Program Change
            case 0x0D: // Channel after-touch 
                midiOutShortMsgWrapper(midi_out_handle, ((cmd & 0xFF) | ((track.ReadByte() & 0xFF) << 8)));
                break;
            case 0x0F: // System Message
                switch (cmd & 0x0F) {
                    case 0x00: // System Exclusive
                        track.SkipBytes(track.ReadVlq());
                        break;
                    case 0x01: // MIDI Time Code Quarter Frame
                    case 0x03: // Song Select
                        track.SkipBytes(1);
                        break;
                    case 0x02: // Song Position Pointer
                        track.SkipBytes(2);
                        break;
                    case 0x06: // Tune Request
                    case 0x07: // End Of Exclusive
                    case 0x08: // Timing Clock
                    case 0x0A: // Start
                    case 0x0B: // Continue
                    case 0x0C: // Stop
                    case 0x0E: // Active Sensing
                    case 0x0F: // Reset
                        // these are probably safe to ignore, so nothing happens here
                        break;
                    default:
                        //std::cout << "Unhandled System Event " << std::hex << (cmd & 0xF) << std::dec << std::endl;
                        SendMessageToConsole("Unhandled System Event 0x%X", cmd & 0x0F);
                }
                break;
            default:
                //std::cout << "Unhandled MIDI event " << std::hex << (cmd >> 4) << std::dec << std::endl;
                SendMessageToConsole("Unhandled MIDI Event 0x%X", cmd >> 4);
        }
        //std::cout << "MIDI Event " << std::hex << (uint32_t)(cmd >> 4) << std::dec << std::endl;
    }
}

void MidiPlayer::Play(bool analyzing_param) {
    if (analyzing_param)
        analyzing = true;
    if (!analyzing) {
        if (!midi_out_handle)
            throw "MIDI Out not initialized!";
    }

    double cur_time = 0;
    double next_trigger;
    bool read_first_event = true;
    bool tracks_still_enabled = true;
    playing = true;
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        if (!analyzing) {
            timeBeginPeriod(1);
            cur_time = timeGetTime();
            timeEndPeriod(1);
        }
        next_trigger = cur_time + 1000;
        int track_num = 0;
        for (MidiTrack& track : loaded_file->tracks) {
            if (!track.enabled)
                continue;
            if (read_first_event) {
                track.cur_event_len = track.ReadVlq() * tick_length;
                track.cur_event_end = cur_time + track.cur_event_len;
            }
            while (cur_time >= track.cur_event_end) {
                ProcessCommand(track, cur_time);
                if (!track.enabled)
                    break;
                track.cur_event_len = track.ReadVlq() * tick_length;
                track.cur_event_end += track.cur_event_len;
                //std::cout << "event on track " << track_num << " for " << std::fixed << std::setprecision(6) << track.cur_event_len << " " << track.cur_event_end << " " << cur_time << std::endl;
                //std::cout << "event on track " << track_num << " for " << std::fixed << std::setprecision(6) << track.cur_event_len << std::endl;
            }
            if (!track.enabled)
                continue;
            
            if (track.cur_event_end < next_trigger) {
                next_trigger = track.cur_event_end;
            }
            track_num++;
        }
        read_first_event = false;
        //cur_time = GetTickCount64();
        
        if (!analyzing) {
            while (cur_time < next_trigger) {
                // who cares if value could be lost? nobody's going to have almost 25 days between MIDI events
                //std::cout << "Sleeping for " << (uint32_t)next_trigger - cur_time << std::endl;
                Sleep(static_cast<uint32_t>(next_trigger) - cur_time);
                //cur_time = GetTickCount64();
                timeBeginPeriod(1);
                cur_time = timeGetTime();
                timeEndPeriod(1);
            }
        }
        else { // time will be simulated if analyzing
            if (cur_time < next_trigger) {
                cur_time = next_trigger;
            }
        }
        

        // check if all tracks got disabled
        tracks_still_enabled = false;
        for (MidiTrack& track : loaded_file->tracks) {
            if (track.enabled) {
                tracks_still_enabled = true;
                break;
            }
        }
        if (!tracks_still_enabled)
            break;
    }
    playing = false;
    auto end = std::chrono::high_resolution_clock::now();
    if (analyzing) {
        std::cout << "Analyzed MIDI in " << ((double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000000 << " seconds" << std::endl;
        analyzing = false;
    }
}