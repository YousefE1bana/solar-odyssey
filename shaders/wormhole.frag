#version 450 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vViewDir;
in vec3 vLocalPos;

out vec4 FragColor;

uniform float uTime;
uniform int uMeshType;       // 0 = Throat Sphere, 1 = Accretion Vortex Disk, 2 = Gravitational Halo Arch
uniform vec3 uWormholePos;
uniform vec3 uCameraPos;
uniform float uRadius;

// C3.6 Portal Uniforms
uniform sampler2D uPortalTex;
uniform bool uPortalAvailable;
uniform float uThroatRadius;
uniform vec3 uApertureRightLocal;
uniform vec3 uApertureUpLocal;
uniform vec3 uApertureNormalLocal;
uniform bool uIsInsideThroat;
uniform float uTransitionProgress;

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

    // For Accretion Disk (Type 1) and Halo Arches (Type 2):
    // When portal is active, carve out the portal aperture cylinder so disk/halo geometry
    // NEVER occludes or contaminates the interior portal window from ANY viewpoint.
    if (uPortalAvailable && !uIsInsideThroat && (uMeshType == 1 || uMeshType == 2)) {
        vec3 rayDir = normalize(vWorldPos - uCameraPos);
        vec3 toCenter = uWormholePos - uCameraPos;
        float tClose = dot(toCenter, rayDir);
        if (tClose > 0.0) {
            vec3 perp = (uCameraPos + tClose * rayDir) - uWormholePos;
            if (dot(perp, perp) < (uThroatRadius * uThroatRadius)) {
                discard;
            }
        }
    }

    // TYPE 0: THROAT SPHERE (Center portal depth / hyperspace gateway)
    if (uMeshType == 0) {
        if (!uPortalAvailable || uIsInsideThroat) {
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

        // Safety: cull back-facing fragments on the throat sphere
        if (!uIsInsideThroat && dot(vNormal, vViewDir) <= 0.0) {
            discard;
        }

        // C3.6 Portal Active Path:
        // 1. Portal-Local Coordinate Space Mapping:
        // Use model-space vLocalPos (= aPos, where ||vLocalPos|| == uThroatRadius).
        // Orthogonally project vLocalPos onto the local aperture basis:
        float r_throat = (uThroatRadius > 0.001) ? uThroatRadius : 4.2;
        float u_ap = dot(vLocalPos, uApertureRightLocal) / r_throat;
        float v_ap = dot(vLocalPos, uApertureUpLocal) / r_throat;

        // Map aperture coordinate to normalized offset d from center where ||d|| <= 0.5
        vec2 d = vec2(u_ap, v_ap) * 0.5;
        float rho = clamp(length(vec2(u_ap, v_ap)), 0.0, 1.0); // rho in [0, 1] on sphere surface

        // 2. Inward Throat Compression (Contract: pushes samples toward center):
        // warpWeight = k * rho^2. uvWarped = center + d * (1.0 - warpWeight)
        float k_warp = 0.22;
        float warpWeight = k_warp * rho * rho;
        vec2 uvCenter = vec2(0.5, 0.5);

        // Subtle spacetime throat ripple: bounded by construction
        float rippleAmp = 0.012;
        float ripple = rippleAmp * sin(rho * 16.0 - uTime * 2.8);
        vec2 warpDir = (rho > 1e-4) ? normalize(d) : vec2(0.0);
        vec2 uvWarped = uvCenter + d * (1.0 - warpWeight) + warpDir * ripple;

        // 3. Stylized / Fictional Throat-Interface Spectral Dispersion:
        // Vacuum GR lensing is strictly achromatic. Here, RGB chromatic separation is a deliberately
        // stylized Wormhole 2.0 visual cue representing fictional throat-interface boundary dispersion.
        // Vanishes at center (rho = 0) so the destination scene remains 100% sharp and readable.
        float k_disp = 0.007;
        float dispDist = k_disp * rho * rho;
        vec2 uvR = uvWarped - warpDir * dispDist;
        vec2 uvG = uvWarped;
        vec2 uvB = uvWarped + warpDir * dispDist;

        // Numerical safety clamp (by construction, samples remain strictly in [0.08, 0.92])
        uvR = clamp(uvR, 0.002, 0.998);
        uvG = clamp(uvG, 0.002, 0.998);
        uvB = clamp(uvB, 0.002, 0.998);

        float sR = texture(uPortalTex, uvR).r;
        float sG = texture(uPortalTex, uvG).g;
        float sB = texture(uPortalTex, uvB).b;
        vec3 portalColor = vec3(sR, sG, sB);

        // 4. Throat Boundary Halo Rim Blending:
        // Inside aperture (rho <= 0.85): 100% pure portal destination scene.
        // Zero fresnel or halo wash inside the interior aperture window.
        float rimFactor = smoothstep(0.85, 0.98, rho);

        // Ethereal cyan/violet throat interface boundary glow (confined to rim)
        vec3 midCyan   = vec3(0.0, 0.85, 1.0);
        vec3 hotViolet = vec3(0.85, 0.2, 1.0);
        vec3 haloColor = mix(hotViolet, midCyan, 0.5 + 0.5 * sin(uTime * 1.5 + rho * 6.28));
        haloColor += vec3(0.9, 0.95, 1.0) * pow(1.0 - NdotV, 3.0) * 1.5;

        // Blend portal view with boundary glow ONLY at the outer boundary rim
        vec3 compositedThroat = mix(portalColor, haloColor, rimFactor);

        // Traversal transition flare effect if uTransitionProgress > 0
        if (uTransitionProgress > 0.0) {
            float tau = clamp(uTransitionProgress, 0.0, 1.0);
            float flare = sin(tau * 3.14159265);
            compositedThroat += vec3(0.9, 0.95, 1.0) * flare * 0.85;
        }

        FragColor = vec4(compositedThroat, 1.0);
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
        float outerFade = (1.0 - smoothstep(0.70, 1.0, r));
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

        float archMask = smoothstep(0.3, 0.6, r) * (1.0 - smoothstep(0.75, 1.0, r));
        float pulse = 0.85 + 0.15 * sin(uTime * 3.0);

        vec3 haloColor = mix(vec3(0.0, 0.8, 1.0), vec3(0.8, 0.2, 1.0), r);
        haloColor += vec3(0.9, 0.95, 1.0) * fresnel * 2.0;

        float alpha = clamp(archMask * fresnel * 1.2 * pulse, 0.0, 1.0);
        FragColor = vec4(haloColor, alpha);
        return;
    }

    FragColor = vec4(0.0, 0.85, 1.0, 1.0);
}
