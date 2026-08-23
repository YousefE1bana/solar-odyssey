#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vWorldPos;
in vec3 vLocalPos;
in vec3 vTangentEye;

out vec4 FragColor;

uniform float uTime;
uniform float uInnerRadius;
uniform float uOuterRadius;
uniform float uShadowRadius;
uniform vec3 uCameraPos;
uniform int uRenderPass; // 0 = Disk Plane, 1 = Lensing Ring / Sphere, 2 = Polar Jets

// Procedural Hash and Noise
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.5));
    for (int i = 0; i < 4; ++i) {
        v += a * noise(p);
        p = rot * p * 2.0;
        a *= 0.5;
    }
    return v;
}

void main() {
    if (uRenderPass == 0) {
        // === ACCRETION DISK PASS ===
        float r = length(vLocalPos.xz);

        // Normalized radial distance [0, 1]
        float normR = (r - uInnerRadius) / (uOuterRadius - uInnerRadius);

        // Polar angle
        float phi = atan(vLocalPos.z, vLocalPos.x);

        // Keplerian differential rotation: inner orbits faster than outer (omega ~ r^-1.5)
        float speed = pow(uInnerRadius / r, 1.5) * 1.8;
        float rotAngle = phi - uTime * speed;

        // Spiral coordinates for turbulence
        vec2 spiralCoord = vec2(normR * 6.0 - rotAngle * 0.8, rotAngle * 2.5);
        float turb = fbm(spiralCoord * 3.0);
        float filaments = sin(rotAngle * 6.0 + normR * 12.0 + turb * 4.0) * 0.5 + 0.5;
        filaments = pow(filaments, 1.6);

        // Relativistic Doppler Beaming
        // vTangentEye is the orbital tangent already transformed to EYE space by the
        // vertex shader, matching vViewDir's space — beaming is now geometrically
        // correct regardless of modelview rotation.
        vec3 viewDir = normalize(vViewDir);
        float dopplerDot = dot(normalize(vTangentEye), viewDir);

        // Relativistic beaming factor (approaching side is significantly brighter and blueshifted)
        float dopplerFactor = clamp(1.0 + dopplerDot * 0.65, 0.35, 1.95);

        // Temperature Gradient: Inner white-hot -> Middle golden amber -> Outer deep crimson
        vec3 colorWhite = vec3(1.0, 0.98, 0.92) * 2.8;
        vec3 colorGold  = vec3(1.0, 0.62, 0.12) * 2.0;
        vec3 colorAmber = vec3(0.95, 0.35, 0.05) * 1.5;
        vec3 colorCrimson = vec3(0.50, 0.08, 0.02) * 0.8;

        vec3 diskColor;
        if (normR < 0.15) {
            diskColor = mix(colorWhite, colorGold, normR / 0.15);
        } else if (normR < 0.55) {
            diskColor = mix(colorGold, colorAmber, (normR - 0.15) / 0.40);
        } else {
            diskColor = mix(colorAmber, colorCrimson, (normR - 0.55) / 0.45);
        }

        // Apply turbulence and filaments
        diskColor *= (0.65 + 0.70 * filaments + 0.35 * turb);
        diskColor *= dopplerFactor;

        // Radial opacity falloff (sharp at inner edge, soft fade at outer edge)
        float innerFade = smoothstep(0.0, 0.06, normR);
        float outerFade = smoothstep(1.0, 0.75, normR);
        float alpha = innerFade * outerFade * 0.95;

        // Inner Photon Ring Glow Accent
        float photonProximity = smoothstep(0.08, 0.0, normR);
        diskColor += vec3(1.0, 0.95, 0.85) * photonProximity * 2.2;
        alpha = max(alpha, photonProximity * 0.98);

        FragColor = vec4(diskColor, alpha);

    } else if (uRenderPass == 1) {
        // === GRAVITATIONAL LENSING & PHOTON SPHERE PASS ===
        float r = length(vLocalPos);
        float ndotv = max(0.0, dot(vNormal, vViewDir));

        // Photon sphere ring at the silhouette edge
        float limbGlow = pow(1.0 - ndotv, 3.5);
        vec3 ringColor = vec3(1.0, 0.78, 0.35) * limbGlow * 3.2;

        // Event Horizon shadow center (pitch black)
        if (r < uShadowRadius * 1.05) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            FragColor = vec4(ringColor, limbGlow * 0.85);
        }

    } else {
        // === RELATIVISTIC POLAR JETS PASS ===
        float h = abs(vLocalPos.y);
        float rad = length(vLocalPos.xz);
        float beamRadius = 0.35 + h * 0.08;

        if (rad > beamRadius * 2.5) discard;

        float core = clamp(1.0 - (rad / beamRadius), 0.0, 1.0);
        core = pow(core, 2.0);

        // Animated pulse along jet
        float pulse = sin(h * 3.0 - uTime * 6.0) * 0.5 + 0.5;

        vec3 jetColor = mix(vec3(0.3, 0.6, 1.0), vec3(0.8, 0.9, 1.0), core) * 2.5;
        jetColor += vec3(0.4, 0.7, 1.0) * pulse * 1.2;

        float heightFade = smoothstep(22.0, 5.0, h);
        float alpha = core * heightFade * 0.75;

        FragColor = vec4(jetColor, alpha);
    }
}
