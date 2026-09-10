#include "catch.hpp"
#include "quality_tiers.h"
#include "atmosphere_effects.h"
#include "black_hole.h"
#include "wormhole_portal_renderer.h"
#include <cstdint>

// C3.8 — Quality Tiers & Performance Tuning.
// The expected matrix values below are the CANONICAL C3.8 SPEC oracle, not
// copies of implementation literals: any drift between this spec and the
// authoritative table must fail loudly here.

TEST_CASE("C3.8 canonical tier matrix values", "[quality_tiers]") {
    REQUIRE(QualityTiers::kCount == 4);

    const QualityTierSettings low = QualityTiers::get(QualityTiers::kLow);
    REQUIRE(low.atmosphereSamples == 6);
    REQUIRE(low.blackHoleSteps == 8);
    REQUIRE(low.portalResolution == 256);
    REQUIRE(low.portalUpdateDivisor == 2);
    REQUIRE(low.shadowSamples == 1);

    const QualityTierSettings med = QualityTiers::get(QualityTiers::kMedium);
    REQUIRE(med.atmosphereSamples == 8);
    REQUIRE(med.blackHoleSteps == 16);
    REQUIRE(med.portalResolution == 384);
    REQUIRE(med.portalUpdateDivisor == 1);
    REQUIRE(med.shadowSamples == 2);

    const QualityTierSettings high = QualityTiers::get(QualityTiers::kHigh);
    REQUIRE(high.atmosphereSamples == 12);
    REQUIRE(high.blackHoleSteps == 24);
    REQUIRE(high.portalResolution == 512);
    REQUIRE(high.portalUpdateDivisor == 1);
    REQUIRE(high.shadowSamples == 4);

    const QualityTierSettings ultra = QualityTiers::get(QualityTiers::kUltra);
    REQUIRE(ultra.atmosphereSamples == 16);
    REQUIRE(ultra.blackHoleSteps == 32);
    REQUIRE(ultra.portalResolution == 512);
    REQUIRE(ultra.portalUpdateDivisor == 1);
    REQUIRE(ultra.shadowSamples == 8);
}

TEST_CASE("C3.8 invalid tier falls back deterministically to High", "[quality_tiers]") {
    for (int bad : {-100, -1, 4, 5, 99}) {
        REQUIRE(!QualityTiers::isValidTier(bad));
        const QualityTierSettings f = QualityTiers::get(bad);
        const QualityTierSettings high = QualityTiers::get(QualityTiers::kHigh);
        REQUIRE(f.atmosphereSamples == high.atmosphereSamples);
        REQUIRE(f.blackHoleSteps == high.blackHoleSteps);
        REQUIRE(f.portalResolution == high.portalResolution);
        REQUIRE(f.portalUpdateDivisor == high.portalUpdateDivisor);
        REQUIRE(f.shadowSamples == high.shadowSamples);
    }
    for (int good = 0; good < QualityTiers::kCount; ++good) {
        REQUIRE(QualityTiers::isValidTier(good));
    }
}

TEST_CASE("C3.8 tier values respect shader-side finite bounds", "[quality_tiers]") {
    // atmosphere.frag: samples = clamp(uSampleCount, 4, 16)
    // black_hole_lensing.frag: steps = max(uMaxSteps, 4)
    // planet.frag: taps = clamp(uShadowSamples, 1, 8)
    for (int t = 0; t < QualityTiers::kCount; ++t) {
        const QualityTierSettings q = QualityTiers::get(t);
        REQUIRE(q.atmosphereSamples >= 4);
        REQUIRE(q.atmosphereSamples <= 16);
        REQUIRE(q.blackHoleSteps >= 4);
        REQUIRE(q.blackHoleSteps <= 32);
        REQUIRE(q.shadowSamples >= 1);
        REQUIRE(q.shadowSamples <= 8);
        // Portal targets are fixed canonical squares, never window-sized.
        const bool resOk = (q.portalResolution == 256 || q.portalResolution == 384 ||
                            q.portalResolution == 512);
        REQUIRE(resOk);
        const bool divOk = (q.portalUpdateDivisor == 1 || q.portalUpdateDivisor == 2);
        REQUIRE(divOk);
    }
    // Low BH mode is genuinely reduced but still bounded and safe.
    REQUIRE(QualityTiers::get(QualityTiers::kLow).blackHoleSteps <= 8);
    REQUIRE(QualityTiers::get(QualityTiers::kLow).blackHoleSteps >= 4);
}

TEST_CASE("C3.8 High tier preserves approved reference features", "[quality_tiers]") {
    const QualityTierSettings high = QualityTiers::get(QualityTiers::kHigh);
    REQUIRE(high.atmosphereSamples == 12); // C3.1-approved default
    REQUIRE(high.blackHoleSteps == 24);    // C3.4-approved default
    REQUIRE(high.portalResolution == 512); // C3.5-approved target
    REQUIRE(high.portalUpdateDivisor == 1);// full-rate portal compositing (C3.6)
    REQUIRE(high.shadowSamples == 4);      // canonical High filter width
}

