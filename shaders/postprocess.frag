#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uSceneTex;
uniform sampler2D uBloomTex;

uniform int uPass;             // 0 = Bright Extract, 1 = Blur H, 2 = Blur V, 3 = Final Composite
uniform vec2 uTexelSize;       // 1.0 / resolution
uniform float uBloomThreshold; // Threshold for bright-pass (e.g. 0.8)
uniform float uBloomIntensity; // Intensity multiplier for bloom (e.g. 0.6)
uniform float uExposure;       // Scene exposure (e.g. 1.0)
uniform float uVignette;       // Subtle vignette strength (e.g. 0.25)
uniform float uFadeAlpha;      // Startup fade overlay (1.0 = fully visible, 0.0 = black)
uniform int uEnableBloom;
uniform int uEnableToneMap;

// ACES Filmic Tone Mapping Curve (Narkowicz 2015)
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    // PASS 0: BRIGHT PASS EXTRACTION
    if (uPass == 0) {
        vec3 color = texture(uSceneTex, vTexCoord).rgb;
        // Calculate perceived luminance
        float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
        float threshold = uBloomThreshold;

        // Soft knee thresholding
        float soft = clamp((brightness - threshold + 0.2) / 0.4, 0.0, 1.0);
        vec3 bright = color * soft * smoothstep(threshold * 0.8, threshold * 1.2, brightness);
        FragColor = vec4(bright, 1.0);
        return;
    }

    // PASS 1: HORIZONTAL GAUSSIAN BLUR (9-tap)
    if (uPass == 1) {
        vec2 offset = vec2(uTexelSize.x, 0.0) * 1.5;
        vec3 result = texture(uSceneTex, vTexCoord).rgb * 0.227027;
        result += texture(uSceneTex, vTexCoord + offset * 1.0).rgb * 0.1945946;
        result += texture(uSceneTex, vTexCoord - offset * 1.0).rgb * 0.1945946;
        result += texture(uSceneTex, vTexCoord + offset * 2.0).rgb * 0.1216216;
        result += texture(uSceneTex, vTexCoord - offset * 2.0).rgb * 0.1216216;
        result += texture(uSceneTex, vTexCoord + offset * 3.0).rgb * 0.054054;
        result += texture(uSceneTex, vTexCoord - offset * 3.0).rgb * 0.054054;
        result += texture(uSceneTex, vTexCoord + offset * 4.0).rgb * 0.016216;
        result += texture(uSceneTex, vTexCoord - offset * 4.0).rgb * 0.016216;
        FragColor = vec4(result, 1.0);
        return;
    }

    // PASS 2: VERTICAL GAUSSIAN BLUR (9-tap)
    if (uPass == 2) {
        vec2 offset = vec2(0.0, uTexelSize.y) * 1.5;
        vec3 result = texture(uSceneTex, vTexCoord).rgb * 0.227027;
        result += texture(uSceneTex, vTexCoord + offset * 1.0).rgb * 0.1945946;
        result += texture(uSceneTex, vTexCoord - offset * 1.0).rgb * 0.1945946;
        result += texture(uSceneTex, vTexCoord + offset * 2.0).rgb * 0.1216216;
        result += texture(uSceneTex, vTexCoord - offset * 2.0).rgb * 0.1216216;
        result += texture(uSceneTex, vTexCoord + offset * 3.0).rgb * 0.054054;
        result += texture(uSceneTex, vTexCoord - offset * 3.0).rgb * 0.054054;
        result += texture(uSceneTex, vTexCoord + offset * 4.0).rgb * 0.016216;
        result += texture(uSceneTex, vTexCoord - offset * 4.0).rgb * 0.016216;
        FragColor = vec4(result, 1.0);
        return;
    }

    // PASS 3: FINAL COMPOSITE + TONE MAPPING + EXPOSURE + VIGNETTE
    vec3 sceneColor = texture(uSceneTex, vTexCoord).rgb;

    // Additive bloom overlay
    if (uEnableBloom > 0) {
        vec3 bloomColor = texture(uBloomTex, vTexCoord).rgb;
        sceneColor += bloomColor * uBloomIntensity;
    }

    // Exposure adjustment
    sceneColor *= uExposure;

    // ACES Filmic Tone Mapping
    if (uEnableToneMap > 0) {
        sceneColor = ACESFilm(sceneColor);
    }

    // Subtle space vignette
    if (uVignette > 0.0) {
        vec2 uv = (vTexCoord - 0.5) * 2.0;
        float dist = dot(uv, uv);
        float vignette = clamp(1.0 - dist * uVignette * 0.35, 0.0, 1.0);
        sceneColor *= vignette;
    }

    // Apply fade alpha (for cinematic startup sequence or transitions)
    sceneColor *= uFadeAlpha;

    FragColor = vec4(sceneColor, 1.0);
}
