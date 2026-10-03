#include "particle_system.h"
#include "scene_renderer.h"
#include <GL/glew.h>
#include <cstdlib>
#include <cmath>
ParticleSystem::ParticleSystem() {
}

ParticleSystem::~ParticleSystem() {
    cleanup();
}

void ParticleSystem::init() {
    // Solar Flares
    solarFlares.clear();
    solarFlares.reserve(350);
    for (int i = 0; i < 350; ++i) {
        Particle flare;
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f;
        float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
        flare.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
        flare.velocity = glm::normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
        flare.color = glm::vec3(1.0f, 0.65f + static_cast<float>(rand()) / RAND_MAX * 0.35f, 0.1f);
        flare.size = 0.06f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
        flare.life = 0.05f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
        flare.maxLife = flare.life;
        solarFlares.push_back(flare);
    }

    // Comets
    cometParticles.clear();
    cometParticles.reserve(240);
    for (int i = 0; i < 240; ++i) {
        Particle particle;
        float t = static_cast<float>(rand()) / RAND_MAX;
        float angle = t * 2.0f * 3.14159265358979323846f;
        float distance = 36.0f;
        particle.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
        particle.velocity = glm::vec3(-sin(angle), 0, cos(angle)) * 0.12f;
        particle.color = glm::vec3(0.75f, 0.85f, 1.0f);
        particle.size = 0.04f + static_cast<float>(rand()) / RAND_MAX * 0.06f;
        particle.life = t * 10.0f;
        particle.maxLife = 10.0f;
        cometParticles.push_back(particle);
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto &flare : solarFlares) {
        flare.position += flare.velocity * deltaTime * 12.0f;
        flare.life -= deltaTime;
        if (flare.life <= 0.0f) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846f;
            float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            flare.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
            flare.velocity = glm::normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
            flare.life = 0.05f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
            flare.maxLife = flare.life;
        }
    }

    for (auto &particle : cometParticles) {
        particle.position += particle.velocity * deltaTime * 6.0f;
        particle.life -= deltaTime;
        if (particle.life <= 0.0f) {
            float angle = atan2(particle.position.z, particle.position.x) + 0.08f;
            float distance = 36.0f;
            particle.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
            particle.velocity = glm::vec3(-sin(angle), 0, cos(angle)) * 0.12f;
            particle.life = particle.maxLife;
        }
    }
}

void ParticleSystem::render(SceneRenderer& renderer, const glm::mat4& viewMat, const glm::mat4& projMat, bool showParticles) {
    if (!showParticles) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    if (!renderer.batch.isReady()) renderer.batch.init(kFlatVS, kFlatFS);

    // Solar Flares
    if (!solarFlares.empty()) {
        renderer.batch.begin(GL_POINTS, projMat, viewMat, 1.0f);
        for (const auto &p : solarFlares) {
            float alpha = (p.maxLife > 0.0f) ? (p.life / p.maxLife) : 1.0f;
            renderer.batch.vertex(p.position, glm::vec4(p.color, alpha * 0.8f), 1.5f);
        }
        renderer.batch.end();
    }

    // Comets
    if (!cometParticles.empty()) {
        renderer.batch.begin(GL_POINTS, projMat, viewMat, 1.0f);
        for (const auto &p : cometParticles) {
            float alpha = (p.maxLife > 0.0f) ? (p.life / p.maxLife) : 1.0f;
            renderer.batch.vertex(p.position, glm::vec4(p.color, alpha * 0.7f), 1.0f);
        }
        renderer.batch.end();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::cleanup() {
    solarFlares.clear();
    cometParticles.clear();
}
