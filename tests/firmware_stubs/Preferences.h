#pragma once
#include <cstddef>
#include <cstring>
#include <cstdint>
struct Preferences {
    inline static uint16_t saved[5] = {};
    inline static bool available = false;
    bool begin(const char*, bool = false) { return true; }
    void end() {}
    size_t getBytesLength(const char*) { return available ? sizeof(saved) : 0; }
    size_t getBytes(const char*, void* data, size_t size) {
        if (!available || size < sizeof(saved)) return 0;
        memcpy(data, saved, sizeof(saved));
        return sizeof(saved);
    }
    size_t putBytes(const char*, const void* data, size_t size) {
        if (size != sizeof(saved)) return 0;
        memcpy(saved, data, size);
        available = true;
        return size;
    }
};
