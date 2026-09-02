#version 450 core

in vec3 vLocalPos;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vSunDir;
in vec3 vEyePos;

out vec4 FragColor;

uniform vec3 uAtmoColor;
uniform float uDensity;
uniform float uGlowIntensity;
uniform vec3 uRayleighCoeff;
uniform float uMieCoeff;
uniform float uMieG;
uniform float uRayleighScaleH;
uniform float uMieScaleH;
uniform int uSampleCount;
uniform float uPlanetRadius;
uniform float uAtmoRadius;

const float PI = 3.14159265359;

// Rayleigh Phase Function: P_R(theta) = 3 / (16 * PI) * (1 + cos^2(theta))
float rayleighPhase(float cosTheta) {
    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
}

// Henyey-Greenstein Mie Phase Function: P_M(theta, g) = (1 - g^2) / (4 * PI * (1 + g^2 - 2g*cosTheta)^1.5)
float miePhase(float cosTheta, float g) {
    float g2 = g * g;
    float denom = 1.0 + g2 - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * ((1.0 - g2) / max(pow(denom, 1.5), 1e-4));
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    vec3 L = normalize(vSunDir);

    float cosTheta = dot(-V, L);
    float pR = rayleighPhase(cosTheta);
    float pM = miePhase(cosTheta, uMieG);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = dot(N, L);

    // Rim optical depth falloff
    float rim = pow(1.0 - NdotV, 3.2);

    // Sunlit forward / twilight wrap
    float sunlit = max(NdotL * 0.75 + 0.25, 0.02);

    // Single-scattering numerical approximation along view ray
    int samples = clamp(uSampleCount, 4, 16);
    float stepSize = 1.0 / float(samples);
    
    vec3 rayleighAccum = vec3(0.0);
    float mieAccum = 0.0;
    
    for (int i = 0; i < samples; ++i) {
        float t = (float(i) + 0.5) * stepSize;
        float h = t; // normalized height in shell [0, 1]
        
        float dR = exp(-h / max(uRayleighScaleH * 15.0, 0.01)) * stepSize;
        float dM = exp(-h / max(uMieScaleH * 15.0, 0.01)) * stepSize;
        
        rayleighAccum += uRayleighCoeff * dR;
        mieAccum += uMieCoeff * dM;
    }

    vec3 inScattered = (rayleighAccum * pR * 12.0 + vec3(mieAccum * pM * 10.0)) * uDensity;
    
    // Combine with base atmospheric tint for vibrancy
    vec3 finalColor = (inScattered + uAtmoColor * (rim * 0.8 + 0.2)) * sunlit * uGlowIntensity * 1.5;
    float alpha = clamp(rim * uDensity * sunlit * 1.2 + length(inScattered) * 0.4, 0.0, 1.0);

    FragColor = vec4(finalColor, alpha);
}
