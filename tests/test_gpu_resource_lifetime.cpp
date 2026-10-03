#include "catch.hpp"
#include "instance_buffer_ring.h"
#include "asteroid_belt.h"
#include "atmosphere_effects.h"
#include "benchmark_runner.h"
#include "black_hole.h"
#include "gl_primitives.h"
#include "immediate_batch.h"
#include "lod_manager.h"
#include "modern_mesh.h"
#include "portal_render_target.h"
#include "post_processing.h"
#include "scene_renderer.h"
#include "shader_utils.h"
#include "wormhole.h"
#include <GLFW/glfw3.h>
#include <type_traits>

// These objects own GL handles and must never duplicate ownership by copying.
static_assert(!std::is_copy_constructible<mesh::GPUMesh>::value);
static_assert(!std::is_copy_constructible<ImmediateBatch>::value);
static_assert(!std::is_copy_constructible<BlackHole>::value);
static_assert(!std::is_copy_constructible<Wormhole>::value);
static_assert(!std::is_copy_constructible<AsteroidBelt>::value);
static_assert(!std::is_copy_constructible<AtmosphereEffects>::value);
static_assert(!std::is_copy_constructible<SceneRenderer>::value);
static_assert(!std::is_copy_constructible<PostProcessingPipeline>::value);
static_assert(!std::is_copy_constructible<PortalRenderTarget>::value);
static_assert(!std::is_copy_constructible<BenchmarkGPUTimer>::value);
static_assert(!std::is_copy_constructible<lod::LODSphereMesh>::value);
static_assert(!std::is_copy_constructible<glprims::FullscreenQuad>::value);

TEST_CASE("Asteroid timeout retains the last uploaded frame's LOD ranges", "[gpu_lifetime][asteroid_ring]") {
    InstanceBufferRing ring;
    for (int slot = 0; slot < InstanceBufferRing::kSize; ++slot) {
        REQUIRE(ring.publish(slot, {slot + 1, 4, 7}, 100));
        ring.submitted(slot, true);
    }
    // Task 3 regression: a skipped upload must not publish the new CPU counts.
    REQUIRE(ring.acquire([](int) { return InstanceBufferRing::Availability::Pending; }) == -1);
    REQUIRE(ring.drawSlot() == 2);
    REQUIRE(ring.frame(ring.drawSlot()).ranges.high == 3);
    REQUIRE(ring.frame(ring.drawSlot()).ranges.medium == 4);
    REQUIRE(ring.frame(ring.drawSlot()).ranges.low == 7);
    ring.submitted(ring.drawSlot(), true); // fallback draws need a fresh fence too
    REQUIRE(ring.acquire([](int slot) {
        return slot == 1 ? InstanceBufferRing::Availability::Ready : InstanceBufferRing::Availability::Pending;
    }) == 1);
    REQUIRE(ring.publish(1, {0, 2, 8}, 100));
    REQUIRE(ring.drawSlot() == 1);
    REQUIRE(ring.frame(1).ranges.total() == 10);
}

TEST_CASE("A failed latest-reader fence blocks overwrite despite a ready older fence", "[gpu_lifetime][asteroid_ring]") {
    InstanceBufferRing ring;
    REQUIRE(ring.publish(0, {5, 2, 1}, 100));
    ring.submitted(0, false);
    REQUIRE_FALSE(ring.canWait(0));
    REQUIRE(ring.acquire([](int slot) {
        return slot == 0 ? InstanceBufferRing::Availability::Ready : InstanceBufferRing::Availability::Pending;
    }) == -1);
    REQUIRE_FALSE(ring.publish(0, {1, 1, 1}, 100));
    REQUIRE(ring.frame(0).ranges.total() == 8);
    // A later successful fence covers every prior draw of the same slot.
    ring.submitted(0, true);
    REQUIRE(ring.acquire([](int slot) {
        return slot == 0 ? InstanceBufferRing::Availability::Ready : InstanceBufferRing::Availability::Pending;
    }) == 0);
}

TEST_CASE("Failed waits and invalid publication cannot authorize new instance contents", "[gpu_lifetime][asteroid_ring]") {
    InstanceBufferRing ring;
    REQUIRE(ring.acquire([](int) { return InstanceBufferRing::Availability::Failed; }) == -1);
    REQUIRE(ring.drawSlot() == -1); // no previously uploaded frame means no draw
    REQUIRE_FALSE(ring.canWait(0));
    ring.reset();
    REQUIRE(ring.publish(0, {1, 2, 3}, 6));
    REQUIRE_FALSE(ring.publish(1, {1, 2, 4}, 6));
    REQUIRE_FALSE(ring.publish(1, {-1, 0, 0}, 6));
    REQUIRE_FALSE(ring.publish(3, {0, 0, 0}, 6));
    REQUIRE(ring.drawSlot() == 0);
    ring.reset();
    REQUIRE(ring.drawSlot() == -1);
    REQUIRE(ring.acquire([](int) { return InstanceBufferRing::Availability::Ready; }) == 0);
}

