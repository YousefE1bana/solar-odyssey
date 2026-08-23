#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class AsteroidBelt {
public:
    struct Asteroid {
        glm::vec3 position;       // Current 3D position
        glm::vec3 rotation;       // Current rotation angles (euler)
        glm::vec3 rotationSpeed;  // Angular velocity
        float size;               // Geometric radius
        float orbitRadius;        // Semi-major axis
        float orbitSpeed;         // Angular speed
        float orbitOffset;        // Initial phase angle (degrees)
        float orbitInclination;   // Orbital tilt (radians)
        float eccentricity;       // Slight non-circular orbit
        glm::vec3 materialColor;  // Carbonaceous / silicate tint
        float brightness;         // Albedo variation
    };

private:
    std::vector<Asteroid> allAsteroids;
    int activeCount = 500;
    float innerRadius = 13.5f;
    float outerRadius = 15.8f;
    GLuint asteroidTexture = 0;

    void generateAsteroids(int totalCapacity);
    void loadTexture(const char* texturePath);

public:
    AsteroidBelt(int count = 500, float inR = 13.5f, float outR = 15.8f, const char* texturePath = "Textures/moon.jpg");
    ~AsteroidBelt();

    void setQualityCount(int count);
    int getAsteroidCount() const;
    void update(float deltaTime, float planetSpeed, const glm::vec3& blackHolePos = glm::vec3(0.0f), float blackHoleStrength = 0.0f);
    void render(float focusFade = 1.0f);
};
