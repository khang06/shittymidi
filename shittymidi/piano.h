#pragma once

#include <array>
#include <list>
#include <set>
#include <vector>

typedef struct {
    int color_index;
    int note;
} draw_queue_entry_t;

class Piano {
public:
    void LoadShaders();
    void LoadKeyboardData();
    int AddToColorTable(const float* color);
    void NoteOn(int track, int note);
    void NoteOff(int track, int note);
    void Draw();
    bool IsSharp(int note) const;
private:
    // opengl-related stuff
    unsigned int program;
    unsigned int color_table_ubo;
    unsigned int note_state_ubo;
    int* mapped_note_state_ubo = nullptr;
    unsigned int vao;
    std::vector<int> indices;
    GLsync fence = nullptr;
    std::vector<draw_queue_entry_t> draw_queue;

    // stuff that gets initialized once and then never changed
    std::array<int, 128> sharp_table;

    // misc
    int color_table_pos = 0;
    std::array<std::set<int>, 128> cur_active_tracks; // sorted list of notes currently activated at a specific note
};

int limit_note(int note);