namespace {
// No GL calls at test-file static initialization; context-owned caches are
// released even when a REQUIRE aborts the body of a test.
struct LifetimeContext {
    bool glfwReady = false;
    bool valid = false;
    GLFWwindow* window = nullptr;
    GLFWwindow* second = nullptr;
    LifetimeContext() {
        glfwReady = glfwInit() == GLFW_TRUE;
        if (!glfwReady) return;
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(64, 64, "GPU lifetime regression", nullptr, nullptr);
        if (!window) return;
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        valid = glewInit() == GLEW_OK && glCreateVertexArrays && glCreateFramebuffers;
        if (valid) while (glGetError() != GL_NO_ERROR) {}
    }
    ~LifetimeContext() {
        for (GLFWwindow* context : {second, window}) {
            if (!context) continue;
            glfwMakeContextCurrent(context);
            if (valid) {
                lod::LODManager::releaseCurrentContext();
                glprims::destroySharedResources();
            }
            glfwDestroyWindow(context);
        }
        if (glfwReady) glfwTerminate();
    }
};

// Override only the driver synchronization boundary to force the real render
// path's timeout branch deterministically; drawing and buffer uploads stay real.
struct PendingSyncs {
    PFNGLCLIENTWAITSYNCPROC originalWait = glClientWaitSync;
    PFNGLFENCESYNCPROC originalFence = glFenceSync;
    int fencesCreated = 0;
    inline static PendingSyncs* active = nullptr;
    PendingSyncs() {
        active = this;
        glClientWaitSync = &timeout;
        glFenceSync = &fence;
    }
    ~PendingSyncs() {
        glClientWaitSync = originalWait;
        glFenceSync = originalFence;
        active = nullptr;
    }
    static GLenum GLAPIENTRY timeout(GLsync, GLbitfield, GLuint64) { return GL_TIMEOUT_EXPIRED; }
    static GLsync GLAPIENTRY fence(GLenum condition, GLbitfield flags) {
        ++active->fencesCreated;
        return active->originalFence(condition, flags);
    }
};
struct ProgramOwner {
    GLuint handle = 0;
    ~ProgramOwner() { if (handle) glDeleteProgram(handle); }
};
}

