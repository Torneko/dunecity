/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATA_H
#define DATA_H

// data definitions
typedef enum {
    Bullet_DRocket = 0,
    Bullet_LargeRocket = 1,
    Bullet_Rocket = 2,
    Bullet_TurretRocket = 3,
    Bullet_ShellSmall = 4,
    Bullet_ShellMedium = 5,
    Bullet_ShellLarge = 6,
    Bullet_ShellTurret = 7,
    Bullet_SmallRocket = 8,
    Bullet_Sonic = 9,
    Bullet_Sandworm = 10,
    Bullet_Flame = 11,            ///< Tornie: launcher-like fire missile with flame area damage
    Bullet_SonicTrike = 12,        ///< Tornie: shorter and weaker Sonic Trike wave
    Bullet_Heal = 13              ///< Jericho: Deviator-style healing missile with direct flight
} BulletID_enum;

typedef enum {
    Explosion_Small = 0,
    Explosion_Medium1 = 1,
    Explosion_Medium2 = 2,
    Explosion_Large1 = 3,
    Explosion_Large2 = 4,
    Explosion_Gas = 5,
    Explosion_ShellSmall = 6,
    Explosion_ShellMedium = 7,
    Explosion_ShellLarge = 8,
    Explosion_SmallUnit = 9,
    Explosion_Flames = 10,
    Explosion_SpiceBloom = 11,
    Explosion_FlameImpact = 12,       ///< Tornie: damaging ground fire without the destroyed-vehicle opening frames
    Explosion_FlameImpactVisual = 13 ///< Tornie: visual-only companion flames around the damaging impact
} ExplosionID_enum;

typedef enum {
    ItemID_Invalid = 0,
    ItemID_FirstID = 1,

    Structure_FirstID = 1,
    Structure_Barracks = 1,
    Structure_ConstructionYard = 2,
    Structure_GunTurret = 3,
    Structure_HeavyFactory = 4,
    Structure_HighTechFactory = 5,
    Structure_IX = 6,
    Structure_LightFactory = 7,
    Structure_Palace = 8,
    Structure_Radar = 9,
    Structure_Refinery = 10,
    Structure_RepairYard = 11,
    Structure_RocketTurret = 12,
    Structure_Silo = 13,
    Structure_Slab1 = 14,
    Structure_Slab4 = 15,
    Structure_StarPort = 16,
    Structure_Wall = 17,
    Structure_WindTrap = 18,
    Structure_WOR = 19,
    Structure_LastID = 19,

    // Values 20-26 were used by the former city-simulation fork.
    // Keep them reserved so all later Tornie IDs retain their historical values.
    ItemID_LegacyReserved20 = 20,
    ItemID_LegacyReserved21 = 21,
    ItemID_LegacyReserved22 = 22,
    ItemID_LegacyReserved23 = 23,
    ItemID_LegacyReserved24 = 24,
    ItemID_LegacyReserved25 = 25,
    ItemID_LegacyReserved26 = 26,

    Unit_FirstID = 27,
    Unit_Carryall = 27,
    Unit_Devastator = 28,
    Unit_Deviator = 29,
    Unit_Frigate = 30,
    Unit_Harvester = 31,
    Unit_Soldier = 32,
    Unit_Launcher = 33,
    Unit_MCV = 34,
    Unit_Ornithopter = 35,
    Unit_Quad = 36,
    Unit_Saboteur = 37,
    Unit_Sandworm = 38,
    Unit_SiegeTank = 39,
    Unit_SonicTank = 40,
    Unit_Tank = 41,
    Unit_Trike = 42,
    Unit_RaiderTrike = 43,
    Unit_Trooper = 44,
    Unit_Special = 45,
    Unit_Infantry = 46,
    Unit_Troopers = 47,
    Unit_LastID = 47,

    // Values 48-51 were used by former city-simulation objects.
    ItemID_LegacyReserved48 = 48,
    ItemID_LegacyReserved49 = 49,
    ItemID_LegacyReserved50 = 50,
    ItemID_LegacyReserved51 = 51,

    // Tornie mod units — IDs remain unchanged for save/network compatibility.
    Unit_RocketTrike = 52,            ///< Tornie: upgraded Trike (rocket weapon, Light Factory, T9 U4)
    Unit_FlameTank = 53,              ///< Tornie: fire missile weapon, Heavy Factory, T9 U4
    Unit_EliteLauncher = 54,          ///< Tornie: upgraded Launcher, Heavy Factory, T9 U4
    Unit_EliteSiegeTank = 55,         ///< Tornie: upgraded Siege Tank, Heavy Factory, T9 U4
    Unit_ExtLastID = 55,

    // Additional extended structures after units (isStructure handles non-contiguous range)
    Structure_AdvancedWindTrap = 56,  ///< Tornie: Advanced Windtrap 3x3
    Structure_Worfinery = 57,        ///< Tornie: WOR + Refinery combo, can produce Troopers
    Structure_AdvancedWindTrapMK2 = 58, ///< Tornie: Advanced Windtrap 2x3

    // Tornie mod unit — distinct from vanilla Harvester with a dedicated sprite.
    Unit_RebelHarvester = 59,

    // Tornie mod building — Palace-equivalent that spawns 1-3 random vehicles
    // when House IX is unlocked. Spawned units use Hunt behavior.
    Structure_TechCenter = 60,
    Structure_AdvancedWindTrapMK3 = 61, ///< Tornie: Advanced Windtrap 3x2
    Structure_Scoutpost = 62,           ///< Tornie: small power/defense/recon post

    // Appended after every 1.0.517 object ID to preserve save compatibility.
    Unit_SonicTrike = 63,               ///< Tornie: Rebels-only light sonic vehicle
    Unit_ChemicalSiegeTank = 64,        ///< Tornie: custom-house chemical Siege Tank
    Structure_LoveFactory = 65,         ///< Tornie: animated 2x3 Love Factory
    Unit_ChemicalCarryall = 66,         ///< Jericho: Wildspade healing transport
    Delivery_Small = 67,               ///< Love Factory small reinforcement delivery
    Delivery_Medium = 68,              ///< Love Factory medium reinforcement delivery
    Delivery_Heavy = 69,               ///< Love Factory heavy reinforcement delivery
    Delivery_Support = 70,             ///< Love Factory support reinforcement delivery
    Structure_Flamepost = 71,          ///< Tornie: flame variant of the Scoutpost
    Structure_ChaosFactory = 72,       ///< Tornie: two random global unit offers
    Structure_Chemipost = 73,          ///< Tornie: healing variant of the Scoutpost

    ItemID_LastID = 73,

    Num_ItemID
} ItemID_enum;

