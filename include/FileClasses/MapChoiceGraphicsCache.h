#ifndef MAPCHOICEGRAPHICSCACHE_H
#define MAPCHOICEGRAPHICSCACHE_H

#include <array>
#include <cstddef>

namespace MapChoiceGraphicsCache {

template<typename Surface, typename Texture, std::size_t Pieces, std::size_t Colors>
void invalidatePieces(std::array<std::array<Surface, Colors>, Pieces>& surfaces,
                      std::array<std::array<Texture, Colors>, Pieces>& textures,
                      int colorSlot, int masterColorSlot) {
    if(colorSlot < 0 || static_cast<std::size_t>(colorSlot) >= Colors) {
        return;
    }

    for(std::size_t piece = 0; piece < Pieces; ++piece) {
        // Master surfaces are the source for every recoloured house.
        if(colorSlot != masterColorSlot) {
            surfaces[piece][colorSlot].reset();
        }
        textures[piece][colorSlot].reset();
    }
}

} // namespace MapChoiceGraphicsCache

#endif // MAPCHOICEGRAPHICSCACHE_H
