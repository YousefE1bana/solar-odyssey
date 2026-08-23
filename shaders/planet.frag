#version 450 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vSunDir;
in vec3 vLocalPos;
in vec3 vLocalSunDir;

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

// Analytical Ring & Eclipse Shadow Uniforms
uniform int uHasRings;          // 1 if planet has rings casting shadows on itself
uniform float uRingInnerRadius; // Inner ring radius in local units
uniform float uRingOuterRadius; // Outer ring radius in local units
uniform int uIsRing;            // 1 if rendering ring geometry (planet casts shadow on ring)
uniform float uPlanetRadius;    // Planet sphere radius in local ring units
uniform int uHasEclipse;        // 1 if moon eclipse shadow is active
uniform vec3 uEclipseLocalPos;  // Moon position in local space
uniform float uEclipseRadius;   // Moon radius in local space

// 1. Ring Shadow cast onto Planet Globe
float calculateRingShadowOnPlanet(vec3 localPos, vec3 sunDir) {
    if (uHasRings == 0) return 1.0;

    // Ray from surface point localPos towards Sun: R(t) = localPos + t * sunDir
    // Ring plane is y = 0
    if (abs(sunDir.y) < 1e-5) return 1.0; // Parallel to ring plane
    
    float t = -localPos.y / sunDir.y;
    if (t <= 0.0) return 1.0; // Ring plane is behind the surface point relative to sun

    vec3 hitPoint = localPos + t * sunDir;
    float dist = length(hitPoint.xz);

    if (dist >= uRingInnerRadius && dist <= uRingOuterRadius) {
        float normDist = (dist - uRingInnerRadius) / (uRingOuterRadius - uRingInnerRadius);
        
        // Cassini division gap at ~0.76 normalized radius
        float cassiniGap = smoothstep(0.02, 0.0, abs(normDist - 0.76));
        float ringDensity = (1.0 - cassiniGap * 0.75) * 0.90;
        
        // Soft edges at ring inner and outer boundaries
        float edgeFade = smoothstep(uRingInnerRadius, uRingInnerRadius + 0.04, dist) *
                         (1.0 - smoothstep(uRingOuterRadius - 0.04, uRingOuterRadius, dist));
        
        return 1.0 - ringDensity * edgeFade;
    }
    return 1.0;
}

// 2. Planet Sphere Shadow cast onto Ring Geometry
float calculatePlanetShadowOnRing(vec3 localPos, vec3 sunDir) {
    if (uIsRing == 0) return 1.0;

    // Ray from ring localPos towards Sun: R(t) = localPos + t * sunDir
    // Planet is a sphere of radius uPlanetRadius at (0,0,0)
    float b = dot(localPos, sunDir);
    float c = dot(localPos, localPos) - uPlanetRadius * uPlanetRadius;
    
    if (b > 0.0) return 1.0; // Ring point is facing the sun, planet cannot occlude

    float discr = b * b - c;
    if (discr > 0.0) {
        float dPerp = sqrt(max(0.0, dot(localPos, localPos) - b * b));
        // Soft penumbra edge
        float shadow = smoothstep(uPlanetRadius * 0.96, uPlanetRadius * 1.03, dPerp);
        return shadow;
    }
    return 1.0;
}

