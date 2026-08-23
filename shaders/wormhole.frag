#version 450 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;

out vec4 FragColor;

uniform float uTime;
uniform int uMeshType;       // 0 = Throat Sphere, 1 = Accretion Vortex Disk, 2 = Gravitational Halo Arch
uniform vec3 uWormholePos;
uniform float uRadius;

// 2D Hash function for procedural noise
float hash21(vec2 p) {
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

// 2D Value Noise
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractional Brownian Motion (4 octaves)
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.5));
    for (int i = 0; i < 4; ++i) {
        v += a * noise(p);
        p = rot * p * 2.0 + vec2(100.0);
        a *= 0.5;
    }
    return v;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    float NdotV = max(0.0, dot(N, V));

    // TYPE 0: THROAT SPHERE (Center portal depth / hyperspace gateway)
    if (uMeshType == 0) {
        float fresnel = pow(1.0 - NdotV, 2.5);

        // Swirling vortex coordinate
        vec2 uv = vTexCoord * 2.0 - 1.0;
        float r = length(uv);
        float angle = atan(uv.y, uv.x);

        float swirl = angle + 4.0 / (r + 0.2) - uTime * 2.5;
        float n = fbm(vec2(cos(swirl) * r * 4.0, sin(swirl) * r * 4.0 + uTime * 0.8));

        // Ethereal cyan/magenta/deep space palette
        vec3 deepColor = vec3(0.02, 0.01, 0.08);
        vec3 midCyan   = vec3(0.0, 0.85, 1.0);
        vec3 hotViolet = vec3(0.85, 0.2, 1.0);
        vec3 coreWhite = vec3(0.9, 0.95, 1.0);

        vec3 portalColor = mix(deepColor, hotViolet, smoothstep(0.1, 0.6, n));
        portalColor = mix(portalColor, midCyan, smoothstep(0.4, 0.8, n * (1.0 - r * 0.5)));
        portalColor += coreWhite * pow(fresnel, 3.0) * 1.5;

        // Rim intensity
        float alpha = clamp(0.75 + fresnel * 0.35, 0.0, 1.0);
        FragColor = vec4(portalColor, alpha);
        return;
    }

    // TYPE 1: ACCRETION VORTEX DISK
    if (uMeshType == 1) {
        vec2 uv = (vTexCoord - 0.5) * 2.0;
        float r = length(uv);
        if (r < 0.22 || r > 1.0) discard;

        float angle = atan(uv.y, uv.x);
        // Differential rotational swirl: inner orbits faster than outer
        float rotSpeed = 3.5 / pow(r + 0.1, 1.2);
        float swirlAngle = angle + uTime * rotSpeed;

        vec2 swirlUV = vec2(cos(swirlAngle) * r * 5.0, sin(swirlAngle) * r * 5.0);
        float turbulence = fbm(swirlUV + vec2(uTime * 0.4));
        float turbulence2 = fbm(swirlUV * 2.0 - vec2(uTime * 0.6));
        float density = turbulence * 0.65 + turbulence2 * 0.35;

        // Radial opacity envelope
        float innerFade = smoothstep(0.22, 0.35, r);
        float outerFade = smoothstep(1.0, 0.70, r);
        float radialMask = innerFade * outerFade;

        // Cosmic color gradient: Inner radiant cyan-white -> Mid violet -> Outer deep magenta
        vec3 innerWhite = vec3(0.95, 0.98, 1.0);
        vec3 midCyan    = vec3(0.0, 0.88, 1.0);
        vec3 outerViolet= vec3(0.65, 0.1, 0.95);
        vec3 edgeDust   = vec3(0.35, 0.02, 0.5);

        float tNorm = clamp((r - 0.22) / 0.78, 0.0, 1.0);
        vec3 diskColor = mix(innerWhite, midCyan, smoothstep(0.0, 0.25, tNorm));
        diskColor = mix(diskColor, outerViolet, smoothstep(0.25, 0.70, tNorm));
        diskColor = mix(diskColor, edgeDust, smoothstep(0.70, 1.0, tNorm));

        diskColor *= (0.7 + density * 0.9);

        // Relativistic Doppler beaming on disk rotation
        vec3 diskTangent = normalize(vec3(-sin(angle), 0.0, cos(angle)));
        float dopplerFactor = 1.0 + 0.45 * dot(diskTangent, V);
        diskColor *= dopplerFactor;

        float alpha = clamp(radialMask * (0.6 + density * 0.6), 0.0, 1.0);
        FragColor = vec4(diskColor * 1.3, alpha);
        return;
    }

    // TYPE 2: GRAVITATIONAL LENSING HALO ARCH (Einstein Ring Warp)
    if (uMeshType == 2) {
        float fresnel = pow(1.0 - NdotV, 3.0);
        vec2 uv = (vTexCoord - 0.5) * 2.0;
        float r = length(uv);

        float archMask = smoothstep(0.3, 0.6, r) * smoothstep(1.0, 0.75, r);
        float pulse = 0.85 + 0.15 * sin(uTime * 3.0);

        vec3 haloColor = mix(vec3(0.0, 0.8, 1.0), vec3(0.8, 0.2, 1.0), r);
        haloColor += vec3(0.9, 0.95, 1.0) * fresnel * 2.0;

        float alpha = clamp(archMask * fresnel * 1.2 * pulse, 0.0, 1.0);
        FragColor = vec4(haloColor, alpha);
        return;
    }

    FragColor = vec4(0.0, 0.85, 1.0, 1.0);
}
