#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <list>
#include "piano.h"
#include "shaders.h"

typedef uint32_t u32;

int limit_note(int note) {
    return std::max(std::min(note, 127), 0);
}


template <typename T>
void push_vec3(std::vector<T>& vec, T x, T y, T z) {
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
}

void Piano::LoadShaders() {
    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &kbd_vert_shader_src, NULL);
    glCompileShader(vert_shader);
    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &kbd_frag_shader_src, NULL);
    glCompileShader(frag_shader);
    program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glUseProgram(program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

void Piano::LoadKeyboardData() {
    // 10 full octaves from -1 to 8, 11th octave only goes to G
    // 75 white keys, 53 sharp keys
    // generate white keys, then sharps
    constexpr float white_key_step = 2.0 / 75.0;
    constexpr int white_key_to_note[] = { 0, 2, 4, 5, 7, 9, 11 };
    constexpr int sharp_key_to_note[] = { 1, 3, 0, 6, 8, 10 };

    std::vector<float> vertices;
    std::vector<int> key_indices;

    push_vec3<float>(vertices, -1.0, -0.7, 1.0);
    key_indices.push_back(0);
    push_vec3<float>(vertices, -1.0, -1.0, 1.0);
    key_indices.push_back(0);
    for (int octave = 0; octave < 10; octave++) {
        float octave_start = (octave * 7) * white_key_step;
        for (int white_key = 0; white_key < 7; white_key++) {
            int white_key_total = (octave * 7) + white_key + 1;
            int note = (octave * 12) + white_key_to_note[white_key];
            push_vec3<float>(vertices, -1.0 + (white_key_step * white_key_total), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, -1.0 + (white_key_step * white_key_total), -1.0, 1.0);
            key_indices.push_back(note);
            push_vec3<int>(indices, (white_key_total - 1) * 2, ((white_key_total - 1) * 2) + 1, white_key_total * 2);
            push_vec3<int>(indices, ((white_key_total - 1) * 2) + 1, white_key_total * 2, (white_key_total * 2) + 1);
            sharp_table[note] = false;
        }
    }
    // generate the last octave for white keys
    {
        float octave_start = 70 * white_key_step;
        for (int white_key = 0; white_key < 6; white_key++) {
            int white_key_total = 71 + white_key;
            int note = limit_note(120 + white_key_to_note[white_key]);
            push_vec3<float>(vertices, -1.0 + (white_key_step * (white_key_total)), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, -1.0 + (white_key_step * (white_key_total)), -1.0, 1.0);
            key_indices.push_back(note);
            push_vec3<int>(indices, (white_key_total - 1) * 2, ((white_key_total - 1) * 2) + 1, white_key_total * 2);
            push_vec3<int>(indices, ((white_key_total - 1) * 2) + 1, white_key_total * 2, (white_key_total * 2) + 1);
            sharp_table[note] = false;
        }
    }
    // then sharp notes
    for (int octave = 0; octave < 10; octave++) {
        float octave_start = -1.0 + (octave * 7) * white_key_step;
        for (int sharp_key = 0; sharp_key < 6; sharp_key++) {
            if (sharp_key == 2)
                continue;
            int note = (octave * 12) + sharp_key_to_note[sharp_key];
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 3) + (white_key_step * sharp_key), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 3) + (white_key_step * sharp_key), -0.85, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 5) + (white_key_step * sharp_key), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 5) + (white_key_step * sharp_key), -0.85, 1.0);
            key_indices.push_back(note);
            int cur_index = vertices.size() / 3;
            push_vec3<int>(indices, cur_index - 4, cur_index - 3, cur_index - 2);
            push_vec3<int>(indices, cur_index - 3, cur_index - 2, cur_index - 1);
            sharp_table[note] = true;
        }
    }
    // generate the last octave for sharp keys
    {
        float octave_start = -1.0 + (10 * 7) * white_key_step;
        for (int sharp_key = 0; sharp_key < 4; sharp_key++) {
            if (sharp_key == 2)
                continue;
            int note = 120 + sharp_key_to_note[sharp_key];
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 3) + (white_key_step * sharp_key), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 3) + (white_key_step * sharp_key), -0.85, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 5) + (white_key_step * sharp_key), -0.7, 1.0);
            key_indices.push_back(note);
            push_vec3<float>(vertices, octave_start + ((white_key_step / 4) * 5) + (white_key_step * sharp_key), -0.85, 1.0);
            key_indices.push_back(note);
            int cur_index = vertices.size() / 3;
            push_vec3<int>(indices, cur_index - 4, cur_index - 3, cur_index - 2);
            push_vec3<int>(indices, cur_index - 3, cur_index - 2, cur_index - 1);
            sharp_table[note] = true;
        }
    }

    // merge vertex data and note map
    std::vector<u32> array_buffer;
    for (int i = 0; i < vertices.size(); i += 3) {
        array_buffer.push_back(reinterpret_cast<u32&>(vertices[i]));
        array_buffer.push_back(reinterpret_cast<u32&>(vertices[i + 1]));
        array_buffer.push_back(reinterpret_cast<u32&>(vertices[i + 2]));
        array_buffer.push_back(key_indices[i / 3]);
    }

    // upload keyboard data to gpu
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, array_buffer.size() * sizeof(u32), array_buffer.data(), GL_STATIC_DRAW);
    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(u32), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 1, GL_INT, 4 * sizeof(u32), (void*)(3 * sizeof(u32)));
    glEnableVertexAttribArray(1);

    // create ubos for note color table and note states
    glGenBuffers(1, &color_table_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, color_table_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2048 * 4 * 4, NULL, GL_STATIC_DRAW);
    constexpr float initial_color_table_white[] = { 1.0, 1.0, 1.0, 1.0 };
    constexpr float initial_color_table_black[] = { 0.0, 0.0, 0.0, 1.0 };
    AddToColorTable(initial_color_table_white);
    AddToColorTable(initial_color_table_black);
    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "note_color_table"), 2);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, color_table_ubo);

    glGenBuffers(1, &note_state_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, note_state_ubo);
    glBufferStorage(GL_UNIFORM_BUFFER, 128 * 4, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sharp_table.size() * 4, sharp_table.data());
    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "note_state"), 3);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, note_state_ubo);

    // map note state into host memory
    mapped_note_state_ubo = (int*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    std::copy(sharp_table.begin(), sharp_table.end(), mapped_note_state_ubo);
}

