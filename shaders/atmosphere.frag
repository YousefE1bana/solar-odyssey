#version 330 core

in vec3 vNormal;
in vec3 vViewDir;
in vec3 vSunDir;

out vec4 FragColor;

uniform vec3 uAtmoColor;        // Base atmosphere color (e.g. Earth cyan/blue, Venus amber, Mars terracotta)
uniform float uDensity;         // Density / opacity factor (0..1)
uniform float uScatteringPower; // Rim falloff power (e.g. 2.0 to 4.5)
uniform float uGlowIntensity;   // Overall brightness multiplier

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    vec3 L = normalize(vSunDir);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = dot(N, L);

    // Rim / edge intensity (fresnel)
    float rim = pow(1.0 - NdotV, uScatteringPower);

    // Sunlit forward scattering boost (stronger on sun-facing limb)
    float sunlit = max(NdotL * 0.7 + 0.3, 0.05);

    // Color gradient from thin outer rim to dense horizon
    vec3 glowColor = uAtmoColor * (1.0 + rim * 0.5) * sunlit * uGlowIntensity;
    float alpha = rim * uDensity * sunlit;

    FragColor = vec4(glowColor, clamp(alpha, 0.0, 1.0));
}
