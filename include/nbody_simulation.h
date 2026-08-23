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
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 1.0f;
    float radius = 1.0f;
    bool isStatic = false;
    std::string parentBody = ""; // Non-empty for satellites (e.g. Moon -> Earth)
};

class NBodySimulation {
public:
    PhysicsMode mode = PHYSICS_KEPLERIAN;
    float gravitationalConstant = 4000.0f; // Calibrated central G*M_sun parameter
    float softening = 0.15f;               // Softening length to prevent singularities
    float fixedDeltaTime = 0.0025f;        // Fixed numerical integration timestep (sim-days)
    float timeAccumulator = 0.0f;
    float maxAccumulatorCap = 0.5f;        // Max accumulator to prevent spiral-of-death on extreme lags

    std::vector<NBodyObject> bodies;
    std::map<std::string, size_t> nameToIndex;

    NBodySimulation();

    void reset();
    void addBody(const std::string& name, float mass, float radius, bool isStatic = false, const std::string& parent = "");
    
    NBodyObject* getBody(const std::string& name);
    const NBodyObject* getBody(const std::string& name) const;
    glm::vec3 getBodyPosition(const std::string& name) const;
    glm::vec3 getBodyVelocity(const std::string& name) const;

    // Direct pairwise gravitational acceleration calculation
    glm::vec3 computeAccelerationForPoint(const glm::vec3& point, float pointMass = 0.0f) const;
    void computeAllAccelerations();

    // Symplectic Velocity Verlet (Leapfrog) Step
    void stepVerlet(float dt);

    // Main update loop with fixed-step accumulation
    void update(float deltaTime, float timeMultiplier);

    // Smooth C1 transition from Keplerian motion function
    void initializeFromKeplerian(
        const std::function<glm::vec3(const std::string&, float)>& positionFunc,
        float currentSimTime,
        float speedScale = 1.0f
    );

    void setPhysicsMode(PhysicsMode newMode);
    PhysicsMode getPhysicsMode() const { return mode; }
};