TEST_CASE("C3.8 atmosphere sample scaling applies", "[quality_tiers]") {
    AtmosphereEffects atmo;
    REQUIRE(atmo.getQualitySamples() == 12); // approved High default
    for (int t = 0; t < QualityTiers::kCount; ++t) {
        atmo.setQualitySamples(QualityTiers::get(t).atmosphereSamples);
        REQUIRE(atmo.getQualitySamples() == QualityTiers::get(t).atmosphereSamples);
    }
    // Runtime tier transitions converge back to High exactly.
    atmo.setQualitySamples(QualityTiers::get(QualityTiers::kLow).atmosphereSamples);
    REQUIRE(atmo.getQualitySamples() == 6);
    atmo.setQualitySamples(QualityTiers::get(QualityTiers::kHigh).atmosphereSamples);
    REQUIRE(atmo.getQualitySamples() == 12);
}

TEST_CASE("C3.8 black-hole step scaling applies with safe clamps", "[quality_tiers]") {
    BlackHole bh;
    REQUIRE(bh.getLensingSteps() == 24); // approved High default
    for (int t = 0; t < QualityTiers::kCount; ++t) {
        bh.setLensingSteps(QualityTiers::get(t).blackHoleSteps);
        REQUIRE(bh.getLensingSteps() == QualityTiers::get(t).blackHoleSteps);
    }
    // Degenerate inputs stay finite and bounded, never below the shader floor.
    bh.setLensingSteps(0);
    REQUIRE(bh.getLensingSteps() == 4);
    bh.setLensingSteps(-50);
    REQUIRE(bh.getLensingSteps() == 4);
    bh.setLensingSteps(100000);
    REQUIRE(bh.getLensingSteps() == 64);
}

TEST_CASE("C3.8 portal divisor setting is GL-free and validated", "[quality_tiers]") {
    WormholePortalRenderer pr;
    REQUIRE(pr.portalUpdateDivisor == 1);
    pr.setPortalUpdateDivisor(2);
    REQUIRE(pr.portalUpdateDivisor == 2);
    pr.setPortalUpdateDivisor(0);
    REQUIRE(pr.portalUpdateDivisor == 1);
    pr.setPortalUpdateDivisor(-7);
    REQUIRE(pr.portalUpdateDivisor == 1);
    REQUIRE(pr.portalContentValid == false);
    REQUIRE(pr.portalEligibleFrameIndex == 0u);
}

TEST_CASE("C3.8 Low half-rate portal schedule", "[quality_tiers]") {
    using QualityTiers::shouldSkipPortalFrame;
    const int lowDiv = QualityTiers::get(QualityTiers::kLow).portalUpdateDivisor;
    REQUIRE(lowDiv == 2);

    // No valid content yet: first eligible frames must render (never uninitialized).
    REQUIRE(shouldSkipPortalFrame(1, lowDiv, false) == false);
    REQUIRE(shouldSkipPortalFrame(2, lowDiv, false) == false);

    // Steady state with valid content: render on even eligible frames only.
    REQUIRE(shouldSkipPortalFrame(2, lowDiv, true) == false);
    REQUIRE(shouldSkipPortalFrame(3, lowDiv, true) == true);
    REQUIRE(shouldSkipPortalFrame(4, lowDiv, true) == false);
    REQUIRE(shouldSkipPortalFrame(5, lowDiv, true) == true);
    REQUIRE(shouldSkipPortalFrame(100, lowDiv, true) == false);
    REQUIRE(shouldSkipPortalFrame(101, lowDiv, true) == true);

    // High/Ultra full-rate: never skip once valid.
    for (int t : {QualityTiers::kMedium, QualityTiers::kHigh, QualityTiers::kUltra}) {
        const int div = QualityTiers::get(t).portalUpdateDivisor;
        REQUIRE(div == 1);
        for (uint64_t f = 1; f <= 6; ++f) {
            REQUIRE(shouldSkipPortalFrame(f, div, true) == false);
        }
    }

    // Degenerate divisors behave as full-rate (fail operational, never blank).
    REQUIRE(shouldSkipPortalFrame(3, 0, true) == false);
    REQUIRE(shouldSkipPortalFrame(3, -2, true) == false);
}

TEST_CASE("C3.8 runtime tier transition sequence", "[quality_tiers]") {
    // Simulates Low -> Ultra -> invalid -> High without GL.
    AtmosphereEffects atmo;
    BlackHole bh;
    WormholePortalRenderer pr;

    auto apply = [&](int tier) {
        const QualityTierSettings q = QualityTiers::get(tier);
        atmo.setQualitySamples(q.atmosphereSamples);
        bh.setLensingSteps(q.blackHoleSteps);
        pr.setPortalUpdateDivisor(q.portalUpdateDivisor);
    };

    apply(QualityTiers::kLow);
    REQUIRE(atmo.getQualitySamples() == 6);
    REQUIRE(bh.getLensingSteps() == 8);
    REQUIRE(pr.portalUpdateDivisor == 2);

    apply(QualityTiers::kUltra);
    REQUIRE(atmo.getQualitySamples() == 16);
    REQUIRE(bh.getLensingSteps() == 32);
    REQUIRE(pr.portalUpdateDivisor == 1);

    apply(42); // invalid -> deterministic High
    REQUIRE(atmo.getQualitySamples() == 12);
    REQUIRE(bh.getLensingSteps() == 24);
    REQUIRE(pr.portalUpdateDivisor == 1);
}
