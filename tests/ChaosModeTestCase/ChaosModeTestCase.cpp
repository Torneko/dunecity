#include <catch2/catch_all.hpp>
#include <mod/ChaosMode.h>
#include <misc/IMemoryStream.h>
#include <misc/OMemoryStream.h>

// The unit target uses synthetic data; integration checks load the real mod INIs.
ObjectData::ObjectData() {
    for(auto& row : data) for(auto& item : row) item = {};
}
ObjectData::~ObjectData() = default;

static void populate(ObjectData& data) {
    for(int item = 0; item < Num_ItemID; ++item) {
        for(int house = 0; house < NUM_HOUSES; ++house) {
            auto& entry = data.data[item][house];
            entry.enabled = true;
            entry.techLevel = 1 + item % 9;
            entry.builder = Structure_ConstructionYard;
            entry.price = 100 * item + house;
            entry.hitpoints = 1000 + house;
        }
    }
}

TEST_CASE("Chaos Mode preserves tiers, ownership data and excluded technologies", "[chaos-mode]") {
    ObjectData data;
    populate(data);
    const ObjectData original = data;
    data.data[Structure_HeavyFactory][HOUSE_ATREIDES].techLevel = 9;
    ChaosMode chaos;
    chaos.generate(data, true, 89123);
    REQUIRE(chaos.isEnabled());
    for(int item = 0; item < Num_ItemID; ++item) {
        for(int house = 0; house < NUM_HOUSES; ++house) {
            if(!ChaosMode::includesStructure(item)) {
                REQUIRE(chaos.getTechnologyHouse(house, item) == house);
                REQUIRE(data.data[item][house].price == original.data[item][house].price);
                REQUIRE(data.data[item][house].hitpoints == original.data[item][house].hitpoints);
            } else {
                const int donor = chaos.getTechnologyHouse(house, item);
                if(item == Structure_HeavyFactory && house == HOUSE_ATREIDES) {
                    REQUIRE(donor == HOUSE_ATREIDES);
                    REQUIRE(data.data[item][house].techLevel == 9);
                } else {
                    REQUIRE(donor != house);
                    REQUIRE(data.data[item][house].techLevel == original.data[item][house].techLevel);
                    REQUIRE(data.data[item][house].price == original.data[item][donor].price);
                }
            }
        }
    }
    REQUIRE_FALSE(ChaosMode::includesStructure(Structure_IX));
    REQUIRE_FALSE(ChaosMode::includesStructure(Structure_Refinery));
    REQUIRE_FALSE(ChaosMode::includesStructure(Structure_ConstructionYard));
    REQUIRE_FALSE(ChaosMode::includesStructure(Structure_AdvancedWindTrap));
}

TEST_CASE("Chaos Mode has deterministic donors and persists the exact draw", "[chaos-mode][save-compat]") {
    ObjectData first, second, third;
    populate(first); populate(second); populate(third);
    ChaosMode a, b, c;
    a.generate(first, true, 777);
    b.generate(second, true, 777);
    c.generate(third, true, 778);
    bool differs = false;
    OMemoryStream output; output.open(); a.save(output);
    IMemoryStream input(output.getData(), output.getDataLength());
    ChaosMode loaded; loaded.load(input);
    REQUIRE(loaded.isEnabled());
    for(int house = 0; house < NUM_HOUSES; ++house) for(int item : ChaosMode::Structures) {
        REQUIRE(a.getTechnologyHouse(house, item) == b.getTechnologyHouse(house, item));
        REQUIRE(a.getTechnologyHouse(house, item) == loaded.getTechnologyHouse(house, item));
        differs |= a.getTechnologyHouse(house, item) != c.getTechnologyHouse(house, item);
    }
    REQUIRE(differs);
    loaded.reset();
    REQUIRE_FALSE(loaded.isEnabled());
    REQUIRE(loaded.getTechnologyHouse(HOUSE_ATREIDES, Structure_HeavyFactory) == HOUSE_ATREIDES);
}

TEST_CASE("Disabled Chaos Mode preserves the normal rules and option hash", "[chaos-mode]") {
    SettingsClass::GameOptionsClass options;
    REQUIRE_FALSE(options.chaosMode);
    const auto normalHash = options.getHash();
    const auto normalOptions = options;
    options.chaosMode = true;
    REQUIRE(options != normalOptions);
    REQUIRE(options.getHash() != normalHash);
    options.chaosMode = false;
    REQUIRE(options.getHash() == normalHash);
    ObjectData data; populate(data);
    ChaosMode chaos; chaos.generate(data, false, 444);
    REQUIRE_FALSE(chaos.isEnabled());
    REQUIRE(data.data[Structure_HeavyFactory][HOUSE_ATREIDES].price == 100 * Structure_HeavyFactory + HOUSE_ATREIDES);
    OMemoryStream output; output.open(); chaos.save(output);
    REQUIRE(output.getDataLength() == 1);
    IMemoryStream input(output.getData(), output.getDataLength());
    chaos.load(input);
    REQUIRE_FALSE(chaos.isEnabled());
}
