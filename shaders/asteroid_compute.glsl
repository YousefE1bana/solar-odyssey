#version 430 core

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GPUAsteroid {
    vec4 position_size;     // xyz = position, w = size
    vec4 rotation_speed;    // xyz = rotation (deg), w = orbitSpeed
    vec4 rotSpeed_offset;   // xyz = rotationSpeed, w = orbitOffset
    vec4 orbitParams;       // x = orbitRadius, y = orbitInclination, z = eccentricity, w = brightness
    vec4 materialColor;     // rgb = materialColor, a = unused
};

layout (std430, binding = 0) buffer AsteroidBlock {
    GPUAsteroid asteroids[];
};

uniform float uDeltaTime;
uniform float uPlanetSpeed;
uniform int   uAsteroidCount;

const float DEG2RAD = 0.01745329251994329576923690768489;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= uint(uAsteroidCount)) return;

    GPUAsteroid ast = asteroids[idx];

    float orbitSpeed = ast.rotation_speed.w;
    float orbitOffset = ast.rotSpeed_offset.w;
    float orbitRadius = ast.orbitParams.x;
    float orbitInclination = ast.orbitParams.y;
    float eccentricity = ast.orbitParams.z;

    // Update orbital phase
    float angle = orbitOffset + uDeltaTime * orbitSpeed * uPlanetSpeed;
    angle = mod(angle, 360.0);
    ast.rotSpeed_offset.w = angle;

    // Keplerian elliptical coordinate computation
    float angleRad = angle * DEG2RAD;
    float rad = orbitRadius * (1.0 + eccentricity * cos(angleRad));
    float px = rad * cos(angleRad);
    float py = rad * sin(angleRad) * sin(orbitInclination);
    float pz = rad * sin(angleRad) * cos(orbitInclination);
    ast.position_size.xyz = vec3(px, py, pz);

    // Integrate Euler rotation
    vec3 rotSpeed = ast.rotSpeed_offset.xyz;
    ast.rotation_speed.xyz += rotSpeed * uDeltaTime * 50.0;

    asteroids[idx] = ast;
}
