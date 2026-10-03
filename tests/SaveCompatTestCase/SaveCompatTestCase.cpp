/*
 *  SaveCompatTestCase.cpp
 *
 *  Regression tests for savegame backward compatibility across the
 *  Num_ItemID expansion history:
 *    Dune Legacy 0.99.x  → 41 items
 *    DuneCity 1.0.0–1.0.7 → 48 items
 *    DuneCity 1.0.8–1.0.10 → 52 items
 *    DuneCity 1.0.11+ (9811) → self-describing (count in stream)
 */

#include <catch2/catch_all.hpp>
#include <data.h>
#include <Definitions.h>
#include <misc/SaveCompat.h>

// ---------- constant checks ----------

TEST_CASE("Save compat: LEGACY_NUM_ITEM_ID_DUNELEGACY matches original Dune Legacy",
          "[save-compat][regression]") {
    REQUIRE(LEGACY_NUM_ITEM_ID_DUNELEGACY == 41);
}

TEST_CASE("Save compat: LEGACY_NUM_ITEM_ID_9810 matches v1.0.7 value",
          "[save-compat][regression]") {
    REQUIRE(LEGACY_NUM_ITEM_ID_9810 == 48);
}

TEST_CASE("Save compat: current Num_ItemID >= legacy",
          "[save-compat][regression]") {
    REQUIRE(Num_ItemID >= LEGACY_NUM_ITEM_ID_9810);
    REQUIRE(Num_ItemID >= LEGACY_NUM_ITEM_ID_DUNELEGACY);
}

TEST_CASE("Save compat: SAVEGAMEVERSION is 9811 or higher",
          "[save-compat][regression]") {
    REQUIRE(SAVEGAMEVERSION >= 9811);
}

TEST_CASE("Save compat: former city IDs remain reserved without shifting Tornie items",
          "[save-compat][regression]") {
    REQUIRE(ItemID_LegacyReserved48 == 48);
    REQUIRE(ItemID_LegacyReserved49 == 49);
    REQUIRE(ItemID_LegacyReserved50 == 50);
    REQUIRE(ItemID_LegacyReserved51 == 51);
    for(int id : {20, 21, 22, 23, 24, 25, 26, 48, 49, 50, 51}) {
        REQUIRE_FALSE(isStructure(id));
        REQUIRE_FALSE(isUnit(id));
    }
}

TEST_CASE("Save compat: Unit_Troopers is last legacy item at index 47",
          "[save-compat][regression]") {
    REQUIRE(Unit_Troopers == 47);
    REQUIRE(Unit_Troopers == LEGACY_NUM_ITEM_ID_9810 - 1);
}

TEST_CASE("Save compat: extended items classified correctly",
          "[save-compat][regression]") {
    REQUIRE(isStructure(Structure_AdvancedWindTrap));
    REQUIRE(isStructure(Structure_TechCenter));
    REQUIRE_FALSE(isUnit(Structure_TechCenter));

    REQUIRE(isUnit(Unit_RocketTrike));
    REQUIRE(isUnit(Unit_RebelHarvester));
    REQUIRE_FALSE(isStructure(Unit_RocketTrike));
}

// ---------- determineLegacySavedItemCount ----------

TEST_CASE("Save compat: determineLegacySavedItemCount maps correctly",
          "[save-compat][regression]") {

    SECTION("Dune Legacy 0.99.5 (v9806) → 41 items") {
        REQUIRE(determineLegacySavedItemCount(9806, "dunelegacy0.99.5") == 41);
    }

    SECTION("Dune Legacy 0.99.4 (v9805) → 41 items") {
        REQUIRE(determineLegacySavedItemCount(9805, "dunelegacy0.99.4") == 41);
    }

    SECTION("DuneCity 1.0.0 (v9810) → 48 items") {
        REQUIRE(determineLegacySavedItemCount(9810, "dunecity1.0.0") == 48);
    }

    SECTION("DuneCity 1.0.7 (v9810) → 48 items") {
        REQUIRE(determineLegacySavedItemCount(9810, "dunecity1.0.7") == 48);
    }

    SECTION("DuneCity 1.0.8 (v9810) → 52 items") {
        REQUIRE(determineLegacySavedItemCount(9810, "dunecity1.0.8") == 52);
    }

    SECTION("DuneCity 1.0.10 (v9810) → 52 items") {
        REQUIRE(determineLegacySavedItemCount(9810, "dunecity1.0.10") == 52);
    }

    SECTION("9811+ → 0 (self-describing)") {
        REQUIRE(determineLegacySavedItemCount(9811, "dunecity1.0.11") == 0);
        REQUIRE(determineLegacySavedItemCount(9812, "dunecity1.1.0") == 0);
    }
}
