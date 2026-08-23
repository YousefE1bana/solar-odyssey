#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vViewDir;
out vec3 vWorldPos;
out vec3 vLocalPos;
out vec3 vTangentEye; // orbital tangent direction in EYE space (for Doppler beaming)

uniform mat4 uModelView;    // model * view
uniform mat4 uProjection;
uniform mat3 uNormalMatrix; // inverse-transpose of upper-left 3x3 of uModelView

void main() {
    vTexCoord = aTexCoord;
    vNormal = normalize(uNormalMatrix * aNormal);

    vec4 eyePos = uModelView * vec4(aPos, 1.0);
    vViewDir = normalize(-eyePos.xyz);
    vLocalPos = aPos;
    vWorldPos = eyePos.xyz;

    // Orbital tangent in local space: perpendicular to radial direction (x,z), in
    // the disk plane. Transform to eye space so the fragment shader can dot it
    // with vViewDir (also eye space) — Doppler beaming stays geometrically correct.
    vec3 localTangent = normalize(vec3(-vLocalPos.z, 0.0, vLocalPos.x));
    vTangentEye = normalize(uNormalMatrix * localTangent);

    gl_Position = uProjection * eyePos;
}
