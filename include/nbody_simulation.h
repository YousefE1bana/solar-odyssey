#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <glm/glm.hpp>

enum PhysicsMode {
    PHYSICS_KEPLERIAN = 0,
    PHYSICS_NBODY = 1
};

struct NBodyObject {
    std::string name;
    glm::dvec3 position{0.0};
    glm::dvec3 velocity{0.0};
    glm::dvec3 acceleration{0.0};
    double mass = 1.0;
    double radius = 1.0;
    bool isStatic = false;
    std::string parentBody = ""; // Non-empty for satellites (e.g. Moon -> Earth)
};

class NBodySimulation {
public:
    PhysicsMode mode = PHYSICS_KEPLERIAN;
    double gravitationalConstant = 4000.0; // Calibrated central G*M_sun parameter
    double softening = 0.15;               // Softening length to prevent singularities
    double fixedDeltaTime = 0.0025;        // Fixed timestep in scaled simulation seconds
    double timeAccumulator = 0.0;
    double maxAccumulatorCap = 0.5;        // Max accumulator to prevent spiral-of-death on extreme lags

    std::vector<NBodyObject> bodies;
    std::map<std::string, size_t> nameToIndex;

    NBodySimulation();

    void reset();
    void addBody(const std::string& name, double mass, double radius, bool isStatic = false, const std::string& parent = "");
    
    NBodyObject* getBody(const std::string& name);
    const NBodyObject* getBody(const std::string& name) const;
    glm::dvec3 getBodyPositionDouble(const std::string& name) const;
    glm::vec3 getBodyPosition(const std::string& name) const;
    glm::dvec3 getBodyVelocityDouble(const std::string& name) const;
    glm::vec3 getBodyVelocity(const std::string& name) const;

    // Direct pairwise gravitational acceleration calculation
    glm::dvec3 computeAccelerationForPoint(const glm::dvec3& point, double pointMass = 0.0) const;
    glm::vec3 computeAccelerationForPoint(const glm::vec3& point, float pointMass = 0.0f) const;
    void computeAllAccelerations();

    // Symplectic Velocity Verlet (Leapfrog) Step
    void stepVerlet(double dt);

    // Main update loop with fixed-step accumulation
    // Returns only time actually integrated, excluding backlog and dropped time.
    double update(double deltaTime, double timeMultiplier);

    // Smooth C1 transition from Keplerian motion function
    void initializeFromKeplerian(
        const std::function<glm::dvec3(const std::string&, double)>& positionFunc,
        double currentSimTime,
        double speedScale = 1.0
    );

    // FINAL HOLD Defect 2 — generic parent-relative circular-velocity
    // seeding. The finite-difference velocities from initializeFromKeplerian
    // reproduce the aesthetic presentation motion (~0.3 units/simTime),
    // which is ~50x slower than the gravitational circular velocity for
    // G=4000 (e.g. Earth needs 20.0 at R=10). Seeding those directly leaves
    // every orbit deeply sub-orbital: bodies fall through the center,
    // slingshot out, and the system disperses within days (pre-existing for
    // 14 bodies; the 19-body roster only exposed it on longer horizons).
    // This pass keeps the seeded world-space POSITIONS and assigns each
    // non-static body a prograde circular velocity around its dynamical
    // parent — Sun for empty-parentBody entries (planets/dwarfs), the named
    // parent body for moons (parent velocity included) — from live masses
    // and G. No body names, no roster counts, no mass/position changes.
    // Moons at presentation radii lie outside their parents' Hill spheres
    // (giant-scale compression), so they become nearby heliocentric
    // companions rather than bound satellites; the system stays coherent
    // instead of dispersing. Keplerian mode is untouched.
    void circularizeOrbitalVelocities();

    void setPhysicsMode(PhysicsMode newMode);
    PhysicsMode getPhysicsMode() const { return mode; }
};
