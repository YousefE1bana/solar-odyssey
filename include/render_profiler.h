#pragma once

#include <atomic>

/**
 * @brief Thread-safe performance profiler tracking software-instrumented actual submitted OpenGL draw calls.
 * 
 * Increments exactly once on every actual OpenGL draw command submitted to the GPU:
 * glDrawArrays, glDrawElements, glDrawArraysInstanced, glDrawElementsInstanced, etc.
 */
class RenderProfiler {
public:
    static RenderProfiler& instance() {
        static RenderProfiler s_instance;
        return s_instance;
    }

    void beginFrame() {
        drawCallCount.store(0, std::memory_order_relaxed);
    }

    inline void recordDrawCall(int count = 1) {
        drawCallCount.fetch_add(count, std::memory_order_relaxed);
    }

    int getDrawCallCount() const {
        return drawCallCount.load(std::memory_order_relaxed);
    }

private:
    RenderProfiler() : drawCallCount(0) {}
    std::atomic<int> drawCallCount;
};
