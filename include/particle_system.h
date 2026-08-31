#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size = 0.0f;
    float life = 0.0f;
    float maxLife = 0.0f;
};

class SceneRenderer;

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    void init();
    void update(float deltaTime);
    void render(SceneRenderer& renderer, const glm::mat4& viewMat, const glm::mat4& projMat, bool showParticles);
    void cleanup();

    // Inspection & testing queries
    size_t getSolarFlareCount() const { return solarFlares.size(); }
    size_t getCometParticleCount() const { return cometParticles.size(); }
    const std::vector<Particle>& getSolarFlares() const { return solarFlares; }
    const std::vector<Particle>& getCometParticles() const { return cometParticles; }

private:
    std::vector<Particle> solarFlares;
    std::vector<Particle> cometParticles;
};
