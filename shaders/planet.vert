#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPos;
out vec3 vViewDir;
out vec3 vSunDir;

uniform mat4 uModelView;      // model * view (eye-space transforms)
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;   // inverse-transpose of upper-left 3x3 of uModelView
uniform vec3 uSunEyePos;      // Sun position in eye space

void main() {
    vTexCoord = aTexCoord;

    // Normal in eye space
    vNormal = normalize(uNormalMatrix * aNormal);

    // Vertex position in eye space
    vec4 eyePos = uModelView * vec4(aPos, 1.0);
    vViewDir = normalize(-eyePos.xyz);
    vWorldPos = eyePos.xyz;

    // Exact vector from vertex to sun in eye space
    vSunDir = normalize(uSunEyePos - eyePos.xyz);

    gl_Position = uProjection * eyePos;
}
