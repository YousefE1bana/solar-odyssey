#include "spaceship.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

Spaceship::Spaceship() {
        resetToSpawnNearEarth();
    }

void Spaceship::resetToSpawnNearEarth() {
        position = glm::vec3(12.0f, 2.5f, 14.0f);
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        forward = glm::normalize(glm::vec3(-0.6f, 0.0f, -0.8f));
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        right = glm::normalize(glm::cross(forward, up));
        up = glm::normalize(glm::cross(right, forward));
        orientation = glm::quatLookAt(forward, up);

        throttle = 0.0f;
        currentThrottle = 0.0f;
        isBoosting = false;
        boostEnergy = maxBoostEnergy;
        flightMode = FLIGHT_MANUAL;
        cameraView = SHIP_CAM_CHASE;

        smoothCameraEye = position - forward * 6.5f + up * 2.2f;
        smoothCameraTarget = position + forward * 8.0f;
        smoothCameraUp = up;
        trails.clear();
    }

    // Mouse flight control parameters
    float mouseSensitivity = 0.0018f; // Radians per pixel

void Spaceship::toggleActive() {
        active = !active;
        if (active) {
            // Keep current velocity or initialize gentle forward glide
            if (glm::length(velocity) < 0.5f) {
                velocity = forward * 2.0f;
                throttle = 0.15f;
            }
        } else {
            flightMode = FLIGHT_MANUAL;
            warpSystem.cancelWarp();
            isBoosting = false;
        }
    }

void Spaceship::cycleCameraMode() {
        if (cameraView == SHIP_CAM_CHASE) cameraView = SHIP_CAM_CLOSE;
        else if (cameraView == SHIP_CAM_CLOSE) cameraView = SHIP_CAM_COCKPIT;
        else cameraView = SHIP_CAM_CHASE;
    }

void Spaceship::setTargetPlanet(const std::string& name, const glm::vec3& pos, float radius) {
        targetPlanetName = name;
        targetPlanetPos = pos;
        targetPlanetRadius = radius;
    }

void Spaceship::toggleOrbitAssist() {
        if (flightMode == FLIGHT_ORBIT_ASSIST) {
            flightMode = FLIGHT_MANUAL;
        } else {
            flightMode = FLIGHT_ORBIT_ASSIST;
            glm::vec3 center = targetPlanetPos;
            float r = std::max(targetPlanetRadius, 1.0f);
            orbitAssistRadius = std::max(r * 2.6f, 3.5f);
            glm::vec3 relPos = position - center;
            orbitAssistAngle = atan2(relPos.z, relPos.x);
        }
    }

void Spaceship::toggleAutopilot() {
        if (flightMode == FLIGHT_AUTOPILOT || warpSystem.isWarpActive()) {
            flightMode = FLIGHT_MANUAL;
            warpSystem.cancelWarp();
            isBoosting = false;
        } else {
            flightMode = FLIGHT_AUTOPILOT;
            warpSystem.engageWarp(targetPlanetPos, targetPlanetName, targetPlanetRadius);
        }
    }

    // Refresh the warp destination each frame so the ship homes onto a MOVING
    // planet's current position instead of where it was when warp was engaged.
    // Without this, the ship arrives at stale coordinates and visibly snaps.
void Spaceship::refreshWarpDestination() {
        if (warpSystem.isWarpActive() && !targetPlanetName.empty()) {
            warpSystem.updateDestination(targetPlanetPos);
        }
    }

    // Relative mouse delta steering for 6-DOF spaceship flight
void Spaceship::processMouseMovement(float xoffset, float yoffset) {
        if (!active) return;
        if (warpSystem.isWarpActive()) {
            return;
        }

        // Yaw: Mouse X delta (moving mouse left/right yaws ship left/right)
        float yawDelta = -xoffset * mouseSensitivity;
        // Pitch: Mouse Y delta (moving mouse down pitches nose up, moving up pitches nose down)
        float pitchDelta = yoffset * mouseSensitivity;

        if (std::abs(yawDelta) > 1e-6f || std::abs(pitchDelta) > 1e-6f) {
            // Disengage orbit assist or autopilot if manual mouse steering input is received
            if (flightMode == FLIGHT_ORBIT_ASSIST || flightMode == FLIGHT_AUTOPILOT) {
                flightMode = FLIGHT_MANUAL;
                warpSystem.cancelWarp();
            }

            glm::quat qYaw = glm::angleAxis(yawDelta, up);
            glm::quat qPitch = glm::angleAxis(pitchDelta, right);

            orientation = glm::normalize(qYaw * qPitch * orientation);
            forward = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f)));
            up = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f)));
            right = glm::normalize(glm::cross(forward, up));
            up = glm::normalize(glm::cross(right, forward));

            // Dynamic roll banking effect based on yaw motion
            float targetBank = glm::clamp(-yawDelta * 600.0f, -30.0f, 30.0f);
            visualBankAngle += (targetBank - visualBankAngle) * 0.35f;
        }
    }

