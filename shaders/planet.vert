#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPos;
out vec3 vViewDir;
out vec3 vSunDir;
out vec3 vLocalPos;
out vec3 vLocalSunDir;

uniform mat4 uModelView;      // model * view (eye-space transforms)
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;   // inverse-transpose of upper-left 3x3 of uModelView
uniform vec3 uSunEyePos;      // Sun position in eye space
uniform vec3 uSunLocalPos;    // Sun position in object/local space

uniform int uIsRing;
uniform float uRingInnerRadius;
uniform float uRingOuterRadius;

void main() {
    vTexCoord = aTexCoord;

    vec3 localPos = aPos;
    vec3 localNormal = aNormal;

    if (uIsRing > 0) {
        float r = mix(uRingInnerRadius, uRingOuterRadius, aTexCoord.x);
        localPos = vec3(aPos.x * r, 0.0, aPos.z * r);
        localNormal = vec3(0.0, 1.0, 0.0);
    }

    vLocalPos = localPos;

    // Normal in eye space
    vNormal = normalize(uNormalMatrix * localNormal);

    // Vertex position in eye space
    vec4 eyePos = uModelView * vec4(localPos, 1.0);
    vViewDir = normalize(-eyePos.xyz);
    vWorldPos = eyePos.xyz;

    // Exact vector from vertex to sun in eye space & local space
    vSunDir = normalize(uSunEyePos - eyePos.xyz);
    vLocalSunDir = normalize(uSunLocalPos - localPos);

    gl_Position = uProjection * eyePos;
}
