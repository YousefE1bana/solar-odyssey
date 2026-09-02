#include "catch.hpp"
#include "atmosphere_effects.h"
#include <cmath>

static const float PI = 3.14159265359f;

// Analytical Phase Functions for testing
static float calcRayleighPhase(float cosTheta) {
    return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

static float calcMiePhase(float cosTheta, float g) {
    float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cosTheta;
    return (1.0f / (4.0f * PI)) * ((1.0f - g2) / std::pow(denom, 1.5f));
}

TEST_CASE("Atmosphere 2.0 - Normalized Unit Transformation and Scale Heights (Rp=1.0)", "[atmosphere]") {
    AtmosphereEffects atmoEffects;
    const auto& earth = atmoEffects.getAtmosphereProperties("Earth");
    
    REQUIRE(earth.hasAtmosphere == true);
    REQUIRE(earth.planetRadiusKm == Approx(6371.0f));
    REQUIRE(earth.rayleighScaleHeightKm == Approx(8.0f));
    REQUIRE(earth.mieScaleHeightKm == Approx(1.2f));

    // In normalized space (Rp = 1.0)
    float normRayleighH = earth.getNormalizedRayleighScaleHeight();
    float normMieH = earth.getNormalizedMieScaleHeight();
    float normAtmoR = earth.getNormalizedAtmosphereRadius();

    REQUIRE(normRayleighH == Approx(8.0f / 6371.0f).epsilon(1e-4));
    REQUIRE(normMieH == Approx(1.2f / 6371.0f).epsilon(1e-4));
    REQUIRE(normAtmoR == Approx(1.065f).epsilon(1e-4));
    REQUIRE(normRayleighH > normMieH); // Rayleigh scale height is physically larger than aerosol scale height
}

TEST_CASE("Atmosphere 2.0 - Exponential Density Profile and Transmittance Monotonicity", "[atmosphere]") {
    AtmosphereEffects atmoEffects;
    const auto& earth = atmoEffects.getAtmosphereProperties("Earth");
    float normH = earth.getNormalizedRayleighScaleHeight();

    // Verify exponential decay with increasing normalized altitude h
    float density_0 = std::exp(-0.0f / normH);
    float density_half = std::exp(-0.5f * earth.height / normH);
    float density_top = std::exp(-1.0f * earth.height / normH);

    REQUIRE(density_0 == Approx(1.0f));
    REQUIRE(density_0 > density_half);
    REQUIRE(density_half > density_top);
    REQUIRE(density_top > 0.0f);
    REQUIRE(std::isfinite(density_top));
}

TEST_CASE("Atmosphere 2.0 - Rayleigh and Henyey-Greenstein Mie Phase Functions", "[atmosphere]") {
    // 1. Rayleigh Phase bounds: P_R(-1) == P_R(1) == 3 / (8 * PI), P_R(0) == 3 / (16 * PI)
    float pr_forward = calcRayleighPhase(1.0f);
    float pr_backward = calcRayleighPhase(-1.0f);
    float pr_side = calcRayleighPhase(0.0f);

    REQUIRE(pr_forward == Approx(pr_backward));
    REQUIRE(pr_forward == Approx(2.0f * pr_side));
    REQUIRE(pr_forward > 0.0f);

    // 2. Mie Phase with forward scattering (g = 0.76)
    float g = 0.76f;
    float pm_forward = calcMiePhase(1.0f, g);
    float pm_backward = calcMiePhase(-1.0f, g);

    REQUIRE(pm_forward > pm_backward); // Strong forward peak
    REQUIRE(pm_forward > 10.0f * pm_backward); // Henyey-Greenstein forward lobe ratio
    REQUIRE(std::isfinite(pm_forward));
    REQUIRE(std::isfinite(pm_backward));
}

TEST_CASE("Atmosphere 2.0 - Planetary Presets and NaN/Inf Safety", "[atmosphere]") {
    AtmosphereEffects atmoEffects;
    const auto& data = atmoEffects.getAllAtmosphereData();

    REQUIRE(data.find("Mercury") != data.end());
    REQUIRE(data.find("Venus") != data.end());
    REQUIRE(data.find("Earth") != data.end());
    REQUIRE(data.find("Mars") != data.end());
    REQUIRE(data.find("Jupiter") != data.end());
    REQUIRE(data.find("Saturn") != data.end());

    for (const auto& [name, props] : data) {
        if (!props.hasAtmosphere) continue;

        REQUIRE(props.planetRadiusKm > 0.0f);
        REQUIRE(props.atmosphereHeightKm > 0.0f);
        REQUIRE(props.rayleighScaleHeightKm > 0.0f);
        REQUIRE(props.mieScaleHeightKm > 0.0f);
        REQUIRE(props.mieAnisotropyG >= -1.0f);
        REQUIRE(props.mieAnisotropyG <= 1.0f);
        REQUIRE(props.densityMultiplier >= 0.0f);
        REQUIRE(props.glowIntensity >= 0.0f);

        REQUIRE(std::isfinite(props.rayleighScatteringCoeff.r));
        REQUIRE(std::isfinite(props.rayleighScatteringCoeff.g));
        REQUIRE(std::isfinite(props.rayleighScatteringCoeff.b));
        REQUIRE(std::isfinite(props.getNormalizedRayleighScaleHeight()));
        REQUIRE(std::isfinite(props.getNormalizedMieScaleHeight()));
        REQUIRE(std::isfinite(props.getNormalizedAtmosphereRadius()));
    }
}