int Piano::AddToColorTable(const float* color) {
    glBindBuffer(GL_UNIFORM_BUFFER, color_table_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, color_table_pos * 4 * 4, 16, color);
    color_table_pos++;
    return color_table_pos - 1;
}

// NOTE: std::set has no ::back(), so i had to use ::rbegin()
// i hate this as much as you do

__declspec(noinline) void Piano::NoteOn(int track, int note) {
    note = limit_note(note);
    auto& set = cur_active_tracks[note];
    // do nothing if trying to turn on a note that's already on
    if (set.count(track)) return;
    const int color_index = track + 2;
    // check if this note is displayed on top of the others
    // if not, don't change the color
    // this check works because std::set is sorted
    if (set.size() == 0 || *set.rbegin() < track) {
        draw_queue.push_back({ color_index, note });
    }
    set.insert(track);
}

__declspec(noinline) void Piano::NoteOff(int track, int note) {
    note = limit_note(note);
    auto& set = cur_active_tracks[note];
    int color_index = 0;
    // do nothing if trying to turn off a note that was never on
    if (set.size() == 0 || set.count(track) == 0) return;
    if (set.size() == 1 && *set.begin() == track) {
        // track to pop is the last track on the note
        set.erase(track);
        color_index = IsSharp(note);
        draw_queue.push_back({ color_index, note });
    } else if (*set.rbegin() == track) {
        // track to pop is the top track on the note
        set.erase(std::next(set.rbegin()).base());
        color_index = *set.rbegin() + 2;
        draw_queue.push_back({ color_index, note });
    } else {
        // track meets none of the other criteria (probably hottest path)
        set.erase(track);
    }
}

__declspec(noinline) void Piano::Draw() {
    // Wait for GPU
    // https://ferransole.wordpress.com/2014/06/08/persistent-mapped-buffers/
    if (fence) {
        while (true) {
            GLenum wait_ret = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait_ret == GL_ALREADY_SIGNALED || wait_ret == GL_CONDITION_SATISFIED)
                break;
        }
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);
    glUseProgram(program);
    for (const auto& draw_call : draw_queue)
        mapped_note_state_ubo[draw_call.note] = draw_call.color_index;
    draw_queue.clear();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    // Make the fence
    if (fence)
        glDeleteSync(fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool Piano::IsSharp(int note) const {
    return sharp_table[note];
}