// 3. Moon Eclipse Shadow cast onto Planet Globe
float calculateEclipseShadow(vec3 localPos, vec3 sunDir) {
    if (uHasEclipse == 0) return 1.0;

    // Vector from localPos to occluder moon center
    vec3 toMoon = uEclipseLocalPos - localPos;
    float t = dot(toMoon, sunDir);
    if (t <= 0.0) return 1.0; // Moon is behind the planet surface point

    // Perpendicular distance from moon center to the light ray
    float dSq = dot(toMoon, toMoon) - t * t;
    float r = uEclipseRadius;
    float rSq = r * r;

    if (dSq < rSq * 2.25) {
        float d = sqrt(max(0.0, dSq));
        // Umbra (total) and Penumbra (partial)
        float umbraRadius = r * 0.70;
        float penumbraRadius = r * 1.30;
        float shadow = smoothstep(umbraRadius, penumbraRadius, d);
        return mix(0.05, 1.0, shadow);
    }
    return 1.0;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vViewDir);
    vec3 L = normalize(vSunDir);
    vec3 localSun = normalize(vLocalSunDir);

    // Illumination dot product with sun
    float NdotL = dot(N, L);

    // Soft wrap diffuse for atmospheric planetary surfaces
    float dayFactor = smoothstep(-0.16, 0.22, NdotL);
    float diffuse = max((NdotL + 0.12) / 1.12, 0.0);

    // Analytical Shadows (Ring on planet, Planet on ring, Moon eclipse on planet)
    float ringShadow = calculateRingShadowOnPlanet(vLocalPos, localSun);
    float planetOnRingShadow = calculatePlanetShadowOnRing(vLocalPos, localSun);
    float eclipseShadow = calculateEclipseShadow(vLocalPos, localSun);
    float totalShadow = ringShadow * planetOnRingShadow * eclipseShadow;

    // Sample daytime surface color
    vec4 dayColor = texture(uDayTex, vTexCoord);
    vec3 surfaceColor = dayColor.rgb;

    // For rings, alpha is in texture or alpha channel
    float alpha = (uIsRing > 0) ? dayColor.a : 1.0;

    // Handle Night Texture (City Lights on Earth)
    if (uHasNightTex > 0) {
        vec4 nightColor = texture(uNightTex, vTexCoord);
        float nightFactor = (1.0 - dayFactor);
        vec3 cityLights = nightColor.rgb * vec3(1.15, 1.05, 0.85) * nightFactor * 1.5;

        vec3 ambient = dayColor.rgb * vec3(0.04, 0.04, 0.06);
        vec3 litDay = dayColor.rgb * (diffuse * vec3(1.0, 0.98, 0.92) * uSunIntensity * totalShadow);
        surfaceColor = litDay + ambient + cityLights;
    } else {
        // Standard celestial body lighting with soft ambient floor
        vec3 ambient = surfaceColor * vec3(0.10, 0.10, 0.12);
        vec3 litDay = surfaceColor * (diffuse * vec3(1.05, 1.02, 0.96) * uSunIntensity * totalShadow);
        surfaceColor = litDay + ambient;
    }

    // Specular highlight for oceans/water (Earth only)
    if (uSpecularStrength > 0.0 && diffuse > 0.0) {
        vec3 H = normalize(L + V);
        float NdotH = max(dot(N, H), 0.0);
        float spec = pow(NdotH, 24.0);
        float isWater = max(0.0, dayColor.b - (dayColor.r + dayColor.g) * 0.45);
        vec3 specular = vec3(0.9, 0.95, 1.0) * spec * isWater * uSpecularStrength * diffuse * totalShadow;
        surfaceColor += specular;
    }

    // Clouds Overlay (Earth)
    if (uHasClouds == 1) {
        vec2 cloudUV = vTexCoord + uCloudOffset;
        vec4 cloudTex = texture(uCloudsTex, cloudUV);
        float cloudAlpha = cloudTex.r;

        vec3 cloudColor = vec3(0.98, 0.98, 1.0) * (diffuse * 1.1 * totalShadow + 0.05);
        float cloudShadow = (1.0 - cloudAlpha * 0.35 * smoothstep(0.0, 0.3, diffuse));
        surfaceColor *= cloudShadow;
        surfaceColor = mix(surfaceColor, cloudColor, cloudAlpha * dayFactor * 0.92 * totalShadow);
    }

    // Rayleigh / Fresnel Limb Glow
    if (uAtmosphereGlow > 0.0) {
        float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
        float sunlitRim = max(NdotL + 0.25, 0.0);
        vec3 atmoGlow = uAtmosphereColor * (fresnel * uAtmosphereGlow * 1.3 * sunlitRim * totalShadow);
        surfaceColor += atmoGlow;
    }

    // Add base emissive term
    surfaceColor += uEmissive;

    FragColor = vec4(surfaceColor, alpha);
}