typedef enum {
    Terrain_Slab,
    Terrain_Sand,
    Terrain_Rock,
    Terrain_Dunes,
    Terrain_Mountain,
    Terrain_Spice,
    Terrain_ThickSpice,
    Terrain_SpiceBloom,
    Terrain_SpecialBloom,
    Terrain_GreenSpice,
    Terrain_ThickGreenSpice,
    Terrain_GreenSpiceBloom,
    Terrain_RedSpice,
    Terrain_ThickRedSpice,
    Terrain_RedSpiceBloom,
    Terrain_PaleLilacSpice,
    Terrain_ThickPaleLilacSpice,
    Terrain_PaleLilacSpiceBloom,
    Terrain_WhiteSpice,
    Terrain_ThickWhiteSpice,
    Terrain_WhiteSpiceBloom
} TERRAINTYPE;

typedef enum {
    Terrain_HiddenIsland    = 0x0,
    Terrain_HiddenUp        = 0x1,
    Terrain_HiddenRight     = 0x2,
    Terrain_HiddenUpRight   = 0x3,
    Terrain_HiddenDown      = 0x4,
    Terrain_HiddenUpDown    = 0x5,
    Terrain_HiddenDownRight = 0x6,
    Terrain_HiddenNotLeft   = 0x7,
    Terrain_HiddenLeft      = 0x8,
    Terrain_HiddenUpLeft    = 0x9,
    Terrain_HiddenLeftRight = 0xA,
    Terrain_HiddenNotDown   = 0xB,
    Terrain_HiddenDownLeft  = 0xC,
    Terrain_HiddenNotRight  = 0xD,
    Terrain_HiddenNotUp     = 0xE,
    Terrain_HiddenFull      = 0xF
} HIDDENTYPE;

/**
    This function determines if the specified itemID is an unit or not.
    \param itemID   the ID of the item (e.g. Unit_Harvester)
    \return true if it is an unit, false otherwise
*/
inline bool isUnit(int itemID) { return (itemID >= Unit_FirstID && itemID <= Unit_LastID) || (itemID >= Unit_RocketTrike && itemID <= Unit_ExtLastID) || itemID == Unit_RebelHarvester || itemID == Unit_SonicTrike || itemID == Unit_ChemicalSiegeTank || itemID == Unit_ChemicalCarryall; }

/**
    This function determines if the specified itemID is a structure or not.
    \param itemID   the ID of the item (e.g. Structure_ConstructionYard)
    \return true if it is a structure, false otherwise
*/
inline bool isStructure(int itemID) { return (itemID >= Structure_FirstID && itemID <= Structure_LastID) || (itemID >= Structure_AdvancedWindTrap && itemID <= Structure_AdvancedWindTrapMK2) || itemID == Structure_TechCenter || itemID == Structure_AdvancedWindTrapMK3 || itemID == Structure_Scoutpost || itemID == Structure_LoveFactory || itemID == Structure_Flamepost || itemID == Structure_ChaosFactory || itemID == Structure_Chemipost; }



/**
    This function determines if the specified itemID is a flying unit or not.
    \param itemID   the ID of the item (e.g. Unit_Carryall)
    \return true if it is a flying unit, false otherwise
*/
inline bool isFlyingUnit(int itemID) { return (itemID == Unit_Carryall) || (itemID == Unit_ChemicalCarryall) || (itemID == Unit_Ornithopter) || (itemID == Unit_Frigate); }
inline bool isCarryallUnit(int itemID) {
    return (itemID == Unit_Carryall) || (itemID == Unit_ChemicalCarryall);
}


/**
    This function determines if the specified itemID is an infantry unit or not.
    \param itemID   the ID of the item (e.g. Unit_Carryall)
    \return true if it is an infantry unit, false otherwise
*/
inline bool isInfantryUnit(int itemID) { return (itemID == Unit_Soldier) || (itemID == Unit_Trooper) || (itemID == Unit_Infantry) || (itemID == Unit_Troopers) || (itemID == Unit_Saboteur); }


#endif // DATA_H
