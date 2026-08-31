#include "catch.hpp"
#include "particle_system.h"

TEST_CASE("ParticleSystem - Initialization and Pool Bounds", "[particles]") {
    ParticleSystem particleSys;
    REQUIRE(particleSys.getSolarFlareCount() == 0);
    REQUIRE(particleSys.getCometParticleCount() == 0);

    particleSys.init();
    REQUIRE(particleSys.getSolarFlareCount() == 350);
    REQUIRE(particleSys.getCometParticleCount() == 240);

    const auto& flares = particleSys.getSolarFlares();
    for (const auto& flare : flares) {
        REQUIRE(flare.life > 0.0f);
        REQUIRE(flare.maxLife > 0.0f);
        REQUIRE(flare.size >= 0.05f);
        REQUIRE(flare.size <= 0.15f);
        // Solar flares must spawn within radial envelope around origin
        float dist = glm::length(glm::vec2(flare.position.x, flare.position.z));
        REQUIRE(dist >= 1.9f);
        REQUIRE(dist <= 2.5f);
    }

    const auto& comets = particleSys.getCometParticles();
    for (const auto& comet : comets) {
        REQUIRE(comet.maxLife == 10.0f);
        float dist = glm::length(glm::vec2(comet.position.x, comet.position.z));
        REQUIRE(dist >= 35.9f);
        REQUIRE(dist <= 36.1f);
    }
}

TEST_CASE("ParticleSystem - Update and Regeneration Mechanics", "[particles]") {
    ParticleSystem particleSys;
    particleSys.init();

    // Fast-forward simulation by 2.5 seconds to cause all initial solar flares to expire & regenerate
    particleSys.update(2.5f);
    REQUIRE(particleSys.getSolarFlareCount() == 350);

    const auto& flares = particleSys.getSolarFlares();
    for (const auto& flare : flares) {
        REQUIRE(flare.life > 0.0f);
    }

    // Fast-forward simulation by 11.0 seconds to cause all comet particles to cycle
    particleSys.update(11.0f);
    REQUIRE(particleSys.getCometParticleCount() == 240);
    const auto& comets = particleSys.getCometParticles();
    for (const auto& comet : comets) {
        REQUIRE(comet.life > 0.0f);
    }

    particleSys.cleanup();
    REQUIRE(particleSys.getSolarFlareCount() == 0);
    REQUIRE(particleSys.getCometParticleCount() == 0);
}
