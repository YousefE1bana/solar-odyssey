#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Instanced attributes from AsteroidInstanceData (buffer binding 1, divisor = 1)
layout(location = 3) in vec4 aInstancePosScale;  // xyz = world pos, w = uniform scale/size
layout(location = 4) in vec4 aInstanceRotParams; // xyz = Euler rotation (deg), w = scaleY factor (0.85)
layout(location = 5) in vec4 aInstanceColor;     // rgb = material color, w = brightness

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPos;
out vec3 vViewDir;
out vec3 vSunDir;
out vec4 vInstanceColor;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uSunEyePos;

void main() {
    vTexCoord = aTexCoord;
    vInstanceColor = aInstanceColor;

    // Reconstruct 3D Euler rotation matrix in exact GLM order (Rx * Ry * Rz)
    vec3 rad = radians(aInstanceRotParams.xyz);
    float cx = cos(rad.x), sx = sin(rad.x);
    float cy = cos(rad.y), sy = sin(rad.y);
    float cz = cos(rad.z), sz = sin(rad.z);

    mat3 rx = mat3(
        1.0,  0.0, 0.0,
        0.0,   cx,  sx,
        0.0,  -sx,  cx
    );

    mat3 ry = mat3(
         cy, 0.0, -sy,
        0.0, 1.0, 0.0,
         sy, 0.0,  cy
    );

    mat3 rz = mat3(
         cz,  sz, 0.0,
        -sz,  cz, 0.0,
        0.0, 0.0, 1.0
    );

    mat3 rot = rx * ry * rz;

    // Non-uniform scale (x=size, y=size*0.85, z=size)
    float baseSize = aInstancePosScale.w;
    float scaleYFactor = (aInstanceRotParams.w > 0.0) ? aInstanceRotParams.w : 0.85;
    vec3 scale = vec3(baseSize, baseSize * scaleYFactor, baseSize);

    vec3 localPos = aPos * scale;
    vec3 worldPos = rot * localPos + aInstancePosScale.xyz;

    // Transform to Eye Space
    vec4 eyePos = uView * vec4(worldPos, 1.0);
    gl_Position = uProjection * eyePos;

    vWorldPos = eyePos.xyz;
    vViewDir = normalize(-eyePos.xyz);
    vSunDir = normalize(uSunEyePos - eyePos.xyz);

    // Normal transformation (eye-space)
    vec3 invScale = vec3(1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z);
    vec3 worldNormal = normalize(rot * (aNormal * invScale));
    vNormal = normalize(mat3(uView) * worldNormal);
}
