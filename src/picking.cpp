#include "picking.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Ray RaycastPicker::createRayFromMouse(float mouseX, float mouseY, float screenWidth, float screenHeight,
                                      const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec3& cameraPos) {
    // Convert to Normalized Device Coordinates (NDC)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;

    glm::vec4 clipCoords(x, y, -1.0f, 1.0f);

    // Clip to Eye coordinates
    glm::mat4 invProj = glm::inverse(projMatrix);
    glm::vec4 eyeCoords = invProj * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

    // Eye to World coordinates
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec4 worldCoords = invView * eyeCoords;
    glm::vec3 rayDir = glm::normalize(glm::vec3(worldCoords));

    Ray ray;
    ray.origin = cameraPos;
    ray.direction = rayDir;
    return ray;
}

float RaycastPicker::intersectRaySphere(const Ray& ray, const glm::vec3& sphereCenter, float sphereRadius) {
    glm::vec3 oc = ray.origin - sphereCenter;
    float b = glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - c;

    if (discriminant < 0.0f) {
        return -1.0f;
    }

    float sqrtDisc = sqrt(discriminant);
    float t0 = -b - sqrtDisc;
    float t1 = -b + sqrtDisc;

    if (t0 > 0.001f) return t0;
    if (t1 > 0.001f) return t1;
    return -1.0f;
}

int RaycastPicker::pickClosestBody(const Ray& ray, const std::vector<PickableBody>& bodies, std::string& outName, float& outDist) {
    int bestIdx = -999;
    float closestT = 1e9f;

    for (const auto& body : bodies) {
        float effectiveRadius = std::max(body.radius * 1.4f, 0.5f);
        float t = intersectRaySphere(ray, body.position, effectiveRadius);
        if (t > 0.0f && t < closestT) {
            closestT = t;
            bestIdx = body.index;
            outName = body.name;
            outDist = t;
        }
    }

    return bestIdx;
}
