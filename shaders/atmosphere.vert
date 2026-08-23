#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 vNormal;
out vec3 vViewDir;
out vec3 vSunDir;

uniform mat4 uModelView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;
uniform vec3 uSunEyePos; // Sun position in eye space

void main() {
    vNormal = normalize(uNormalMatrix * aNormal);
    vec4 eyePos = uModelView * vec4(aPos, 1.0);
    vViewDir = normalize(-eyePos.xyz);

    // Exact vector from vertex to sun in eye space
    vSunDir = normalize(uSunEyePos - eyePos.xyz);

    gl_Position = uProjection * eyePos;
}
