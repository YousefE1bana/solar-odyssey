#version 450 core

in vec2 vScreenUV;
out vec4 FragColor;

uniform sampler2D uSceneColorTex;

// Camera and coordinate system (authoritative origin is the Black Hole)
uniform vec3 uCameraLocal;
uniform mat4 uInvProjection;
uniform mat4 uProjection;
uniform mat4 uViewMatrix;
uniform mat4 uInvViewMatrix;
uniform vec2 uScreenResolution;

// Authoritative Black Hole parameters
uniform float uSchwarzschildRadius;   // r_s (default 2.5)
uniform float uLensingInfluenceRadius; // R_infl (default 24.0)
uniform float uAccretionDiskInner;     // r_in (default 4.0)
uniform float uAccretionDiskOuter;     // r_out (default 18.0)
uniform float uTime;
uniform int uMaxSteps;                 // default 24

// Procedural noise for secondary accretion disk appearance
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

// Evaluate disk emission at a local hit position on Y = 0 plane
vec4 evaluateDiskEmission(vec3 hitPos, vec3 rayDir, float rs, float rIn, float rOut) {
    float r = length(hitPos.xz);
    if (r < rIn || r > rOut) return vec4(0.0);

    float normR = (r - rIn) / (rOut - rIn);
    float phi = atan(hitPos.z, hitPos.x);

    // Differential Keplerian rotation
    float speed = pow(rIn / max(r, 0.1), 1.5) * 1.8;
    float rotAngle = phi - uTime * speed;

    vec2 spiralCoord = vec2(normR * 6.0 - rotAngle * 0.8, rotAngle * 2.5);
    float turb = fbm(spiralCoord * 3.0);
    float filaments = sin(rotAngle * 6.0 + normR * 12.0 + turb * 4.0) * 0.5 + 0.5;
    filaments = pow(filaments, 1.6);

    // Relativistic Doppler Beaming
    // Orbital tangent in disk plane
    vec3 tangent = normalize(vec3(-hitPos.z, 0.0, hitPos.x));
    // Emission direction to observer (backwards along ray towards camera)
    vec3 kObs = -rayDir;
    float cosTheta = dot(tangent, kObs);

    float beta = clamp(sqrt(rs / (2.0 * max(r, rs * 1.1))), 0.0, 0.70);
    float gamma = 1.0 / sqrt(clamp(1.0 - beta * beta, 0.0001, 1.0));
    float dopplerFactor = clamp(1.0 / (gamma * (1.0 - beta * cosTheta)), 0.2, 3.0);

    // Gravitational Redshift
    float redshift = sqrt(clamp(1.0 - rs / max(r, rs * 1.001), 0.0, 1.0));

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

    diskColor *= (0.65 + 0.70 * filaments + 0.35 * turb);
    diskColor *= (dopplerFactor * redshift);

    float innerFade = smoothstep(0.0, 0.06, normR);
    float outerFade = smoothstep(1.0, 0.75, normR);
    float alpha = innerFade * outerFade * 0.92;

    float photonProximity = smoothstep(0.08, 0.0, normR);
    diskColor += vec3(1.0, 0.95, 0.85) * photonProximity * 2.2;
    alpha = max(alpha, photonProximity * 0.98);

    return vec4(diskColor, alpha);
}

