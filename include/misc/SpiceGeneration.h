#pragma once

#include <data.h>
#include <utility>

// One roll from the game's deterministic RNG. Keep the existing 20% variant
// budget while making all four supported colors reachable.
inline std::pair<int, int> generatedSpiceTerrainForRoll(int roll, unsigned mask = 15) {
    if(roll >= 0 && roll < 5 && (mask & 1)) return {Terrain_GreenSpice, Terrain_ThickGreenSpice};
    if(roll >= 5 && roll < 10 && (mask & 2)) return {Terrain_RedSpice, Terrain_ThickRedSpice};
    if(roll >= 10 && roll < 15 && (mask & 4)) return {Terrain_PaleLilacSpice, Terrain_ThickPaleLilacSpice};
    if(roll >= 15 && roll < 20 && (mask & 8)) return {Terrain_WhiteSpice, Terrain_ThickWhiteSpice};
    return {Terrain_Spice, Terrain_ThickSpice};
}

inline int allowedSpiceTerrain(int type, unsigned mask) {
    unsigned family = 0;
    int normal = Terrain_Spice;
    switch(type) {
        case Terrain_GreenSpice: family = 1; break;
        case Terrain_ThickGreenSpice: family = 1; normal = Terrain_ThickSpice; break;
        case Terrain_GreenSpiceBloom: family = 1; normal = Terrain_SpiceBloom; break;
        case Terrain_RedSpice: family = 2; break;
        case Terrain_ThickRedSpice: family = 2; normal = Terrain_ThickSpice; break;
        case Terrain_RedSpiceBloom: family = 2; normal = Terrain_SpiceBloom; break;
        case Terrain_PaleLilacSpice: family = 4; break;
        case Terrain_ThickPaleLilacSpice: family = 4; normal = Terrain_ThickSpice; break;
        case Terrain_PaleLilacSpiceBloom: family = 4; normal = Terrain_SpiceBloom; break;
        case Terrain_WhiteSpice: family = 8; break;
        case Terrain_ThickWhiteSpice: family = 8; normal = Terrain_ThickSpice; break;
        case Terrain_WhiteSpiceBloom: family = 8; normal = Terrain_SpiceBloom; break;
        default: return type;
    }
    return (mask & family) ? type : normal;
}
