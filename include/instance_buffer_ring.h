#pragma once

#include <array>

// GL-free publication policy used by the asteroid uploader. A frame's ranges
// belong to its slot contents, never to a skipped upload. Fence handles remain
// owned by the renderer; this policy tracks readers which have no valid fence.
class InstanceBufferRing {
public:
    static constexpr int kSize = 3;
    enum class Availability { Ready, Pending, Failed };
    struct Ranges {
        int high = 0, medium = 0, low = 0;
        int total() const { return high + medium + low; }
    };
    struct Frame {
        Ranges ranges;
        bool valid = false;
        bool untrackedReaders = false;
    };

    template<class Poll>
    int acquire(Poll poll) {
        for (int i = 0; i < kSize; ++i) {
            const int slot = (nextWrite + i) % kSize;
            if (frames[slot].untrackedReaders) continue;
            const auto status = poll(slot);
            if (status == Availability::Ready) return slot;
            if (status == Availability::Failed) frames[slot].untrackedReaders = true;
        }
        return -1;
    }
    bool canWait(int slot) const { return !frames[slot].untrackedReaders; }
    void waitFailed(int slot) { frames[slot].untrackedReaders = true; }
    bool publish(int slot, Ranges ranges, int capacity) {
        if (slot < 0 || slot >= kSize || frames[slot].untrackedReaders ||
            ranges.high < 0 || ranges.medium < 0 || ranges.low < 0 ||
            ranges.high > capacity || ranges.medium > capacity - ranges.high ||
            ranges.low > capacity - ranges.high - ranges.medium) return false;
        frames[slot].ranges = ranges;
        frames[slot].valid = true;
        lastValid = slot;
        nextWrite = (slot + 1) % kSize;
        return true;
    }
    // A fresh fence after the latest draw covers all earlier readers too.
    // If creation fails, even a signaled older fence cannot authorize writes.
    void submitted(int slot, bool fenceCreated) { frames[slot].untrackedReaders = !fenceCreated; }
    int nextSlot() const { return nextWrite; }
    int drawSlot() const { return lastValid; }
    const Frame& frame(int slot) const { return frames[slot]; }
    void reset() { frames = {}; nextWrite = 0; lastValid = -1; }

private:
    std::array<Frame, kSize> frames{};
    int nextWrite = 0;
    int lastValid = -1;
};