void main() {
    // 1. Reconstruct eye-space ray direction from screen UV
    vec2 ndc = vScreenUV * 2.0 - 1.0;
    vec4 clipRay = vec4(ndc, 1.0, 1.0);
    vec4 eyeRayH = uInvProjection * clipRay;
    vec3 eyeDir = normalize(eyeRayH.xyz / max(abs(eyeRayH.w), 1e-6));

    // 2. Transform ray to Black-Hole-centered local frame
    // World ray direction:
    vec3 worldDir = normalize(mat3(uInvViewMatrix) * eyeDir);
    // Origin is Black Hole:
    vec3 rayPos = uCameraLocal;
    vec3 rayDir = worldDir;
    vec3 initialDir = worldDir;

    float rs = uSchwarzschildRadius;
    float rInfl = uLensingInfluenceRadius;
    float rHorizon = rs * 1.05; // Explicit event-horizon capture boundary

    // 3. Analytic Jump to Influence Boundary
    float r0 = length(rayPos);
    if (r0 > rInfl) {
        float b0 = dot(rayPos, rayDir);
        float c0 = dot(rayPos, rayPos) - rInfl * rInfl;
        float discr = b0 * b0 - c0;
        if (discr >= 0.0) {
            float tEnter = -b0 - sqrt(discr);
            if (tEnter > 0.0) {
                rayPos += rayDir * tEnter;
            } else {
                // Ray points away from influence sphere: passthrough
                FragColor = texture(uSceneColorTex, vScreenUV);
                return;
            }
        } else {
            // Ray misses influence sphere entirely: passthrough
            FragColor = texture(uSceneColorTex, vScreenUV);
            return;
        }
    }

    // 4. Numerical Integration Loop (tier-controlled; Low = 8-step fast bounded mode)
    vec4 secondaryDiskAcc = vec4(0.0);
    bool captured = false;
    int steps = max(uMaxSteps, 4);
    float dtBase = (2.0 * rInfl) / float(steps);

    for (int i = 0; i < steps; ++i) {
        float r = length(rayPos);

        // Check Event Horizon Capture
        if (r <= rHorizon) {
            captured = true;
            break;
        }

        // Distance-aware adaptive step size
        float stepScale = clamp((r - rs) / (rInfl - rs), 0.40, 1.40);
        float dt = dtBase * stepScale;

        // Schwarzschild-inspired acceleration
        float r3 = r * r * r;
        float factor = (rs / max(r3, 1e-6)) * (1.0 + 1.5 * rs / max(r, 1e-3));
        vec3 accel = -factor * rayPos;

        vec3 nextPos = rayPos + rayDir * dt + 0.5 * accel * (dt * dt);
        vec3 nextDir = normalize(rayDir + accel * dt);

        // Topological Secondary Disk Crossing Detection
        // Check if ray crosses physical disk plane Y = 0
        if (rayPos.y * nextPos.y <= 0.0 && abs(nextPos.y - rayPos.y) > 1e-6) {
            float tHit = -rayPos.y / (nextPos.y - rayPos.y);
            vec3 hitPos = mix(rayPos, nextPos, clamp(tHit, 0.0, 1.0));
            float rHit = length(hitPos.xz);

            // Topological classification:
            // (a) Deflection occurred: dot(rayDir, initialDir) < 0.99
            // (b) Post-periapsis: dot(rayPos, rayDir) >= -0.05
            // (c) Hit radius within physical disk bounds
            bool isDeflected = dot(rayDir, initialDir) < 0.99;
            bool isPostPeriapsis = dot(rayPos, rayDir) >= -0.05;

            // Supplementary far-side check if observer projection on disk plane has magnitude
            bool farSideOk = true;
            if (length(uCameraLocal.xz) > 0.5) {
                farSideOk = dot(hitPos.xz, uCameraLocal.xz) < 0.0;
            }

            if (isDeflected && isPostPeriapsis && farSideOk && rHit >= uAccretionDiskInner && rHit <= uAccretionDiskOuter) {
                vec4 diskSample = evaluateDiskEmission(hitPos, rayDir, rs, uAccretionDiskInner, uAccretionDiskOuter);
                secondaryDiskAcc.rgb += diskSample.rgb * (1.0 - secondaryDiskAcc.a);
                secondaryDiskAcc.a += diskSample.a * (1.0 - secondaryDiskAcc.a);
            }
        }

        rayPos = nextPos;
        rayDir = nextDir;

        // Escape condition: ray has exited the influence sphere moving outward
        if (length(rayPos) > rInfl && dot(rayPos, rayDir) > 0.0) {
            break;
        }
    }

    // 5. Output Composition
    if (captured) {
        // Horizon shadow: pure black singularity silhouette (composited with secondary disk if intersected)
        FragColor = vec4(secondaryDiskAcc.rgb, 1.0);
        return;
    }

    // 6. Deflected Background Sampling & Fallback
    // Transform final ray direction to eye space
    vec3 eyeFinal = mat3(uViewMatrix) * rayDir;

    vec3 bgSample = vec3(0.0);
    // Passthrough-safe guard: if ray points behind camera or invalid projection, fallback to original UV
    if (eyeFinal.z >= -0.01) {
        bgSample = texture(uSceneColorTex, vScreenUV).rgb;
    } else {
        vec4 clipFinal = uProjection * vec4(eyeFinal, 0.0);
        if (clipFinal.w <= 0.0001) {
            bgSample = texture(uSceneColorTex, vScreenUV).rgb;
        } else {
            vec2 deflectedUV = (clipFinal.xy / clipFinal.w) * 0.5 + 0.5;
            if (deflectedUV.x < 0.0 || deflectedUV.x > 1.0 || deflectedUV.y < 0.0 || deflectedUV.y > 1.0) {
                bgSample = texture(uSceneColorTex, vScreenUV).rgb;
            } else {
                bgSample = texture(uSceneColorTex, deflectedUV).rgb;
            }
        }
    }

    vec3 finalColor = secondaryDiskAcc.rgb + (1.0 - secondaryDiskAcc.a) * bgSample;
    FragColor = vec4(finalColor, 1.0);
}
