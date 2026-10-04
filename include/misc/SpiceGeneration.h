#pragma once

#include <data.h>
#include <utility>

// One roll from the game's deterministic RNG. Keep the existing 20% variant
// budget while making all four supported colors reachable.
inline std::pair<int, int> generatedSpiceTerrainForRoll(int roll) {
    if(roll >= 0 && roll < 5) return {Terrain_GreenSpice, Terrain_ThickGreenSpice};
    if(roll >= 5 && roll < 10) return {Terrain_RedSpice, Terrain_ThickRedSpice};
    if(roll >= 10 && roll < 15) return {Terrain_PaleLilacSpice, Terrain_ThickPaleLilacSpice};
    if(roll >= 15 && roll < 20) return {Terrain_WhiteSpice, Terrain_ThickWhiteSpice};
    return {Terrain_Spice, Terrain_ThickSpice};
}