void Spaceship::processInput(bool fwd, bool back, bool yawL, bool yawR, bool rollL, bool rollR,
                              bool pitchUp, bool pitchDown, bool boost, float dt) {
        if (!active) return;

        // Boost logic
        if (boost && boostEnergy > 5.0f && fwd) {
            isBoosting = true;
            boostEnergy = std::max(0.0f, boostEnergy - boostDrainRate * dt);
        } else {
            isBoosting = false;
            boostEnergy = std::min(maxBoostEnergy, boostEnergy + boostRechargeRate * dt);
        }

        if (flightMode == FLIGHT_ORBIT_ASSIST || flightMode == FLIGHT_AUTOPILOT || warpSystem.isWarpActive()) {
            // In Orbit assist or Autopilot/Warp, pilot can disengage by applying direct manual flight keys: W, S, A, D, Q, E, R, F
            if (fwd || back || yawL || yawR || rollL || rollR || pitchUp || pitchDown) {
                flightMode = FLIGHT_MANUAL;
                warpSystem.cancelWarp();
            } else {
                return;
            }
        }

        // Throttle control
        if (fwd) {
            throttle = std::min(1.0f, throttle + 2.0f * dt);
        } else if (back) {
            throttle = std::max(-0.35f, throttle - 2.5f * dt);
        } else {
            // Natural throttle cruise hold / gentle glide
            throttle = std::max(0.0f, throttle - 0.35f * dt);
        }

        // Angular control rates
        float yawDelta = 0.0f;
        if (yawL) yawDelta += 1.0f;
        if (yawR) yawDelta -= 1.0f;

        float rollDelta = 0.0f;
        if (rollL) rollDelta += 1.0f;
        if (rollR) rollDelta -= 1.0f;

        float pitchDelta = 0.0f;
        if (pitchUp) pitchDelta += 0.8f;
        if (pitchDown) pitchDelta -= 0.8f;

        // Apply smooth rotational updates without gimbal lock
        if (std::abs(yawDelta) > 0.01f || std::abs(rollDelta) > 0.01f || std::abs(pitchDelta) > 0.01f) {
            glm::quat qYaw = glm::angleAxis(glm::radians(yawDelta * yawRate * dt), up);
            glm::quat qPitch = glm::angleAxis(glm::radians(pitchDelta * pitchRate * dt), right);
            glm::quat qRoll = glm::angleAxis(glm::radians(rollDelta * rollRate * dt), forward);

            orientation = glm::normalize(qYaw * qPitch * qRoll * orientation);
            forward = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f)));
            up = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f)));
            right = glm::normalize(glm::cross(forward, up));
            up = glm::normalize(glm::cross(right, forward));
        }

        // Banking visual effect when yawing
        float targetBank = -yawDelta * 32.0f;
        visualBankAngle += (targetBank - visualBankAngle) * std::min(1.0f, 8.0f * dt);
    }

