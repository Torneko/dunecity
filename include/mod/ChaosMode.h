#pragma once

#include <ObjectData.h>
#include <array>

class InputStream;
class OutputStream;

// One donor faction for each production/defence building of each faction.
// Colour, voices, ownership and the unmodified unit catalogue remain separate.
class ChaosMode {
public:
    ChaosMode() { reset(); }
    void reset();
    void generate(ObjectData& objectData, bool enabled, Uint32 seed);
    void save(OutputStream& stream) const;
    void load(InputStream& stream);
    bool isEnabled() const { return enabled; }
    int getTechnologyHouse(int house, int structure) const;
    static bool includesStructure(int item);

    static constexpr std::array<int, 15> Structures{{
        Structure_Barracks, Structure_WOR, Structure_LightFactory,
        Structure_HeavyFactory, Structure_HighTechFactory, Structure_StarPort,
        Structure_Worfinery, Structure_LoveFactory, Structure_ChaosFactory,
        Structure_Palace, Structure_GunTurret, Structure_RocketTurret,
        Structure_Scoutpost, Structure_Flamepost, Structure_Chemipost
    }};

private:
    bool enabled = false;
    std::array<std::array<int, Structures.size()>, NUM_HOUSES> donors{};
};
