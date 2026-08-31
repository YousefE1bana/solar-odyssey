#version 450 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPos;
in vec3 vViewDir;
in vec3 vSunDir;
in vec4 vInstanceColor;

out vec4 FragColor;

uniform sampler2D uDayTex;
uniform float uSunIntensity;

void main() {
    vec4 dayColor = texture(uDayTex, vTexCoord);
    vec3 surfaceColor = dayColor.rgb * vInstanceColor.rgb * vInstanceColor.w;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(vSunDir);

    float NdotL = dot(N, L);
    float diffuse = max((NdotL + 0.12) / 1.12, 0.0);

    vec3 ambient = surfaceColor * vec3(0.10, 0.10, 0.12);
    vec3 litDay = surfaceColor * (diffuse * vec3(1.05, 1.02, 0.96) * uSunIntensity);

    vec3 finalColor = litDay + ambient;
    FragColor = vec4(finalColor, 1.0);
}
