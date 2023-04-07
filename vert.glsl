#version 330 vore
layout(location = 0) in vec3 vertexPosition_modelspace;

void main() {
    gl_Position.xyz = vertexPosition_modelSpace;
    gl_Position.w = 1.0;
}
