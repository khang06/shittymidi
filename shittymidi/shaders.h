#pragma once

const char* kbd_vert_shader_src = R"(
#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in int note_idx;
flat out vec4 note_color;

layout (shared) uniform note_color_table {
    vec4 color[2048]; // Should be enough
};

layout (shared) uniform note_state {
    int state[128];
};

void main() {
    gl_Position = vec4(in_pos, 1.0);
    note_color = color[state[note_idx]];
}
)";

const char* kbd_frag_shader_src = R"(
#version 330 core
flat in vec4 note_color;
out vec4 out_color;

void main() {
    out_color = note_color;
}
)";