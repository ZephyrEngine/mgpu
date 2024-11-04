#version 450

layout(location = 0) in vec3 v_color;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(v_color, 0.5);
}