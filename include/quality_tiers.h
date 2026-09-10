#pragma once

#include <cstdint>

// C3.8 — Quality Tiers & Performance Tuning.
//
// ONE authoritative quality configuration. All render systems (atmosphere,
// black-hole lensing, wormhole portal, Saturn/shadow filtering), the Settings
// UI, and the benchmark harness consume this table. Tier indices match the
// existing GraphicsQuality enum (0=Low, 1=Medium, 2=High, 3=Ultra).
// HIGH is the approved/reference gameplay tier and preserves C3.1–C3.7 visuals.

struct QualityTierSettings {
    int atmosphereSamples;   // Atmosphere 2.0 integration samples
    int blackHoleSteps;      // Bounded lensing integration steps
    int portalResolution;    // Wormhole portal target (square, px)
    int portalUpdateDivisor; // 1 = full-rate, 2 = every other eligible frame
    int shadowSamples;       // C3.7 soft-shadow filter taps
};

namespace QualityTiers {

constexpr int kLow = 0;
constexpr int kMedium = 1;
constexpr int kHigh = 2;
constexpr int kUltra = 3;
constexpr int kCount = 4;

// Canonical C3.8 tier matrix.
constexpr QualityTierSettings kTable[kCount] = {
    // atmo, BH steps, portal px, portal divisor, shadow taps
    {  6,  8, 256, 2, 1 }, // Low:    cheap, bounded, half-rate portal
    {  8, 16, 384, 1, 2 }, // Medium: full-rate portal
    { 12, 24, 512, 1, 4 }, // High:   approved reference gameplay tier
    { 16, 32, 512, 1, 8 }, // Ultra:  full-rate portal, max sampling
};

constexpr bool isValidTier(int tier) {
    return tier >= kLow && tier <= kUltra;
}

// Invalid tiers fall back deterministically to High (reference tier).
constexpr QualityTierSettings get(int tier) {
    return isValidTier(tier) ? kTable[tier] : kTable[kHigh];
}

// Low-tier half-rate portal schedule (pure; unit-tested without GL).
// Never skips when there is no valid content yet (first eligible frame must
// render so no uninitialized texture is ever displayed), and never skips at
// full-rate divisors. Frame indices start at 1 for the first eligible frame.
constexpr bool shouldSkipPortalFrame(uint64_t eligibleFrameIndex, int divisor, bool hasValidContent) {
    if (!hasValidContent) return false;
    if (divisor <= 1) return false;
    return (eligibleFrameIndex % static_cast<uint64_t>(divisor)) != 0u;
}

} // namespace QualityTiers
