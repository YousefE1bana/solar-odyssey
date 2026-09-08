#version 450 core
layout(location = 0) in vec2 aPos;
out vec2 vScreenUV;

void main() {
    vScreenUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