void Spaceship::update(float dt, const std::vector<std::pair<std::string, std::pair<glm::vec3, float>>>& planetaryBodies) {
        if (!active) return;
        dt = std::min(0.05f, std::max(0.001f, dt));

        // Track moving warp destinations before physics runs
        refreshWarpDestination();

        thrusterPulseTimer += dt * 8.0f;

        // Smooth throttle response
        currentThrottle += (throttle - currentThrottle) * std::min(1.0f, 6.0f * dt);
        float effectiveMaxSpeed = isBoosting ? boostSpeed : cruiseSpeed;
        float targetLinearSpeed = currentThrottle * effectiveMaxSpeed;

        if (flightMode == FLIGHT_ORBIT_ASSIST) {
            // Smooth circular orbit mechanics around target planet
            orbitAssistAngle += (orbitAssistSpeed / std::max(1.0f, orbitAssistRadius)) * dt * 35.0f;
            glm::vec3 targetPos = targetPlanetPos + glm::vec3(
                cos(orbitAssistAngle) * orbitAssistRadius,
                sin(orbitAssistAngle * 0.5f) * (orbitAssistRadius * 0.25f),
                sin(orbitAssistAngle) * orbitAssistRadius
            );

            glm::vec3 moveDir = targetPos - position;
            float distToOrbit = glm::length(moveDir);
            if (distToOrbit > 0.01f) {
                position += moveDir * std::min(1.0f, 4.0f * dt);
            }

            // Align ship tangent to orbit path
            glm::vec3 orbitTangent = glm::normalize(glm::vec3(-sin(orbitAssistAngle), 0.0f, cos(orbitAssistAngle)));
            forward += (orbitTangent - forward) * std::min(1.0f, 5.0f * dt);
            forward = glm::normalize(forward);
            right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            up = glm::normalize(glm::cross(right, forward));
            orientation = glm::quatLookAt(forward, up);
            velocity = orbitTangent * (orbitAssistSpeed * 25.0f);
        } else if (flightMode == FLIGHT_AUTOPILOT || warpSystem.isWarpActive()) {
            // Cinematic Warp Travel Sequence
            warpSystem.update(dt, position, forward, up, right, orientation, velocity, currentThrottle, isBoosting);

            if (!warpSystem.isWarpActive()) {
                // Arrived at destination! Transition to stable circular orbit assist
                flightMode = FLIGHT_ORBIT_ASSIST;
                float r = std::max(targetPlanetRadius, 1.0f);
                orbitAssistRadius = std::max(r * 2.8f, 6.0f);
                orbitAssistAngle = atan2(position.z - targetPlanetPos.z, position.x - targetPlanetPos.x);
                isBoosting = false;
            }
        } else {
            // Manual flight propulsion
            glm::vec3 desiredThrust = forward * (targetLinearSpeed * acceleration * dt);
            velocity += desiredThrust;
            velocity *= pow(linearDrag, dt * 60.0f);

            // Clamp max speed
            float curSpd = glm::length(velocity);
            if (curSpd > effectiveMaxSpeed) {
                velocity = (velocity / curSpd) * effectiveMaxSpeed;
            }

            position += velocity * dt;
        }

        // Anti-tunneling: substep large displacements so per-step movement never
        // exceeds a fraction of the smallest plausible collision radius. At warp
        // speeds a single frame can move the ship many units; without substeps the
        // proximity check below can skip clean past a planet's safety envelope.
        {
            const float minBodyRadius = 1.5f;   // conservative floor for body radii
            const float maxStepFraction = 0.4f; // step <= 40% of min safe radius
            float maxStep = minBodyRadius * 1.28f * maxStepFraction;
            glm::vec3 totalDisplacement = position - prevFramePosition;
            float dispLen = glm::length(totalDisplacement);
            if (dispLen > maxStep && dispLen > 0.0001f) {
                int steps = (int)std::ceil(dispLen / maxStep);
                steps = std::min(steps, 64); // hard cap for pathological frames
                glm::vec3 stepVec = totalDisplacement / (float)steps;
                glm::vec3 testPos = prevFramePosition;
                bool collided = false;
                for (int s = 0; s < steps && !collided; ++s) {
                    testPos += stepVec;
                    for (const auto& body : planetaryBodies) {
                        float safeR = body.second.second * 1.28f;
                        if (body.first == "Sun") safeR = body.second.second * 1.45f;
                        if (glm::length(testPos - body.second.first) < safeR) {
                            // Stop at the envelope edge instead of passing through
                            glm::vec3 outDir = glm::normalize(testPos - body.second.first);
                            position = body.second.first + outDir * safeR;
                            // Kill inward velocity component
                            float velDot = glm::dot(velocity, outDir);
                            if (velDot < 0.0f) velocity -= outDir * velDot;
                            collided = true;
                            break;
                        }
                    }
                }
                if (!collided) position = prevFramePosition + totalDisplacement;
            }
        }
        prevFramePosition = position;

        // Proximity detection and collision safety envelope
        nearestPlanetDist = 99999.0f;
        nearestPlanetName = "None";
        proximityAlertActive = false;

        for (const auto& body : planetaryBodies) {
            const std::string& name = body.first;
            const glm::vec3& bPos = body.second.first;
            float bRadius = body.second.second;

            if (name == targetPlanetName) {
                targetPlanetPos = bPos;
                targetPlanetRadius = bRadius;
                targetDistance = glm::length(position - bPos);
            }

            float dist = glm::length(position - bPos);
            if (dist < nearestPlanetDist) {
                nearestPlanetDist = dist;
                nearestPlanetName = name;
                nearestPlanetRadius = bRadius;
            }

            // Collision safety: soft repulsive deflection before clipping surface
            float safeRadius = bRadius * 1.28f;
            if (name == "Sun") safeRadius = bRadius * 1.45f;

            if (dist < safeRadius && dist > 0.001f) {
                proximityAlertActive = true;
                glm::vec3 repulsion = glm::normalize(position - bPos);
                float penetration = safeRadius - dist;
                position += repulsion * (penetration * std::min(1.0f, 12.0f * dt));

                // Deflect velocity outward
                float velDot = glm::dot(velocity, repulsion);
                if (velDot < 0.0f) {
                    velocity -= repulsion * (velDot * 1.5f);
                }
            } else if (dist < safeRadius * 1.8f) {
                proximityAlertActive = true;
            }
        }

        // Update Particle Trail System
        if (currentThrottle > 0.05f || isBoosting) {
            float emitRate = isBoosting ? 4 : 2;
            for (int i = 0; i < emitRate; ++i) {
                glm::vec3 engineOffsetL = position - forward * 1.4f - right * 0.45f - up * 0.1f;
                glm::vec3 engineOffsetR = position - forward * 1.4f + right * 0.45f - up * 0.1f;

                glm::vec4 trailCol = isBoosting ? glm::vec4(0.35f, 0.75f, 1.0f, 0.95f)
                                                : glm::vec4(0.20f, 0.55f, 0.95f, 0.75f);

                trails.push_back({engineOffsetL, -forward * (isBoosting ? 18.0f : 8.0f) + (float(rand()%100)/100.0f - 0.5f)*0.3f, 0.45f, 0.45f, isBoosting ? 0.28f : 0.18f, trailCol});
                trails.push_back({engineOffsetR, -forward * (isBoosting ? 18.0f : 8.0f) + (float(rand()%100)/100.0f - 0.5f)*0.3f, 0.45f, 0.45f, isBoosting ? 0.28f : 0.18f, trailCol});
            }
        }

        for (auto it = trails.begin(); it != trails.end();) {
            it->life -= dt;
            it->pos += it->vel * dt;
            it->size *= (1.0f + 1.2f * dt);
            if (it->life <= 0.0f) {
                it = trails.erase(it);
            } else {
                ++it;
            }
        }

        // Smooth Camera Follow calculation
        float currentSpeedVal = glm::length(velocity);
        glm::vec3 targetCameraEye;
        glm::vec3 targetCameraLook;

        if (cameraView == SHIP_CAM_CHASE) {
            float distBehind = 6.2f + (currentSpeedVal / boostSpeed) * 3.5f;
            float distAbove = 1.8f + (currentSpeedVal / boostSpeed) * 0.6f;
            targetCameraEye = position - forward * distBehind + up * distAbove;
            targetCameraLook = position + forward * 14.0f;
        } else if (cameraView == SHIP_CAM_CLOSE) {
            targetCameraEye = position - forward * 3.6f + up * 1.1f;
            targetCameraLook = position + forward * 10.0f;
        } else { // Cockpit
            targetCameraEye = position + forward * 0.65f + up * 0.35f;
            targetCameraLook = position + forward * 25.0f;
        }

        float camLerpSpeed = (cameraView == SHIP_CAM_COCKPIT) ? 28.0f : 12.0f;
        smoothCameraEye += (targetCameraEye - smoothCameraEye) * std::min(1.0f, camLerpSpeed * dt);
        smoothCameraTarget += (targetCameraLook - smoothCameraTarget) * std::min(1.0f, camLerpSpeed * dt);
        smoothCameraUp += (up - smoothCameraUp) * std::min(1.0f, camLerpSpeed * dt);

        // Sound modulation parameters
        float speedRatio = std::min(1.0f, currentSpeedVal / cruiseSpeed);
        soundPitch = 0.80f + speedRatio * 0.85f + (isBoosting ? 0.45f : 0.0f);
        soundVolume = 0.15f + currentThrottle * 0.65f + (isBoosting ? 0.20f : 0.0f);
    }

glm::mat4 Spaceship::getViewMatrix() const {
        return glm::lookAt(smoothCameraEye, smoothCameraTarget, smoothCameraUp);
    }

glm::vec3 Spaceship::getCameraEye() const {
        return smoothCameraEye;
    }

glm::vec3 Spaceship::getCameraTarget() const {
        return smoothCameraTarget;
    }

