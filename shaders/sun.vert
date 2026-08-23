#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vViewDir;
out vec3 vPosition;

uniform mat4 uModelView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;
uniform float uTime;

void main() {
    vTexCoord = aTexCoord;
    vNormal = normalize(uNormalMatrix * aNormal);

    vec4 eyePos = uModelView * vec4(aPos, 1.0);
    vViewDir = normalize(-eyePos.xyz);
    vPosition = aPos;

    gl_Position = uProjection * eyePos;
}
