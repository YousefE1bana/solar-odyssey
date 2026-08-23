#version 450 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vPosition;

out vec4 FragColor;

uniform sampler2D uSunTex;
uniform float uTime;
uniform float uSunBrightness; // Default ~1.0 - 1.4

// Multi-frequency procedural turbulence
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
        p = rot * p * 2.0 + vec2(uTime * 0.035);
        a *= 0.5;
    }
    return v;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    float NdotV = max(dot(N, V), 0.0);

    // Multi-speed convection UV flow
    vec2 uv1 = vTexCoord + vec2(uTime * 0.012, uTime * 0.004);
    vec2 uv2 = vec2(vTexCoord.x * 1.6 - uTime * 0.008, vTexCoord.y * 1.6 + uTime * 0.006);

    vec4 tex1 = texture(uSunTex, uv1);
    vec4 tex2 = texture(uSunTex, uv2);
    vec3 baseTexture = mix(tex1.rgb, tex2.rgb, 0.5);

    // Multi-scale convective plasma turbulence
    float turbulence = fbm(vTexCoord * 14.0 + vec2(uTime * 0.04));
    float granules = noise(vTexCoord * 55.0 + vec2(uTime * 0.06));
    float microGranules = noise(vTexCoord * 110.0 - vec2(uTime * 0.08));

    // Rich physical solar palette:
    // Cooler intergranular lanes -> Warm golden convective cells -> Hot plasma peaks
    vec3 deepAmber = vec3(0.85, 0.32, 0.02);
    vec3 richGold = vec3(1.00, 0.68, 0.14);
    vec3 brightPlasma = vec3(1.15, 0.96, 0.55);

    // Synthesize surface convection texture
    float heat = baseTexture.r * 0.6 + turbulence * 0.4;
    heat += (granules - 0.5) * 0.22 + (microGranules - 0.5) * 0.10;
    heat = clamp(heat, 0.0, 1.0);

    vec3 surfacePlasma = mix(deepAmber, richGold, smoothstep(0.15, 0.65, heat));
    surfacePlasma = mix(surfacePlasma, brightPlasma, smoothstep(0.65, 0.98, heat));

    // Stellar limb darkening: centers are optical-depth deeper (brighter), edges darken slightly
    float limbDarkening = 0.45 + 0.55 * pow(NdotV, 0.65);
    vec3 solarBody = surfacePlasma * limbDarkening;

    // Atmospheric coronal limb glow (soft radiant orange-gold halo at limb)
    float rim = pow(1.0 - NdotV, 3.2);
    vec3 coronaGlow = vec3(1.35, 0.65, 0.12) * rim * 1.4;

    // Combined radiance with controlled HDR intensity
    float brightness = (uSunBrightness > 0.0) ? uSunBrightness : 1.0;
    vec3 finalSun = (solarBody * 1.15 + coronaGlow) * brightness;

    // Subtle gentle pulsation
    float pulse = 1.0 + 0.02 * sin(uTime * 1.5);
    finalSun *= pulse;

    FragColor = vec4(finalSun, 1.0);
}
