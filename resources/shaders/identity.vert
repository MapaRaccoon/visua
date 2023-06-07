#version 330 core
layout(location = 0) in vec3 position;

out vec2 vPos;

void main() {
    gl_Position.xyz = position;
    gl_Position.w = 1.0;
    vPos.xy = position.xy;
}
