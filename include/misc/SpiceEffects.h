#pragma once
#include <Definitions.h>
#include <cstdint>

// One HP per simulated second while actively harvesting healing spice.
// Object-ID phase staggering requires no extra state in saves or network messages.
inline bool spiceHealingTick(std::uint64_t cycle, std::uint32_t object) {
    constexpr auto interval = (1000 + GAMESPEED_DEFAULT - 1) / GAMESPEED_DEFAULT;
    return cycle % interval == object % interval;
}