TEST_CASE("Asteroid render timeout preserves uploaded LOD counts and fences its new readers", "[gpu_lifetime][gl][asteroid_ring]") {
    LifetimeContext ctx;
    if (!ctx.valid) { WARN("OpenGL 4.5 context unavailable; GL lifetime case not executed."); return; }
    ProgramOwner shader;
    shader.handle = loadProgramFromFiles("shaders/asteroid.vert", "shaders/asteroid.frag");
    REQUIRE(shader.handle != 0);
    AsteroidBelt belt(800, 15.0f, 17.8f, nullptr);
    auto& manager = lod::LODManager::instance();
    for (int frame = 0; frame < InstanceBufferRing::kSize; ++frame) {
        manager.beginFrame();
        belt.render(0.0f, shader.handle, glm::mat4(1.0f), glm::mat4(1.0f),
                    glm::vec3(0.0f), glm::vec3(0.0f), true, 1);
    }
    const int previousCount = belt.getTelemetry().activeAsteroids;
    REQUIRE(previousCount > 0);
    // Persistent mapping is optional. The mutable path has driver-ordered writes.
    if (belt.getTelemetry().backendName.find("Persistent-Mapped") == std::string::npos) {
        WARN("Persistent mapping unavailable; timeout regression case not executed."); return;
    }
    PendingSyncs pending;
    manager.beginFrame();
    // Both population and LOD differ from the buffered frame (the original bug).
    belt.render(1.0f, shader.handle, glm::mat4(1.0f), glm::mat4(1.0f),
                glm::vec3(0.0f), glm::vec3(0.0f), true, 3);
    REQUIRE(belt.getTelemetry().activeAsteroids == previousCount);
    REQUIRE(manager.getAsteroidTierCounts()[lod::ASTEROID_LOD_HIGH] == previousCount);
    REQUIRE(manager.getAsteroidTierCounts()[lod::ASTEROID_LOD_LOW] == 0);
    REQUIRE(belt.getTelemetry().ringWaitTimeouts == 1);
    REQUIRE(belt.getTelemetry().reusedFrameDraws == 1);
    REQUIRE(pending.fencesCreated == 1); // fallback readers replace the older fence
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("Shared primitive and LOD owners are isolated by context and released before teardown", "[gpu_lifetime][gl]") {
    LifetimeContext ctx;
    if (!ctx.valid) { WARN("OpenGL 4.5 context unavailable; GL lifetime case not executed."); return; }
    auto& quad = glprims::sharedFullscreenQuad();
    quad.ensure();
    const GLuint firstVAO = quad.vao, firstVBO = quad.vbo;
    REQUIRE(firstVAO != 0);
    lod::LODManager::instance().init();
    const GLuint firstLOD = lod::LODManager::instance().getSphereMesh(lod::SPHERE_LOD_LOW).vao;
    REQUIRE(firstLOD != 0);

    ctx.second = glfwCreateWindow(64, 64, "Second lifetime context", nullptr, nullptr);
    REQUIRE(ctx.second != nullptr);
    glfwMakeContextCurrent(ctx.second);
    REQUIRE(glprims::sharedFullscreenQuad().vao == 0);
    REQUIRE(lod::LODManager::instance().getSphereMesh(lod::SPHERE_LOD_LOW).vao == 0);
    glprims::sharedFullscreenQuad().ensure();
    lod::LODManager::instance().init();
    glprims::destroySharedResources();
    lod::LODManager::releaseCurrentContext();

    glfwMakeContextCurrent(ctx.window);
    REQUIRE(glIsVertexArray(firstVAO) == GL_TRUE);
    REQUIRE(glIsBuffer(firstVBO) == GL_TRUE);
    REQUIRE(glIsVertexArray(firstLOD) == GL_TRUE);
    glprims::destroySharedResources();
    lod::LODManager::releaseCurrentContext();
    REQUIRE(glIsVertexArray(firstVAO) == GL_FALSE);
    REQUIRE(glIsBuffer(firstVBO) == GL_FALSE);
    REQUIRE(glIsVertexArray(firstLOD) == GL_FALSE);
    glprims::destroySharedResources();
    lod::LODManager::releaseCurrentContext(); // repeated teardown is a no-op
}

TEST_CASE("Black hole teardown deletes owned programs and preserves borrowed programs", "[gpu_lifetime][gl]") {
    LifetimeContext ctx;
    if (!ctx.valid) { WARN("OpenGL 4.5 context unavailable; GL lifetime case not executed."); return; }
    ImmediateBatch batch;
    REQUIRE(batch.init(kFlatVS, kFlatFS));
    REQUIRE(batch.init(kFlatVS, kFlatFS)); // repeated init must release its former objects
    batch.destroy();
    batch.destroy();
    REQUIRE_FALSE(batch.isReady());

    const GLuint borrowed = linkProgram(compileShader(GL_VERTEX_SHADER, kFlatVS),
                                        compileShader(GL_FRAGMENT_SHADER, kFlatFS));
    REQUIRE(borrowed != 0);
    BlackHole hole;
    hole.initShader(borrowed);
    hole.initLensingShader(borrowed);
    hole.cleanup();
    REQUIRE(glIsProgram(borrowed) == GL_TRUE);
    glDeleteProgram(borrowed);
    hole.initShader();
    hole.initLensingShader();
    const GLuint ownedMain = hole.program, ownedLens = hole.lensingProgram;
    REQUIRE(ownedMain != 0);
    REQUIRE(ownedLens != 0);
    hole.cleanup();
    hole.cleanup();
    REQUIRE(hole.program == 0);
    REQUIRE(hole.lensingProgram == 0);
    REQUIRE(glIsProgram(ownedMain) == GL_FALSE);
    REQUIRE(glIsProgram(ownedLens) == GL_FALSE);
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("Wormhole reinitialization and partial shader failure release their owned objects", "[gpu_lifetime][gl]") {
    LifetimeContext ctx;
    if (!ctx.valid) { WARN("OpenGL 4.5 context unavailable; GL lifetime case not executed."); return; }
    Wormhole wormhole;
    wormhole.initGeometry();
    const GLuint oldVAO = wormhole.sphereVAO;
    REQUIRE(oldVAO != 0);
    wormhole.initGeometry();
    REQUIRE(wormhole.sphereVAO != 0);
    // Names can be recycled during init; inspect the final live names instead.
    const GLuint vao = wormhole.sphereVAO, vbo = wormhole.sphereVBO, ebo = wormhole.sphereEBO;
    wormhole.cleanup();
    wormhole.cleanup();
    REQUIRE(glIsVertexArray(vao) == GL_FALSE);
    REQUIRE(glIsBuffer(vbo) == GL_FALSE);
    REQUIRE(glIsBuffer(ebo) == GL_FALSE);
    REQUIRE(wormhole.diskVAO == 0);
    REQUIRE(wormhole.archVAO == 0);
    const GLuint successfulShader = compileShader(GL_VERTEX_SHADER, kFlatVS);
    REQUIRE(successfulShader != 0);
    REQUIRE(linkProgram(successfulShader, 0) == 0);
    REQUIRE(glIsShader(successfulShader) == GL_FALSE);
    mesh::GPUMesh mesh;
    mesh.buildFromStrip({}); // formerly underflowed the reserve size
    REQUIRE_FALSE(mesh.isBuilt());
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("CPU resource construction remains GL-free after a previous context was destroyed", "[gpu_lifetime][gl]") {
    { LifetimeContext prior; }
    // GLEW entry points may remain non-null, so pointers alone are insufficient.
    REQUIRE(glfwGetCurrentContext() == nullptr);
    AsteroidBelt belt(50, 15.0f, 17.8f, nullptr);
    AtmosphereEffects atmosphere;
    ImmediateBatch batch;
    PortalRenderTarget target;
    PostProcessingPipeline post;
    REQUIRE_FALSE(batch.init(kFlatVS, kFlatFS));
    REQUIRE_FALSE(target.init(64, 64));
    REQUIRE_FALSE(post.init(64, 64));
    REQUIRE(belt.getAsteroidCount() == 50);
}
