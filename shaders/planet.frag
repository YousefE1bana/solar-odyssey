#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vSunDir;

out vec4 FragColor;

uniform sampler2D uDayTex;
uniform sampler2D uNightTex;
uniform sampler2D uCloudsTex;

uniform int uHasNightTex;       // 1 if planet has night texture (Earth)
uniform int uHasClouds;         // 1 if clouds (Earth), 2 if Jupiter procedural bands
uniform vec2 uCloudOffset;      // Animated independent cloud UV offset
uniform vec3 uEmissive;         // Base emissive color
uniform float uSunIntensity;    // Sun light strength (default 1.0)
uniform vec3 uAtmosphereColor;  // Rim atmospheric scattering tint
uniform float uAtmosphereGlow;  // Atmosphere rim strength (0.0 .. 1.0)
uniform float uSpecularStrength;// Specular reflection strength (oceans)
uniform float uTime;            // Time for subtle animated effects

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    vec3 L = normalize(vSunDir);

    // Illumination dot product with sun
    float NdotL = dot(N, L);

    // Soft wrap diffuse for atmospheric planetary surfaces
    float dayFactor = smoothstep(-0.16, 0.22, NdotL);
    float diffuse = max((NdotL + 0.12) / 1.12, 0.0);

    // Sample daytime surface color
    vec4 dayColor = texture(uDayTex, vTexCoord);
    vec3 surfaceColor = dayColor.rgb;

    // Handle Night Texture (City Lights on Earth)
    if (uHasNightTex > 0) {
        vec4 nightColor = texture(uNightTex, vTexCoord);
        float nightFactor = (1.0 - dayFactor);
        vec3 cityLights = nightColor.rgb * vec3(1.15, 1.05, 0.85) * nightFactor * 1.5;

        vec3 ambient = dayColor.rgb * vec3(0.04, 0.04, 0.06);
        vec3 litDay = dayColor.rgb * (diffuse * vec3(1.0, 0.98, 0.92) * uSunIntensity);
        surfaceColor = litDay + ambient + cityLights;
    } else {
        // Standard celestial body lighting with soft ambient floor
        vec3 ambient = surfaceColor * vec3(0.10, 0.10, 0.12);
        vec3 litDay = surfaceColor * (diffuse * vec3(1.05, 1.02, 0.96) * uSunIntensity);
        surfaceColor = litDay + ambient;
    }

    // Specular highlight for oceans/water (Earth only)
    if (uSpecularStrength > 0.0 && diffuse > 0.0) {
        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);
        float spec = pow(NdotH, 24.0);
        float isWater = max(0.0, dayColor.b - (dayColor.r + dayColor.g) * 0.45);
        vec3 specular = vec3(0.9, 0.95, 1.0) * spec * isWater * uSpecularStrength * diffuse;
        surfaceColor += specular;
    }

    // Clouds Overlay (Earth)
    if (uHasClouds == 1) {
        vec2 cloudUV = vTexCoord + uCloudOffset;
        vec4 cloudTex = texture(uCloudsTex, cloudUV);
        float cloudAlpha = cloudTex.r;

        vec3 cloudColor = vec3(0.98, 0.98, 1.0) * (diffuse * 1.1 + 0.05);
        float cloudShadow = (1.0 - cloudAlpha * 0.35 * smoothstep(0.0, 0.3, diffuse));
        surfaceColor *= cloudShadow;
        surfaceColor = mix(surfaceColor, cloudColor, cloudAlpha * dayFactor * 0.92);
    }

    // Rayleigh / Fresnel Limb Glow
    if (uAtmosphereGlow > 0.0) {
        float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
        float sunlitRim = max(NdotL + 0.25, 0.0);
        vec3 atmoGlow = uAtmosphereColor * (fresnel * uAtmosphereGlow * 1.3 * sunlitRim);
        surfaceColor += atmoGlow;
    }

    // Add base emissive term
    surfaceColor += uEmissive;

    FragColor = vec4(surfaceColor, 1.0);
}
