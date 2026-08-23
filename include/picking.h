#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct PickableBody {
    std::string name;
    int index; // -1 for Sun, 0..7 for planets, 100 for Moon
    glm::vec3 position;
    float radius;
    float clickRadius; // Slightly enlarged for comfortable picking
};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

class RaycastPicker {
public:
    static Ray createRayFromMouse(float mouseX, float mouseY, float screenWidth, float screenHeight,
                                  const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec3& cameraPos);

    // Ray-Sphere intersection test; returns distance t >= 0 if hit, else -1.0f
    static float intersectRaySphere(const Ray& ray, const glm::vec3& sphereCenter, float sphereRadius);

    // Test a list of pickable bodies and return index of closest hit, or -999 if none
    static int pickClosestBody(const Ray& ray, const std::vector<PickableBody>& bodies, std::string& outName, float& outDist);
};