float Spaceship::getSpeedKmh() const {
        return glm::length(velocity) * 1250.0f; // Scale to simulated cinematic km/s
    }

    // Procedural Sci-Fi Spaceship Mesh Rendering (High Fidelity)
    // Core-profile path: all geometry streams through ImmediateBatch with
    // per-vertex colors (material diffuse values baked in). Fixed-function
    // materials/lighting are retired; setMat is kept as a no-op so historic
    // call sites remain valid.
void Spaceship::setMat(const GLfloat amb[4], const GLfloat diff[4], const GLfloat spec[4], float shininess, const GLfloat emis[4]) {
        (void)amb; (void)diff; (void)spec; (void)shininess; (void)emis; // no-op: core profile
    }

void Spaceship::render(const glm::mat4& projection, const glm::mat4& view) {
        if (!active) return;
        if (!renderBatch.isReady()) renderBatch.init(kFlatVS, kFlatFS);

        // In cockpit view, render interior framing instead of exterior 3rd person body
        if (cameraView == SHIP_CAM_COCKPIT) {
            renderCockpitInterior(projection, view);
            return;
        }

        // Compute orientation matrix with dynamic banking roll
        glm::mat4 rotMat = glm::mat4_cast(orientation);
        rotMat = glm::rotate(rotMat, glm::radians(visualBankAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        shipModelMat = glm::translate(glm::mat4(1.0f), position) * rotMat;

        // Local-space -> world-space helper for batch vertices
        auto V = [&](float x, float y, float z) {
            return glm::vec3(shipModelMat * glm::vec4(x, y, z, 1.0f));
        };

        // --- MATERIAL PALETTE (Matching Concept Art) ---
        // 1. Primary Off-White Aerospace Ceramic Plating
        GLfloat matHullWhiteAmb[]  = {0.28f, 0.32f, 0.36f, 1.0f};
        GLfloat matHullWhiteDiff[] = {0.88f, 0.91f, 0.95f, 1.0f};
        GLfloat matHullWhiteSpec[] = {0.85f, 0.90f, 0.95f, 1.0f};

        // 2. Charcoal Thermal Insulation Tiles / Ventral Panels
        GLfloat matCarbonDarkAmb[]  = {0.10f, 0.12f, 0.14f, 1.0f};
        GLfloat matCarbonDarkDiff[] = {0.20f, 0.22f, 0.26f, 1.0f};
        GLfloat matCarbonDarkSpec[] = {0.35f, 0.38f, 0.42f, 1.0f};

        // 3. Burnished Titanium Metal Accents & Engine Cowlings
        GLfloat matTitaniumAmb[]  = {0.22f, 0.25f, 0.28f, 1.0f};
        GLfloat matTitaniumDiff[] = {0.48f, 0.52f, 0.58f, 1.0f};
        GLfloat matTitaniumSpec[] = {0.92f, 0.95f, 0.98f, 1.0f};

        // 4. Cockpit Crystalline Cyan Glass Canopy
        GLfloat matCanopyGlassAmb[]  = {0.08f, 0.30f, 0.45f, 1.0f};
        GLfloat matCanopyGlassDiff[] = {0.15f, 0.68f, 0.92f, 1.0f};
        GLfloat matCanopyGlassSpec[] = {1.00f, 1.00f, 1.00f, 1.0f};
        GLfloat matCanopyGlassEmis[] = {0.06f, 0.24f, 0.36f, 1.0f};

        // 5. Canopy Structural Mullions / Frame
        GLfloat matCanopyFrameAmb[]  = {0.14f, 0.16f, 0.18f, 1.0f};
        GLfloat matCanopyFrameDiff[] = {0.28f, 0.30f, 0.34f, 1.0f};
        GLfloat matCanopyFrameSpec[] = {0.70f, 0.75f, 0.80f, 1.0f};

        // 6. Luminescent Cyan Panel Lines & Status Accents
        GLfloat matCyanEmisAmb[]  = {0.00f, 0.35f, 0.55f, 1.0f};
        GLfloat matCyanEmisDiff[] = {0.10f, 0.80f, 1.00f, 1.0f};
        GLfloat matCyanEmisSpec[] = {1.00f, 1.00f, 1.00f, 1.0f};
        GLfloat matCyanEmisEmis[] = {0.20f, 0.85f, 1.00f, 1.0f};

        // -------------------------------------------------------------
        // 1. PRIMARY FUSELAGE & SCULPTED CHINES (White Aerospace Plating)
        // -------------------------------------------------------------
        setMat(matHullWhiteAmb, matHullWhiteDiff, matHullWhiteSpec, 64.0f);
        const glm::vec4 hullWhite(matHullWhiteDiff[0], matHullWhiteDiff[1], matHullWhiteDiff[2], 1.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, hullWhite); renderBatch.vertex(b, hullWhite); renderBatch.vertex(c, hullWhite);
            };
            // Forward Nose Radome Upper Apex
            T(V(0.0f, 0.16f, -2.15f), V(-0.36f, 0.24f, -1.05f), V(0.36f, 0.24f, -1.05f));
            // Forward Nose Upper-Mid Transition
            T(V(-0.36f, 0.24f, -1.05f), V(-0.52f, 0.22f, -0.15f), V(0.52f, 0.22f, -0.15f));
            T(V(-0.36f, 0.24f, -1.05f), V(0.52f, 0.22f, -0.15f), V(0.36f, 0.24f, -1.05f));
            // Dorsal Spine Ridge
            T(V(0.0f, 0.44f, -0.15f), V(-0.52f, 0.22f, -0.15f), V(-0.46f, 0.20f, 1.45f));
            T(V(0.0f, 0.44f, -0.15f), V(-0.46f, 0.20f, 1.45f), V(0.0f, 0.38f, 1.45f));
            T(V(0.0f, 0.44f, -0.15f), V(0.0f, 0.38f, 1.45f), V(0.46f, 0.20f, 1.45f));
            T(V(0.0f, 0.44f, -0.15f), V(0.46f, 0.20f, 1.45f), V(0.52f, 0.22f, -0.15f));
            // Port & Starboard Chamfered Chine Flanges
            T(V(0.0f, 0.16f, -2.15f), V(-0.38f, -0.06f, -1.05f), V(-0.36f, 0.24f, -1.05f));
            T(V(0.0f, 0.16f, -2.15f), V(0.36f, 0.24f, -1.05f), V(0.38f, -0.06f, -1.05f));
            // Mid Fuselage Side Flanks
            T(V(-0.36f, 0.24f, -1.05f), V(-0.38f, -0.06f, -1.05f), V(-0.52f, 0.22f, -0.15f));
            T(V(-0.38f, -0.06f, -1.05f), V(-0.54f, -0.08f, -0.15f), V(-0.52f, 0.22f, -0.15f));
            T(V(0.36f, 0.24f, -1.05f), V(0.52f, 0.22f, -0.15f), V(0.38f, -0.06f, -1.05f));
            T(V(0.38f, -0.06f, -1.05f), V(0.52f, 0.22f, -0.15f), V(0.54f, -0.08f, -0.15f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 2. VENTRAL HEAT SHIELD & INTAKE UNDERBELLIES (Dark Carbon Ceramic)
        // -------------------------------------------------------------
        setMat(matCarbonDarkAmb, matCarbonDarkDiff, matCarbonDarkSpec, 24.0f);
        const glm::vec4 carbonDark(matCarbonDarkDiff[0], matCarbonDarkDiff[1], matCarbonDarkDiff[2], 1.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, carbonDark); renderBatch.vertex(b, carbonDark); renderBatch.vertex(c, carbonDark);
            };
            // Forward Ventral Nose Ramp
            T(V(0.0f, -0.12f, -2.15f), V(0.38f, -0.06f, -1.05f), V(-0.38f, -0.06f, -1.05f));
            // Ventral Belly Tiling (Centerline heatshield)
            T(V(-0.38f, -0.06f, -1.05f), V(0.38f, -0.06f, -1.05f), V(0.48f, -0.12f, 1.45f));
            T(V(-0.38f, -0.06f, -1.05f), V(0.48f, -0.12f, 1.45f), V(-0.48f, -0.12f, 1.45f));
            // Aft Bulkhead
            T(V(-0.48f, 0.22f, 1.45f), V(0.48f, 0.22f, 1.45f), V(0.48f, -0.12f, 1.45f));
            T(V(-0.48f, 0.22f, 1.45f), V(0.48f, -0.12f, 1.45f), V(-0.48f, -0.12f, 1.45f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 3. LATERAL INTAKE SCOOPS & INTAKE CAVITIES (Port & Starboard)
        // -------------------------------------------------------------
        setMat(matTitaniumAmb, matTitaniumDiff, matTitaniumSpec, 80.0f);
        const glm::vec4 titanium(matTitaniumDiff[0], matTitaniumDiff[1], matTitaniumDiff[2], 1.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, titanium); renderBatch.vertex(b, titanium); renderBatch.vertex(c, titanium);
            };
            // Starboard Intake Cowling Outer Lip
            T(V(0.50f, 0.22f, -0.55f), V(0.78f, 0.16f, -0.30f), V(0.52f, 0.04f, 0.35f));
            T(V(0.78f, 0.16f, -0.30f), V(0.76f, -0.06f, -0.30f), V(0.52f, 0.04f, 0.35f));
            // Port Intake Cowling Outer Lip
            T(V(-0.50f, 0.22f, -0.55f), V(-0.52f, 0.04f, 0.35f), V(-0.78f, 0.16f, -0.30f));
            T(V(-0.78f, 0.16f, -0.30f), V(-0.52f, 0.04f, 0.35f), V(-0.76f, -0.06f, -0.30f));
        }
        renderBatch.end();

        // Recessed Dark Intake Inlets (Black/Cyan cavity)
        setMat(matCarbonDarkAmb, matCarbonDarkDiff, matCarbonDarkSpec, 10.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto Q = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
                renderBatch.vertex(a, carbonDark); renderBatch.vertex(b, carbonDark); renderBatch.vertex(c, carbonDark);
                renderBatch.vertex(a, carbonDark); renderBatch.vertex(c, carbonDark); renderBatch.vertex(d, carbonDark);
            };
            // Starboard inlet mouth
            Q(V(0.50f, 0.22f, -0.55f), V(0.78f, 0.16f, -0.30f), V(0.76f, -0.06f, -0.30f), V(0.50f, -0.04f, -0.55f));
            // Port inlet mouth
            Q(V(-0.50f, 0.22f, -0.55f), V(-0.50f, -0.04f, -0.55f), V(-0.76f, -0.06f, -0.30f), V(-0.78f, 0.16f, -0.30f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 4. CRANKED-DELTA WINGS & WINGLETS (Aerospace Ceramic + Carbon Trim)
        // -------------------------------------------------------------
        setMat(matHullWhiteAmb, matHullWhiteDiff, matHullWhiteSpec, 64.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, hullWhite); renderBatch.vertex(b, hullWhite); renderBatch.vertex(c, hullWhite);
            };
            // Starboard Wing Upper Surface
            T(V(0.52f, 0.12f, -0.35f), V(2.35f, 0.06f, 1.15f), V(0.48f, 0.12f, 1.35f));
            // Port Wing Upper Surface
            T(V(-0.52f, 0.12f, -0.35f), V(-0.48f, 0.12f, 1.35f), V(-2.35f, 0.06f, 1.15f));
            // Wing Underbellies
            T(V(0.52f, -0.06f, -0.35f), V(0.48f, -0.06f, 1.35f), V(2.35f, -0.02f, 1.15f));
            T(V(-0.52f, -0.06f, -0.35f), V(-2.35f, -0.02f, 1.15f), V(-0.48f, -0.06f, 1.35f));
            // Canted Wingtip Winglets (Angled Outward & Upward)
            T(V(2.35f, 0.06f, 1.15f), V(2.55f, 0.68f, 1.35f), V(1.85f, 0.08f, 1.35f));
            T(V(-2.35f, 0.06f, 1.15f), V(-1.85f, 0.08f, 1.35f), V(-2.55f, 0.68f, 1.35f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 5. TWIN ANGLED VERTICAL STABILIZERS (Outward-Canted Twin Tailfins)
        // -------------------------------------------------------------
        setMat(matHullWhiteAmb, matHullWhiteDiff, matHullWhiteSpec, 64.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, hullWhite); renderBatch.vertex(b, hullWhite); renderBatch.vertex(c, hullWhite);
            };
            // Starboard Tailfin (Canted Outward at ~16 deg)
            T(V(0.42f, 0.20f, 0.45f), V(0.68f, 0.82f, 1.30f), V(0.46f, 0.20f, 1.50f));
            // Port Tailfin
            T(V(-0.42f, 0.20f, 0.45f), V(-0.46f, 0.20f, 1.50f), V(-0.68f, 0.82f, 1.30f));
        }
        renderBatch.end();

        // Tailfin Leading Edge Carbon Ceramic Trim
        setMat(matCarbonDarkAmb, matCarbonDarkDiff, matCarbonDarkSpec, 35.0f);
        renderBatch.begin(GL_LINES, projection, view);
        {
            auto L = [&](const glm::vec3& a, const glm::vec3& b) {
                renderBatch.vertex(a, carbonDark); renderBatch.vertex(b, carbonDark);
            };
            L(V(0.42f, 0.20f, 0.45f), V(0.68f, 0.82f, 1.30f));
            L(V(-0.42f, 0.20f, 0.45f), V(-0.68f, 0.82f, 1.30f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 6. COCKPIT CRYSTALLINE GLASS CANOPY (Multi-Pane Sci-Fi Glass)
        // -------------------------------------------------------------
        setMat(matCanopyGlassAmb, matCanopyGlassDiff, matCanopyGlassSpec, 128.0f, matCanopyGlassEmis);
        const glm::vec4 canopyGlass(matCanopyGlassDiff[0], matCanopyGlassDiff[1], matCanopyGlassDiff[2], 0.85f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto T = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                renderBatch.vertex(a, canopyGlass); renderBatch.vertex(b, canopyGlass); renderBatch.vertex(c, canopyGlass);
            };
            // Front Main Windshield Pane
            T(V(0.0f, 0.36f, -1.25f), V(-0.28f, 0.28f, -0.45f), V(0.28f, 0.28f, -0.45f));
            // Overhead Canopy Roof Pane
            T(V(0.0f, 0.45f, -0.25f), V(0.28f, 0.28f, -0.45f), V(-0.28f, 0.28f, -0.45f));
            T(V(0.0f, 0.45f, -0.25f), V(0.24f, 0.32f, 0.05f), V(0.28f, 0.28f, -0.45f));
            T(V(0.0f, 0.45f, -0.25f), V(-0.28f, 0.28f, -0.45f), V(-0.24f, 0.32f, 0.05f));
        }
        renderBatch.end();

        // Canopy Structural Framing Mullions (Dark Graphite Ribs)
        setMat(matCanopyFrameAmb, matCanopyFrameDiff, matCanopyFrameSpec, 45.0f);
        const glm::vec4 canopyFrame(matCanopyFrameDiff[0], matCanopyFrameDiff[1], matCanopyFrameDiff[2], 1.0f);
        renderBatch.begin(GL_LINES, projection, view);
        {
            auto L = [&](const glm::vec3& a, const glm::vec3& b) {
                renderBatch.vertex(a, canopyFrame); renderBatch.vertex(b, canopyFrame);
            };
            L(V(0.0f, 0.36f, -1.25f), V(0.0f, 0.45f, -0.25f));
            L(V(-0.28f, 0.28f, -0.45f), V(0.0f, 0.45f, -0.25f));
            L(V(0.28f, 0.28f, -0.45f), V(0.0f, 0.45f, -0.25f));
            L(V(-0.28f, 0.28f, -0.45f), V(0.28f, 0.28f, -0.45f));
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 7. LUMINESCENT CYAN STATUS STRIPES & SENSOR SUITE
        // -------------------------------------------------------------
        setMat(matCyanEmisAmb, matCyanEmisDiff, matCyanEmisSpec, 96.0f, matCyanEmisEmis);
        const glm::vec4 cyanGlow(matCyanEmisDiff[0], matCyanEmisDiff[1], matCyanEmisDiff[2], 1.0f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto Q = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
                renderBatch.vertex(a, cyanGlow); renderBatch.vertex(b, cyanGlow); renderBatch.vertex(c, cyanGlow);
                renderBatch.vertex(a, cyanGlow); renderBatch.vertex(c, cyanGlow); renderBatch.vertex(d, cyanGlow);
            };
            // Dorsal Spine Avionics Status Strip
            Q(V(-0.04f, 0.44f, -0.15f), V(0.04f, 0.44f, -0.15f), V(0.03f, 0.38f, 1.45f), V(-0.03f, 0.38f, 1.45f));
            // Forward Nose Sensor Optical Window
            Q(V(-0.06f, 0.17f, -2.12f), V(0.06f, 0.17f, -2.12f), V(0.05f, 0.21f, -1.75f), V(-0.05f, 0.21f, -1.75f));
        }
        renderBatch.end();

        // Forward Sensor Needle Probe
        setMat(matTitaniumAmb, matTitaniumDiff, matTitaniumSpec, 95.0f);
        renderBatch.begin(GL_LINES, projection, view);
        {
            renderBatch.vertex(V(0.0f, 0.16f, -2.15f), titanium);
            renderBatch.vertex(V(0.0f, 0.16f, -2.55f), titanium);
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 8. DUAL VECTORING ENGINE NOZZLES & MAGNETIC CONFINEMENT RINGS
        // -------------------------------------------------------------
        setMat(matTitaniumAmb, matTitaniumDiff, matTitaniumSpec, 90.0f);

        float engX[2] = {-0.30f, 0.30f};
        for (int e = 0; e < 2; ++e) {
            float ex = engX[e];

            // Cylindrical Titanium Nozzle Bell (quad strip -> triangles)
            renderBatch.begin(GL_TRIANGLES, projection, view);
            for (int i = 0; i < 16; ++i) {
                float a0 = (float)i * 6.2831853f / 16.0f;
                float a1 = (float)(i + 1) * 6.2831853f / 16.0f;
                glm::vec3 b0 = V(ex + cos(a0) * 0.19f, sin(a0) * 0.19f, 1.40f);
                glm::vec3 t0 = V(ex + cos(a0) * 0.22f, sin(a0) * 0.22f, 1.58f);
                glm::vec3 b1 = V(ex + cos(a1) * 0.19f, sin(a1) * 0.19f, 1.40f);
                glm::vec3 t1 = V(ex + cos(a1) * 0.22f, sin(a1) * 0.22f, 1.58f);
                renderBatch.vertex(b0, titanium); renderBatch.vertex(t0, titanium); renderBatch.vertex(b1, titanium);
                renderBatch.vertex(b1, titanium); renderBatch.vertex(t0, titanium); renderBatch.vertex(t1, titanium);
            }
            renderBatch.end();

            // Inner Magnetic Confinement Ring (Emissive Blue)
            setMat(matCyanEmisAmb, matCyanEmisDiff, matCyanEmisSpec, 96.0f, matCyanEmisEmis);
            renderBatch.begin(GL_TRIANGLES, projection, view);
            for (int i = 0; i < 12; ++i) {
                float a0 = (float)i * 6.2831853f / 12.0f;
                float a1 = (float)(i + 1) * 6.2831853f / 12.0f;
                glm::vec3 lo0 = V(ex + cos(a0) * 0.16f, sin(a0) * 0.16f, 1.44f);
                glm::vec3 hi0 = V(ex + cos(a0) * 0.16f, sin(a0) * 0.16f, 1.52f);
                glm::vec3 lo1 = V(ex + cos(a1) * 0.16f, sin(a1) * 0.16f, 1.44f);
                glm::vec3 hi1 = V(ex + cos(a1) * 0.16f, sin(a1) * 0.16f, 1.52f);
                renderBatch.vertex(lo0, cyanGlow); renderBatch.vertex(hi0, cyanGlow); renderBatch.vertex(lo1, cyanGlow);
                renderBatch.vertex(lo1, cyanGlow); renderBatch.vertex(hi0, cyanGlow); renderBatch.vertex(hi1, cyanGlow);
            }
            renderBatch.end();
        }

        // -------------------------------------------------------------
        // 9. DYNAMIC DUAL ION THRUSTER PLUMES (Nested Hot Core + Plasma Mantle)
        // -------------------------------------------------------------
        // Pulse timer modulation
        float plumePulse = 0.90f + 0.10f * sin(thrusterPulseTimer * 14.0f);
        float basePlumeLen = (currentThrottle * 1.6f + (isBoosting ? 3.2f : 0.0f)) * plumePulse;

        if (basePlumeLen > 0.05f) {
            for (int e = 0; e < 2; ++e) {
                float ex = engX[e];

                // Layer 1: Outer Translucent Ion Flame (Cyan / Electric Blue)
                // Triangle fan -> triangle list: tip + ring segments
                const glm::vec4 plumeOuter(0.15f, 0.65f, 1.0f, 0.65f);
                const glm::vec4 plumeOuterEdge(0.08f, 0.35f, 0.95f, 0.08f);
                glm::vec3 tip1 = V(ex, 0.0f, 1.60f + basePlumeLen * 2.1f);
                renderBatch.begin(GL_TRIANGLES, projection, view);
                for (int i = 0; i < 16; ++i) {
                    float a0 = (float)i * 6.2831853f / 16.0f;
                    float a1 = (float)(i + 1) * 6.2831853f / 16.0f;
                    glm::vec3 r0 = V(ex + cos(a0) * 0.24f, sin(a0) * 0.24f, 1.55f);
                    glm::vec3 r1 = V(ex + cos(a1) * 0.24f, sin(a1) * 0.24f, 1.55f);
                    renderBatch.vertex(tip1, plumeOuter);
                    renderBatch.vertex(r0, plumeOuterEdge);
                    renderBatch.vertex(r1, plumeOuterEdge);
                }
                renderBatch.end();

                // Layer 2: Inner Blinding High-Velocity Core (Radiant Blue-White)
                const glm::vec4 coreTip(0.92f, 0.97f, 1.0f, 0.95f);
                const glm::vec4 coreEdge(0.40f, 0.85f, 1.0f, 0.25f);
                glm::vec3 tip2 = V(ex, 0.0f, 1.60f + basePlumeLen * 1.1f);
                renderBatch.begin(GL_TRIANGLES, projection, view);
                for (int i = 0; i < 12; ++i) {
                    float a0 = (float)i * 6.2831853f / 12.0f;
                    float a1 = (float)(i + 1) * 6.2831853f / 12.0f;
                    glm::vec3 r0 = V(ex + cos(a0) * 0.11f, sin(a0) * 0.11f, 1.55f);
                    glm::vec3 r1 = V(ex + cos(a1) * 0.11f, sin(a1) * 0.11f, 1.55f);
                    renderBatch.vertex(tip2, coreTip);
                    renderBatch.vertex(r0, coreEdge);
                    renderBatch.vertex(r1, coreEdge);
                }
                renderBatch.end();
            }
        }

        // -------------------------------------------------------------
        // 10. NAVIGATION BEACONS & FORMATION STROBE LIGHTS
        // -------------------------------------------------------------
        // Pulsing Nav Lights (Red Port, Green Starboard)
        float navPulse = (sin(thrusterPulseTimer * 3.5f) > 0.0f) ? 1.0f : 0.25f;
        float strobeBlink = (fmod(thrusterPulseTimer * 2.0f, 1.0f) < 0.15f) ? 1.0f : 0.0f;
        renderBatch.begin(GL_POINTS, projection, view);
        // Port Wingtip Nav Light (Aviation Red)
        renderBatch.vertex(V(-2.55f, 0.68f, 1.35f), glm::vec4(1.0f, 0.15f, 0.20f, navPulse), 6.0f);
        // Starboard Wingtip Nav Light (Aviation Green)
        renderBatch.vertex(V(2.55f, 0.68f, 1.35f), glm::vec4(0.15f, 1.0f, 0.30f, navPulse), 6.0f);
        // Tailfin Apex Formation Markers (Steady Cyan)
        renderBatch.vertex(V(-0.68f, 0.82f, 1.30f), glm::vec4(0.20f, 0.85f, 1.0f, 0.85f), 6.0f);
        renderBatch.vertex(V(0.68f, 0.82f, 1.30f), glm::vec4(0.20f, 0.85f, 1.0f, 0.85f), 6.0f);
        // Dorsal Anti-Collision Strobe (Bright White Pulse at 2.0 Hz)
        if (strobeBlink > 0.5f) {
            renderBatch.vertex(V(0.0f, 0.44f, -0.15f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 6.0f); // Dorsal Beacon
            renderBatch.vertex(V(0.0f, 0.38f, 1.45f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 6.0f);  // Aft Beacon
        }
        renderBatch.end();

        // -------------------------------------------------------------
        // 11. PARTICLE EXHAUST TRAILS (3D Swirling Ion Wake)
        // -------------------------------------------------------------
        if (!trails.empty()) {
            renderBatch.begin(GL_POINTS, projection, view);
            for (const auto& t : trails) {
                float alpha = (t.life / t.maxLife) * t.color.a;
                renderBatch.vertex(t.pos, glm::vec4(t.color.r, t.color.g, t.color.b, alpha), 4.5f);
            }
            renderBatch.end();
        }
    }

    // First-Person Cockpit Interior Frame Rendering
void Spaceship::renderCockpitInterior(const glm::mat4& projection, const glm::mat4& view) {
        if (!renderBatch.isReady()) renderBatch.init(kFlatVS, kFlatFS);

        // Compute orientation matrix with dynamic banking roll
        glm::mat4 rotMat = glm::mat4_cast(orientation);
        rotMat = glm::rotate(rotMat, glm::radians(visualBankAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        shipModelMat = glm::translate(glm::mat4(1.0f), position) * rotMat;

        auto V = [&](float x, float y, float z) {
            return glm::vec3(shipModelMat * glm::vec4(x, y, z, 1.0f));
        };

        // Cockpit Frame Material: Sleek Dark Graphite Aerospace Alloy
        // Eye is at (0.0, 0.35, -0.65), looking forward (-Z)
        const glm::vec4 cockpitFrame(0.06f, 0.08f, 0.11f, 0.96f);
        renderBatch.begin(GL_TRIANGLES, projection, view);
        {
            auto Q = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
                renderBatch.vertex(a, cockpitFrame); renderBatch.vertex(b, cockpitFrame); renderBatch.vertex(c, cockpitFrame);
                renderBatch.vertex(a, cockpitFrame); renderBatch.vertex(c, cockpitFrame); renderBatch.vertex(d, cockpitFrame);
            };
            // Left A-Pillar (Canopy Strut)
            Q(V(-0.80f, -0.15f, -0.85f), V(-0.62f, -0.15f, -0.85f), V(-0.32f, 0.72f, -1.35f), V(-0.42f, 0.72f, -1.35f));
            // Right A-Pillar (Canopy Strut)
            Q(V(0.62f, -0.15f, -0.85f), V(0.80f, -0.15f, -0.85f), V(0.42f, 0.72f, -1.35f), V(0.32f, 0.72f, -1.35f));
            // Top Canopy Header Arch
            Q(V(-0.42f, 0.72f, -1.35f), V(0.42f, 0.72f, -1.35f), V(0.42f, 0.85f, -1.35f), V(-0.42f, 0.85f, -1.35f));
            // Lower Coaming / Avionics Glare Shield Dashboard
            Q(V(-1.15f, -0.15f, -0.85f), V(1.15f, -0.15f, -0.85f), V(0.75f, 0.08f, -1.55f), V(-0.75f, 0.08f, -1.55f));
        }
        renderBatch.end();

        // Luminescent Cyan Cockpit Instrument Frame Lines & Status Displays
        const glm::vec4 instrumentCyan(0.0f, 0.85f, 1.0f, 0.85f);
        renderBatch.begin(GL_LINES, projection, view);
        {
            auto L = [&](const glm::vec3& a, const glm::vec3& b) {
                renderBatch.vertex(a, instrumentCyan); renderBatch.vertex(b, instrumentCyan);
            };
            // Left A-Pillar inner trim
            L(V(-0.64f, -0.12f, -0.88f), V(-0.34f, 0.70f, -1.33f));
            // Right A-Pillar inner trim
            L(V(0.64f, -0.12f, -0.88f), V(0.34f, 0.70f, -1.33f));
            // Top arch inner trim
            L(V(-0.34f, 0.70f, -1.33f), V(0.34f, 0.70f, -1.33f));
            // Lower dash brow line
            L(V(-0.72f, 0.09f, -1.52f), V(0.72f, 0.09f, -1.52f));
            // Left & Right MFD bezel outlines
            L(V(-0.55f, 0.04f, -1.40f), V(-0.18f, 0.04f, -1.40f));
            L(V(0.18f, 0.04f, -1.40f), V(0.55f, 0.04f, -1.40f));
        }
        renderBatch.end();
    }

