#include <catch2/catch_all.hpp>
#include <INIMap/SpiceVariants.h>
#include <misc/SpiceGeneration.h>
#include <misc/SpiceEffects.h>

TEST_CASE("Generated spice includes every supported color within the original variant budget", "[map][spice]") {
    const std::pair<int, int> families[] = {
        {Terrain_Spice, Terrain_ThickSpice}, {Terrain_GreenSpice, Terrain_ThickGreenSpice},
        {Terrain_RedSpice, Terrain_ThickRedSpice}, {Terrain_PaleLilacSpice, Terrain_ThickPaleLilacSpice},
        {Terrain_WhiteSpice, Terrain_ThickWhiteSpice}
    };
    int counts[5]{};
    for(int roll = 0; roll < 100; ++roll) {
        const auto result = generatedSpiceTerrainForRoll(roll);
        const auto found = std::find(std::begin(families), std::end(families), result);
        REQUIRE(found != std::end(families));
        ++counts[found - std::begin(families)];
    }
    REQUIRE(counts[0] == 80);
    for(int color = 1; color < 5; ++color) REQUIRE(counts[color] == 5);
    REQUIRE(generatedSpiceTerrainForRoll(-1) == families[0]);
    REQUIRE(generatedSpiceTerrainForRoll(100) == families[0]);
}

TEST_CASE("Spice variants preserve terrain, thickness and the moderate replacement budget", "[map][spice]") {
    std::string terrain;
    for(int row = 0; row < 20; ++row) {
        terrain += "~~+++%OQ@g";
    }
    const auto result = SpiceVariants::apply(terrain, 10, 10, 123456789);
    size_t changed = 0;
    for(size_t index = 0; index < terrain.size(); ++index) {
        if(terrain[index] == result[index]) {
            continue;
        }
        ++changed;
        if(terrain[index] == '~') {
            REQUIRE((result[index] == 'g' || result[index] == 'r'));
        } else {
            REQUIRE(terrain[index] == '+');
            REQUIRE((result[index] == 'G' || result[index] == 'R'));
        }
    }
    REQUIRE(changed == 10); // 10% of the 100 normal spice tiles

    // Golden checksum shared with scripts/add-spice-variants.py: saved maps and
    // multiplayer peers must agree across standard libraries and architectures.
    uint32_t checksum = 2166136261U;
    for(const unsigned char tile : result) {
        checksum = (checksum ^ tile) * 16777619U;
    }
    REQUIRE(checksum == 2137090413U);
}

TEST_CASE("Spice pockets include both colours and depend only on the map seed", "[map][spice]") {
    const std::string terrain(1600, '~');
    const auto result = SpiceVariants::apply(terrain, 40, 10, 42);
    REQUIRE(result == SpiceVariants::apply(terrain, 40, 10, 42));
    REQUIRE(result != SpiceVariants::apply(terrain, 40, 10, 43));
    REQUIRE(std::count(result.begin(), result.end(), '~') == 1440);
    REQUIRE(std::count(result.begin(), result.end(), 'g') > 0);
    REQUIRE(std::count(result.begin(), result.end(), 'r') > 0);
}

TEST_CASE("Spice variants are opt-in, bounded and safe on tiny or invalid maps", "[map][spice]") {
    const std::string terrain(100, '~');
    REQUIRE(SpiceVariants::apply(terrain, 10, 0, 42) == terrain);
    REQUIRE(SpiceVariants::apply(terrain, 10, -1, 42) == terrain);
    REQUIRE(SpiceVariants::apply(terrain, 0, 10, 42) == terrain);
    REQUIRE(SpiceVariants::apply(terrain, 3, 10, 42) == terrain);
    REQUIRE(SpiceVariants::apply("~~~~~~~~~", 3, 10, 42) == "~~~~~~~~~");
    REQUIRE(SpiceVariants::apply("%%%OOOQQQ", 3, 10, 42) == "%%%OOOQQQ");
    const auto bounded = SpiceVariants::apply(terrain, 10, 100, 42);
    REQUIRE(std::count(bounded.begin(), bounded.end(), '~') == 85);
}

TEST_CASE("Version two provided maps include all four variants without changing the old algorithm","[map][spice]") {
    const std::string terrain(1600,'~');
    for(uint32_t seed:{0U,42U,123456789U}) {
        const auto result=SpiceVariants::apply(terrain,40,10,seed,2);
        REQUIRE(std::count(result.begin(),result.end(),'~')==1440);
        for(char variant:std::string("rglw"))REQUIRE(std::count(result.begin(),result.end(),variant)>0);
        REQUIRE(result==SpiceVariants::apply(terrain,40,10,seed,2));
    }
}
TEST_CASE("Healing spice is limited to one point per simulated second independent of unit ID","[spice][balance]") {
    constexpr uint64_t interval=(1000+GAMESPEED_DEFAULT-1)/GAMESPEED_DEFAULT;
    for(uint32_t object:{0U,1U,62U,63U,123456U}) {
        int heals=0;
        for(uint64_t cycle=0;cycle<interval*10;++cycle)heals+=spiceHealingTick(cycle,object);
        REQUIRE(heals==10);
        REQUIRE(interval*GAMESPEED_DEFAULT>=1000);
    }
}
