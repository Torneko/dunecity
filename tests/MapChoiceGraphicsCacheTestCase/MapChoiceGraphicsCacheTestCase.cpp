#include <catch2/catch_test_macros.hpp>
#include <FileClasses/MapChoiceGraphicsCache.h>

#include <array>
#include <memory>

namespace {

constexpr int masterColorSlot = 0; // HOUSE_HARKONNEN
using Cache = std::array<std::array<std::unique_ptr<int>, 18>, 28>;

void populate(Cache& cache) {
    for(auto& piece : cache) {
        for(auto& entry : piece) {
            entry = std::make_unique<int>(1);
        }
    }
}

} // namespace

TEST_CASE("Harkonnen map cache keeps the master surfaces on repeated visits",
          "[campaign][map-choice][regression]") {
    Cache surfaces;
    Cache textures;
    populate(surfaces);
    populate(textures);

    std::array<const int*, 28> originals;
    for(std::size_t piece = 0; piece < surfaces.size(); ++piece) {
        originals[piece] = surfaces[piece][masterColorSlot].get();
    }

    for(int visit = 0; visit < 2; ++visit) {
        MapChoiceGraphicsCache::invalidatePieces(
            surfaces, textures, masterColorSlot, masterColorSlot);
        for(std::size_t piece = 0; piece < surfaces.size(); ++piece) {
            REQUIRE(surfaces[piece][masterColorSlot].get() == originals[piece]);
            REQUIRE_FALSE(textures[piece][masterColorSlot]);
            REQUIRE(surfaces[piece][1]);
            REQUIRE(textures[piece][1]);
        }
    }
}

TEST_CASE("Map cache clears only the selected derived colour",
          "[campaign][map-choice][regression]") {
    Cache surfaces;
    Cache textures;
    populate(surfaces);
    populate(textures);

    for(int colorSlot = 1; colorSlot < 18; ++colorSlot) {
        MapChoiceGraphicsCache::invalidatePieces(
            surfaces, textures, colorSlot, masterColorSlot);
        for(std::size_t piece = 0; piece < surfaces.size(); ++piece) {
            REQUIRE_FALSE(surfaces[piece][colorSlot]);
            REQUIRE_FALSE(textures[piece][colorSlot]);
            REQUIRE(surfaces[piece][masterColorSlot]);
            REQUIRE(textures[piece][masterColorSlot]);
        }
    }
}

TEST_CASE("Invalid map colour slots leave the cache intact",
          "[campaign][map-choice][regression]") {
    Cache surfaces;
    Cache textures;
    populate(surfaces);
    populate(textures);

    for(int colorSlot : {-1, 18}) {
        MapChoiceGraphicsCache::invalidatePieces(
            surfaces, textures, colorSlot, masterColorSlot);
    }
    for(std::size_t piece = 0; piece < surfaces.size(); ++piece) {
        for(std::size_t color = 0; color < surfaces[piece].size(); ++color) {
            REQUIRE(surfaces[piece][color]);
            REQUIRE(textures[piece][color]);
        }
    }
}
