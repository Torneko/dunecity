#pragma once

#include <Campaign/CoopCampaignSession.h>
#include <Campaign/CoopCampaignRuntime.h>
#include <Game.h>
#include <House.h>
#include <AStarSearch.h>
#include <Achievements/AchievementEvents.h>
#include <Achievements/AchievementManager.h>
#include <structures/BuilderBase.h>
#include <structures/Palace.h>
#include <units/MCV.h>
#include <Trigger/ReinforcementTrigger.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/INIFile.h>
#include <SpecialVehicle.h>
#include <misc/ModContentPolicy.h>
#include <misc/FileSystem.h>
#include <misc/string_util.h>
#include <algorithm>
#include <filesystem>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

namespace coopSmoke {
inline void require(bool value, const std::string& message) {
    if(!value) throw std::runtime_error("Coop original campaign: " + message);
}

struct Geometry {
    int width = 64, offset = 0;
    explicit Geometry(INIFile& map) {
        if(map.getIntValue("BASIC", "Version", 1) >= 2) width = map.getIntValue("MAP", "SizeX", 64);
        else {
            const int scale = map.getIntValue("BASIC", "MapScale", 0);
            require(scale >= 0 && scale <= 2, "invalid classic map scale");
            offset = scale == 0 ? 1 : (scale == 1 ? 16 : 11);
        }
    }
    Coord location(int position) const { return Coord(position % width - offset, position / width - offset); }
};

inline int playerSlot(const std::string& owner, int participantCount) {
    int slot = 0;
    require(owner.rfind("Player", 0) == 0 && parseString(owner.substr(6), slot)
            && slot >= 1 && slot <= participantCount, "invalid participant " + owner);
    return slot - 1;
}

inline std::vector<int> specialPool(Game& game, int faction, bool custom, const std::string& mod) {
    const auto candidates = discoverHouseSpecialVehicleCandidates([&](int item) {
        const auto& data = game.objectData.data[item][faction];
        return HouseSpecialVehicleCandidateData{data.enabled, data.builder,
            data.prerequisiteStructuresSet[Structure_IX]};
    });
    auto pool = resolveSpecialVehiclePoolForHouse(custom ? faction : HOUSE_NEUTRAL,
        custom && ModManager::instance().isTornieContentActive(), custom && mod == "Jericho",
        candidates, custom && mod == "Tornie" && faction == HOUSE_CUSTOM);
    pool.erase(std::remove_if(pool.begin(), pool.end(), [&](int item) {
        return !isSpecialVehicleSelectionCandidate(item) || !game.objectData.data[item][faction].enabled;
    }), pool.end());
    require(!pool.empty(), "faction " + std::to_string(faction) + " has no Special spawn");
    return pool;
}

struct ExpectedUnit { std::vector<int> items; int count = 1; };
inline ExpectedUnit unitRules(Game& game, const GameInitSettings& init, int faction,
    int item, bool copied, const std::string& mod) {
    const bool custom = init.getGameOptions().content.customUnitsAndBuildings;
    if(!custom) item = neutralVanillaMapItem(item);
    ExpectedUnit result;
    if(item == Unit_Infantry5) { item = Unit_Soldier; result.count = 5; }
    else if(item == Unit_Troopers5) { item = Unit_Trooper; result.count = 5; }
    else if(item == Unit_Infantry) { item = Unit_Soldier; result.count = 3; }
    else if(item == Unit_Troopers) { item = Unit_Trooper; result.count = 3; }
    else if(item == Unit_Special) {
        result.items = specialPool(game, faction, custom, mod);
        return result;
    }
    if(custom && isHouseFaction(static_cast<HOUSETYPE>(faction), HOUSE_WILDSPADE)) {
        if(item == Unit_Trike) item = Unit_RaiderTrike;
        else if(item == Unit_Quad) item = Unit_RocketTrike;
    }
    if(game.objectData.data[item][faction].enabled || (!copied && item == Unit_ChemicalCarryall && mod != "vanilla")) {
        result.items = {item};
    } else if(copied) {
        // Require a playable equivalent in the original category rather than
        // accepting a silently omitted guest unit. Selection order is private
        // loader policy; Special results must still belong to this faction.
        const auto category = [&](std::initializer_list<int> candidates) {
            for(const int candidate : candidates)
                if(game.objectData.data[candidate][faction].enabled) result.items.push_back(candidate);
        };
        if(item == Unit_Trike || item == Unit_RaiderTrike || item == Unit_Quad
                || item == Unit_RocketTrike || item == Unit_SonicTrike) {
            category({Unit_Trike, Unit_RaiderTrike, Unit_Quad, Unit_RocketTrike, Unit_SonicTrike});
        } else if(item == Unit_Harvester || item == Unit_RebelHarvester) {
            category({Unit_Harvester, Unit_RebelHarvester});
        } else if(item == Unit_Soldier || item == Unit_Trooper || item == Unit_Saboteur) {
            category({Unit_Trooper, Unit_Soldier});
        } else if(item == Unit_Carryall || item == Unit_ChemicalCarryall) {
            category({Unit_Carryall, Unit_ChemicalCarryall});
        } else if(item == Unit_Ornithopter || item == Unit_Frigate) {
            category({Unit_Ornithopter, Unit_Carryall, Unit_ChemicalCarryall});
        } else if(item == Unit_Tank || item == Unit_SiegeTank || item == Unit_Launcher) {
            category({Unit_Tank, Unit_SiegeTank, Unit_Launcher});
        } else if(item == Unit_Devastator || item == Unit_Deviator || item == Unit_SonicTank
                || item == Unit_FlameTank || item == Unit_EliteLauncher || item == Unit_EliteSiegeTank
                || item == Unit_ChemicalSiegeTank) {
            result.items = specialPool(game, faction, custom, mod);
        }
        require(!result.items.empty(), "copied unit has no playable faction equivalent");
    } else result.count = 0; // The original solo loader deliberately ignores unavailable objects.
    return result;
}

inline void verifySource(INIFile& map) {
    const auto sourceFile = map.getStringValue("COOP_TEMPLATE", "SourceFile");
    const auto sourceHash = map.getStringValue("COOP_TEMPLATE", "SourceSha256");
    require(!sourceFile.empty() && sourceHash.size() == 64, "missing original map provenance");
    const auto separator = sourceFile.find_last_of("/\\:");
    const auto filename = separator == std::string::npos ? sourceFile : sourceFile.substr(separator + 1);
    auto source = pFileManager->openCampaignFile(filename);
    INIFile original(source.get());
    const auto sourceHouse = map.getStringValue("COOP_TEMPLATE", "SourceHouse");
    require(!sourceHouse.empty() && original.hasSection(sourceHouse), "original human section missing");
    std::map<std::string, std::string> ownerRoles, ownerCorrections;
    ownerRoles[strToUpper(sourceHouse)] = "Player1";
    const int participants = map.getIntValue("COOP_TEMPLATE", "ParticipantCount", 5);
    for(int role = 1; role <= participants - 2; ++role) {
        const auto opponent = map.getStringValue("COOP_TEMPLATE", "SourceOpponent" + std::to_string(role));
        require(!opponent.empty(), "original opponent role identity missing");
        ownerRoles[strToUpper(opponent)] = "Player" + std::to_string(role + 2);
    }
    const int correctionHouses = map.getIntValue("COOP_TEMPLATE", "OwnerCorrectionHouseCount", 0);
    for(int index = 1; index <= correctionHouses; ++index) {
        std::string owner, participant;
        require(splitString(map.getStringValue("COOP_TEMPLATE", "OwnerCorrection" + std::to_string(index)), owner, participant),
                "malformed explicit original owner correction");
        playerSlot(participant, participants);
        ownerCorrections[strToUpper(owner)] = participant;
    }
    for(const auto& key : original.getSection("MAP"))
        require(map.getStringValue("MAP", key.getKeyName()) == key.getStringValue(),
                "original terrain/resource key changed: " + key.getKeyName());
    if(original.hasSection("CHOAM"))
        for(const auto& key : original.getSection("CHOAM"))
            require(map.getStringValue("CHOAM", key.getKeyName()) == key.getStringValue(),
                    "original starport inventory changed: " + key.getKeyName());
    for(const char* flag : {"WinFlags", "LoseFlags"}) {
        const int value = original.getIntValue("BASIC", flag, std::string(flag) == "WinFlags" ? 3 : 1);
        require(map.getIntValue("BASIC", flag, -1) == value
                && map.getIntValue("COOP_TEMPLATE", std::string("Original") + flag, -1) == value,
                "original victory/defeat flags changed");
    }
    int correctedRecords = 0;
    for(const char* section : {"UNITS", "STRUCTURES", "TEAMS", "REINFORCEMENTS"}) {
        if(!original.hasSection(section)) continue;
        for(const auto& key : original.getSection(section)) {
            const auto before = key.getStringValue(), after = map.getStringValue(section, key.getKeyName());
            const auto first = before.find(','), second = after.find(',');
            require(first != std::string::npos && second != std::string::npos
                    && before.substr(first) == after.substr(second),
                    std::string("original ") + section + " record changed: " + key.getKeyName());
            const auto oldOwner = strToUpper(trim(before.substr(0, first))), newOwner = trim(after.substr(0, second));
            if(newOwner == "CoopWildlife") {
                const auto values = splitStringToStringVector(after);
                require(std::string(section) == "UNITS" && values.size() == 6
                        && getItemIDByName(trim(values[1])) == Unit_Sandworm,
                        "the wildlife exception reassigned a non-worm original record");
            } else if(ownerCorrections.count(oldOwner)) {
                require(newOwner == ownerCorrections.at(oldOwner), "explicit original owner correction was not applied");
                ++correctedRecords;
            } else require(ownerRoles.count(oldOwner) && newOwner == ownerRoles.at(oldOwner),
                    "original host/opponent record was assigned another participant");
        }
    }
    require(correctedRecords == map.getIntValue("COOP_TEMPLATE", "OwnerCorrectionCount", 0),
            "original owner corrections are not explicitly accounted for");
    for(const char* field : {"Credits", "Quota", "MaxUnit"}) {
        const int originalValue = original.getIntValue(sourceHouse, field, -1);
        if(originalValue < 0) continue;
        require(map.getIntValue("Player1", field, -1) == originalValue
                && map.getIntValue("Player2", field, -1) == originalValue,
                std::string("human ") + field + " differs from original campaign");
    }
}

inline void verifyReinforcements(Game& game, const GameInitSettings& init, const coop::Context& context,
    INIFile& map, const std::string& mod) {
    using Delivery = std::tuple<int, Uint32, int, bool>;
    std::map<Delivery, std::multiset<Uint32>> expected, actual;
    std::vector<std::pair<Delivery, ExpectedUnit>> guestDeliveries;
    if(map.hasSection("REINFORCEMENTS")) for(const auto& key : map.getSection("REINFORCEMENTS")) {
        std::string owner, name, location, time, plus;
        require(splitString(key.getStringValue(), owner, name, location, time)
                || splitString(key.getStringValue(), owner, name, location, time, plus),
                "invalid original reinforcement " + key.getKeyName());
        const int slot = playerSlot(owner, static_cast<int>(context.slots.size()));
        const auto& binding = context.slots[slot];
        const int item = getItemIDByName(name);
        require(item != ItemID_Invalid && isUnit(item), "unknown original reinforcement unit");
        auto units = unitRules(game, init, binding.faction, item, false, mod);
        // Reinforcements use ordinary availability, without the editor's
        // Chemical Carryall exception or a scenario Special-spawn resolver.
        if(item == Unit_Special || units.items.empty()
                || !game.objectData.data[units.items.front()][binding.faction].enabled) units.count = 0;
        if(units.count) require(units.items.size() == 1, "unexpected variable reinforcement type");
        Uint32 minutes = 0;
        require(parseString(time, minutes), "invalid original reinforcement time");
        auto drop = getDropLocationByName(location);
        if(drop == Drop_Invalid) drop = Drop_Homebase;
        const bool repeat = (!time.empty() && time.back() == '+') || plus == "+";
        const Delivery delivery{binding.house, MILLI2CYCLES(minutes * 60 * 1000), static_cast<int>(drop), repeat};
        for(int unit = 0; unit < units.count; ++unit) expected[delivery].insert(units.items.front());
        if(slot == 0 && item != Unit_Special) {
            const auto guest = unitRules(game, init, context.slots[1].faction, item, true, mod);
            guestDeliveries.push_back({Delivery{context.slots[1].house, MILLI2CYCLES(minutes * 60 * 1000),
                static_cast<int>(drop), repeat}, guest});
        }
    }
    for(const auto& trigger : game.getTriggerManager().getTriggers())
        if(const auto* reinforcement = dynamic_cast<const ReinforcementTrigger*>(trigger.get())) {
            const Delivery delivery{reinforcement->getHouseID(), reinforcement->getCycleNumber(),
                static_cast<int>(reinforcement->getDropLocation()), reinforcement->isRepeat()};
            for(const auto unit : reinforcement->getDroppedUnits()) actual[delivery].insert(unit);
        }
    for(const auto& delivery : guestDeliveries) {
        auto found = actual.find(delivery.first);
        require(found != actual.end(), "guest reinforcement owner/time/location/repeat missing");
        for(int i = 0; i < delivery.second.count; ++i) {
            const auto unit = std::find_if(found->second.begin(), found->second.end(), [&](Uint32 item) {
                return std::find(delivery.second.items.begin(), delivery.second.items.end(), item) != delivery.second.items.end();
            });
            require(unit != found->second.end(), "guest reinforcement count/faction equivalent missing");
            found->second.erase(unit);
        }
        if(found->second.empty()) actual.erase(found);
    }
    require(expected == actual, "original/deferred reinforcement owner, unit count, location, time or repeat flag was lost");
}

inline coop::Context verifyMission(Game& game, const GameInitSettings& init,
    const std::string& mod, int sourceFaction, int stage, bool advanceSimulation = true) {
    const auto decoded = coop::readContext(init.getFiledata());
    require(decoded.has_value(), "prepared map lost its co-op context");
    const auto context = *decoded;
    auto input = sdl2::RWops_ptr{SDL_RWFromConstMem(init.getFiledata().data(), static_cast<int>(init.getFiledata().size()))};
    INIFile map(input.get());
    Geometry geometry(map);
    static constexpr std::array<int, 9> missions{1, 2, 5, 8, 11, 14, 17, 20, 22};
    require(map.getIntValue("COOP_TEMPLATE", "SourceFaction", -1) == sourceFaction
            && map.getIntValue("COOP_TEMPLATE", "OriginalMission", -1) == missions[stage - 1],
            "wrong original faction/mission selected");
    const int expectedTech = mod == "vanilla" && stage == 9 ? 8 : stage;
    require(game.techLevel == expectedTech, "original mission technology/progression changed");
    verifySource(map);
    std::set<int> controls;
    int participatingEnemies = 0;
    require(context.slots.size() >= 5 && context.slots.size() <= 7
            && map.getIntValue("COOP_TEMPLATE", "ParticipantCount", 5) == static_cast<int>(context.slots.size()),
            "source participant count was not preserved");
    for(int i = 0; i < static_cast<int>(context.slots.size()); ++i) {
        const auto& slot = context.slots[i];
        auto* house = game.getHouse(slot.house);
        require(house && house->getHouseID() == slot.house && house->getFactionID() == slot.faction
                && house->getTeamID() == slot.team() && !house->getPlayerList().empty()
                && controls.insert(slot.house).second, "participant control/faction/team binding failed");
        require(house->getCredits() == map.getIntValue("Player" + std::to_string(i + 1), "Credits", 0),
                "participant starting funds differ from the source");
        if(i >= 2) {
            require(house->isAI(), "enemy role lost its campaign AI");
            const bool participating = map.getIntValue("COOP_TEMPLATE", "EnemyFaction" + std::to_string(i - 1), -1) >= 0;
            const bool present = map.getBoolValue("COOP_TEMPLATE", "EnemyPresent" + std::to_string(i - 1), participating);
            const bool deferred = map.getBoolValue("COOP_TEMPLATE", "EnemyDeferred" + std::to_string(i - 1), false);
            require(context.enemyPresent[i - 2] == present, "initial enemy presence marker was lost during binding");
            if(participating) {
                ++participatingEnemies;
                const int originalEnemy = map.getIntValue("COOP_TEMPLATE", "EnemyFaction" + std::to_string(i - 1), -1);
                require((slot.faction == originalEnemy || originalEnemy == context.slots[0].faction || originalEnemy == context.slots[1].faction)
                    && slot.faction != context.slots[0].faction && slot.faction != context.slots[1].faction,
                        "original/deferred opponent was assigned another faction");
                if(!present && deferred)
                    require(house->getNumUnits() == 0 && house->getNumStructures() == 0,
                            "reinforcement-only opponent gained an invented starting army/base");
            } else require(!present && !deferred && house->getNumUnits() == 0 && house->getNumStructures() == 0 && house->getCredits() == 0,
                    "an absent original opponent gained a base/army/funds");
        }
    }
    require(participatingEnemies >= 1 && participatingEnemies <= static_cast<int>(context.slots.size()) - 2
            && participatingEnemies <= 5 && pLocalHouse && pLocalHouse->getTeamID() == 1,
            "original enemy presence or local alliance is incorrect");
    if(stage == 9) {
        const int expectedEnemies = mod == "vanilla" && sourceFaction == HOUSE_FREMEN ? 4
            : (mod == "vanilla" && (sourceFaction == HOUSE_MERCENARY || sourceFaction == HOUSE_SARDAUKAR) ? 5 : 3);
        require(participatingEnemies == expectedEnemies, "an original final-mission opponent was dropped");
    }
    auto* first = game.getHouse(context.slots[0].house);
    auto* second = game.getHouse(context.slots[1].house);
    require(first->getNumItems(Structure_ConstructionYard) > 0 && second->getNumStructures() == (stage == 1 ? 1 : 0) && second->getNumItems(Structure_WOR) == (stage == 1 ? 1 : 0)
            && second->getNumItems(Unit_MCV) == first->getNumItems(Unit_MCV) + 1,
            "guest must start with an extra MCV and no prebuilt base");
    std::multiset<std::string> firstForce, secondForce;
    MCV* extraMCV = nullptr;
    int wildlifeHouse = -1;
    for(const int available : context.roster)
        if(!controls.count(available)) { wildlifeHouse = available; break; }
    for(const auto& key : map.getSection("UNITS")) {
        std::string owner, name, health, pos, angle, mode;
        require(splitString(key.getStringValue(), owner, name, health, pos, angle, mode), "invalid unit record");
        const bool wildlife = owner == "CoopWildlife";
        const int slot = wildlife ? -1 : playerSlot(owner, static_cast<int>(context.slots.size()));
        const int control = wildlife ? wildlifeHouse : context.slots[slot].house;
        auto* ownerHouse = game.getHouse(control);
        require(ownerHouse != nullptr, "declared unit owner did not load");
        if(wildlife) require(getItemIDByName(name) == Unit_Sandworm && map.getStringValue("COOP_TEMPLATE", "WildlifeOwner") == owner
                && !controls.count(control) && ownerHouse->getTeamID() == 0 && ownerHouse->getPlayerList().empty()
                && ownerHouse->getFactionID() == control, "neutral wildlife was assigned to a campaign participant");
        const bool copied = key.getKeyName().rfind("IDCOOP", 0) == 0;
        const bool extra = key.getKeyName() == "IDCOOPMCV";
        const auto signature = name + "," + health + "," + angle + "," + mode;
        if(slot == 0) firstForce.insert(signature);
        else if(slot == 1 && !extra) secondForce.insert(signature);
        int position = 0, fraction = 256, originalAngle = 64;
        require(parseString(pos, position) && parseString(health, fraction) && parseString(angle, originalAngle),
                "invalid unit position/health/angle");
        const int expectedAngle = ((NUM_ANGLES - (originalAngle + 16) / 32) + 2) % NUM_ANGLES;
        const Coord location = geometry.location(position);
        const int item = getItemIDByName(name);
        require(item != ItemID_Invalid && isUnit(item), "invalid starting unit " + name);
        auto expected = unitRules(game, init, ownerHouse->getFactionID(), item, copied, mod);
        if(!currentGameMap->tileExists(location)) {
            // The shipped solo sources contain 37 enemy records outside the
            // classic MapScale crop. Keep those source records, matching the
            // original loader's omission, without excusing a missing player
            // force, guest copy, MCV or neutral wildlife order.
            require(slot >= 2 && !copied && !wildlife, "a human/wildlife starting order lies outside the playable map");
            expected.count = 0;
        }
        int matches = 0;
        for(auto* unit : unitList) {
            if(unit->getOwner()->getHouseID() != control || unit->getLocation() != location) continue;
            if(std::find(expected.items.begin(), expected.items.end(), unit->getItemID()) == expected.items.end()) continue;
            require(unit->getHealth() == unit->getMaxHealth() * (FixPoint(fraction) / 256)
                    && unit->getAttackMode() == getAttackModeByName(mode) && unit->getAngle() == expectedAngle,
                    "starting unit health/order/angle changed: " + key.getKeyName());
            ++matches;
            if(extra) extraMCV = dynamic_cast<MCV*>(unit);
        }
        require(matches == expected.count, "starting unit missing/overlapping/wrong arsenal: " + key.getKeyName());
    }
    require(firstForce == secondForce, "guest declared forces differ from the original host force");
    require(extraMCV && extraMCV->getOwner() == second && extraMCV->canDeploy(),
            "guest MCV is missing or cannot deploy on its starting footprint");
    StructureBase* yard = nullptr;
    for(auto* structure : structureList)
        if(structure->getOwner() == first && structure->getItemID() == Structure_ConstructionYard) { yard = structure; break; }
    require(yard != nullptr, "host construction yard missing");
    const auto origin = yard->getLocation(), start = extraMCV->getLocation();
    require(std::max(std::abs(start.x - origin.x), std::abs(start.y - origin.y)) <= 10,
            "guest MCV is too far from the host construction yard");
    bool reachable = false;
    for(int y = origin.y - 1; y <= origin.y + yard->getStructureSizeY() && !reachable; ++y)
        for(int x = origin.x - 1; x <= origin.x + yard->getStructureSizeX() && !reachable; ++x) {
            if(start == Coord(x, y)) { reachable = true; break; }
            if(!extraMCV->canPass(x, y)) continue;
            AStarSearch path(currentGameMap, extraMCV, start, Coord(x, y));
            const auto route = path.getFoundPath();
            reachable = !route.empty() && route.back() == Coord(x, y);
        }
    require(reachable, "guest MCV is isolated from the host base by terrain or occupied tiles");
    for(const auto& key : map.getSection("STRUCTURES")) {
        if(key.getKeyName().rfind("ID", 0) != 0) continue;
        std::string owner, name, health, pos;
        require(splitString(key.getStringValue(), owner, name, health, pos), "invalid structure record");
        const int slot = playerSlot(owner, static_cast<int>(context.slots.size())), faction = context.slots[slot].faction;
        int item = getItemIDByName(name), position = 0;
        require(item != ItemID_Invalid && isStructure(item) && parseString(pos, position), "invalid structure " + name);
        if(!init.getGameOptions().content.customUnitsAndBuildings) item = neutralVanillaMapItem(item);
        if(!game.objectData.data[item][faction].enabled) continue;
        int matches = 0;
        for(auto* structure : structureList)
            if(structure->getOwner()->getHouseID() == context.slots[slot].house
                    && structure->getItemID() == item && structure->getLocation() == geometry.location(position)) ++matches;
        require(matches == 1, "original structure did not load at its position: " + key.getKeyName());
    }
    for(auto* unit : unitList) {
        const auto* owner = unit->getOwner();
        require(unit->getOriginalHouseID() == owner->getHouseID()
                && unit->getProductionHouseID() == owner->getFactionID()
                && unit->getMaxHealth() == game.objectData.data[unit->getItemID()][owner->getFactionID()].hitpoints,
                "starting unit mixes ownership and faction rules");
    }
    for(auto* structure : structureList) {
        const int faction = structure->getOwner()->getFactionID();
        const int technology = init.getGameOptions().content.customUnitsAndBuildings ? faction : HOUSE_NEUTRAL;
        require(structure->getProductionHouseID() == faction && structure->getTechnologyHouseID() == technology,
                "starting structure has wrong technology provenance");
        if(auto* builder = dynamic_cast<BuilderBase*>(structure)) {
            builder->updateBuildList();
            for(const auto& item : builder->getBuildList()) {
                const int price = structure->getItemID() == Structure_StarPort
                    ? structure->getOwner()->getChoam().getPrice(item.itemID)
                    : game.objectData.data[item.itemID][technology].price;
                const auto expectedPrice = static_cast<Uint32>(init.campaignPurchasePrice(price, structure->getOwner()->getHouseID()));
                require(item.price == expectedPrice,
                        "builder price differs from chosen faction/CHOAM rules: " + mod
                        + " faction=" + std::to_string(sourceFaction) + " stage=" + std::to_string(stage)
                        + " builder=" + std::to_string(structure->getItemID()) + " item=" + std::to_string(item.itemID)
                        + " expected=" + std::to_string(expectedPrice) + " actual=" + std::to_string(item.price));
            }
        }
    }
    verifyReinforcements(game, init, context, map, mod);
    require(!game.hasFinished(), "campaign finishes immediately after loading");
    if(advanceSimulation) {
        for(int tick = 0; tick < 3; ++tick) game.processObjects();
        require(!game.hasFinished(), "empty reserved opponents caused an immediate victory/defeat");
    }
    return context;
}

inline void verifyHarvest(const std::string& output, const std::string& mod, const std::vector<int>& roster) {
    const auto session = coop::CoopCampaignSession::create("runtime-harvest", mod, roster,
        {roster[0], roster[1]}, {settings.general.playerName, "Second Coop Human"}, 0x51554f54u, false);
    auto options = effectiveGameOptions;
    options.chaosMode = false; options.easyMode = false;
    const auto init = coop::makeGameSettings(session, options, "coop-harvest-smoke");
    const auto decoded = coop::readContext(init.getFiledata());
    require(decoded.has_value(), "harvest fixture lost its context");
    const auto context = *decoded;
    const auto& objective = init.getCoopHarvestObjective();
    require(objective.quota > 0 && objective.houses[0] == context.slots[0].house
            && objective.houses[1] == context.slots[1].house, "shared harvest quota/participants were not derived");
    struct RestoreName { std::string value = settings.general.playerName; ~RestoreName() { settings.general.playerName = value; } } restore;
    for(int view = 0; view < 2; ++view) {
        settings.general.playerName = context.slots[view].playerName;
        const auto path = (std::filesystem::path(output) / (mod + "-coop-harvest-" + std::to_string(view) + ".sav")).string();
        {
            auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
            require(pLocalHouse && pLocalHouse->getHouseID() == context.slots[view].house, "harvest fixture selected wrong local player");
            auto* local = game->getHouse(context.slots[view].house);
            auto* remote = game->getHouse(context.slots[1 - view].house);
            local->addCredits(objective.quota / 2, true);
            remote->addCredits(objective.quota - objective.quota / 2 - 1, true);
            require(!game->hasFinished(), "shared harvest completed below the team threshold");
            require(game->saveGame(path), "shared harvest checkpoint failed");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        {
            auto game = std::make_unique<Game>(); currentGame = game.get();
            GameInitSettings load(getBasename(path, true), readCompleteFile(path), "coop-harvest-smoke");
            for(const auto& info : init.getHouseInfoList()) load.addHouseInfo(info);
            game->initGame(load);
            const auto& recovered = game->getGameInitSettings().getCoopHarvestObjective();
            require(recovered.quota == objective.quota && recovered.houses == objective.houses
                    && pLocalHouse && pLocalHouse->getHouseID() == context.slots[view].house && !game->hasFinished(),
                    "shared harvest quota/view/progress changed after save/load");
            auto* remote = game->getHouse(context.slots[1 - view].house);
            remote->addCredits(1, true);
            require(game->hasWon(), "nonlocal player's threshold contribution did not win for the allied team");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
    }
    SDL_Log("COOP HARVEST PASS: %s both local views, team threshold, nonlocal contribution and checkpoint objective/progress", mod.c_str());
}

inline void verifyOwnerRejection(const GameInitSettings& valid) {
    auto input = sdl2::RWops_ptr{SDL_RWFromConstMem(valid.getFiledata().data(), static_cast<int>(valid.getFiledata().size()))};
    INIFile map(input.get());
    const auto rawFaction = map.getStringValue("COOP_TEMPLATE", "SourceHouse");
    require(!rawFaction.empty(), "owner rejection fixture lost its source faction");
    for(const auto& invalidOwner : {rawFaction, std::string("Player99")}) {
        auto data = valid.getFiledata();
        const auto normalized = strToUpper(data);
        const auto position = normalized.find("=PLAYER1,", normalized.find("[UNITS]"));
        require(position != std::string::npos, "owner rejection fixture has no original host unit");
        data.replace(position + 1, std::string("Player1").size(), invalidOwner);
        auto invalid = valid; invalid.setMapData(data);
        bool rejected = false;
        {
            auto game = std::make_unique<Game>(); currentGame = game.get();
            try { game->initGame(invalid); }
            catch(const std::runtime_error& error) {
                rejected = std::string(error.what()).find("Unmapped original co-op owner") != std::string::npos;
            }
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        require(rejected, "raw faction/unknown participant owner was allowed to bind to another player's army");
    }
    SDL_Log("COOP OWNER REJECTION PASS: raw faction and Player99 cannot become an original campaign unit owner");
}
} // namespace coopSmoke

inline void verifyCoopMissionStages(const std::string& mod) {
    std::vector<int> roster;
    for(int h = 0; h < NUM_HOUSES; ++h)
        if(isCampaignHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
    auto options = effectiveGameOptions; options.chaosMode = false; options.easyMode = false;
    int maps = 0;
    for(std::size_t i = 0; i < roster.size(); ++i) {
        const int source = roster[i], guest = roster[(i + 1) % roster.size()];
        auto session = coop::CoopCampaignSession::create("runtime-original-" + std::to_string(source), mod, roster,
            {source, guest}, {settings.general.playerName, "Second Coop Human"}, 0x53544147u, false);
        for(int stage = 1; stage <= coop::StageCount; ++stage) {
            const auto init = coop::makeGameSettings(session, options, "coop-stage-smoke");
            {
                auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
                coopSmoke::verifyMission(*game, init, mod, source, stage);
            }
            currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
            ++maps; session.completeMission(true);
            SDL_Log("COOP MISSION PASS: %s faction=%d stage=%d original terrain/objectives/enemies, copied forces, reachable deployable MCV and faction technology", mod.c_str(), source, stage);
        }
        coopSmoke::require(session.isComplete(), "nine victories did not complete the common campaign");
    }
    SDL_Log("COOP ORIGINAL MATRIX PASS: %s %d real campaign variants", mod.c_str(), maps);
}

inline void verifyCoopFactionMatrix(const std::string& output, const std::string& mod) {
    static std::set<std::string> verified;
    if(verified.count(mod)) return;
    std::vector<int> roster;
    for(int h = 0; h < NUM_HOUSES; ++h)
        if(isCampaignHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
    const std::size_t expected = mod == "vanilla" ? 9 : (mod == "TornieLite" || mod == "JerichoLite" ? 6 : 12);
    coopSmoke::require(roster.size() == expected, "unexpected campaign roster");
    auto options = effectiveGameOptions; options.chaosMode = false; options.easyMode = false;
    {
        const auto session = coop::CoopCampaignSession::create("runtime-invalid-owner", mod, roster,
            {roster[0], roster[1]}, {settings.general.playerName, "Second Coop Human"}, 0x4f574e52u, false);
        coopSmoke::verifyOwnerRejection(coop::makeGameSettings(session, options, "coop-owner-smoke"));
    }
    struct SavedUnit { Uint32 id; int house, faction, item; };
    struct SavedStructure { Uint32 id; int house, faction; };
    for(const int chosen : roster) {
        auto session = coop::CoopCampaignSession::create("runtime-faction-" + std::to_string(chosen), mod, roster,
            {chosen, chosen}, {settings.general.playerName, "Second Coop Human"}, 0x434f4f50u + static_cast<Uint32>(chosen), false);
        session.setPlayerColor(0, HOUSE_HARKONNEN); session.setPlayerColor(1, HOUSE_ATREIDES);
        const auto init = coop::makeGameSettings(session, options, "coop-faction-matrix");
        const auto decoded = coop::readContext(init.getFiledata());
        coopSmoke::require(decoded.has_value(), "prepared same-faction map lost context");
        const auto context = *decoded;
        const auto path = (std::filesystem::path(output) / (mod + "-same-faction-" + std::to_string(chosen) + ".sav")).string();
        std::vector<SavedUnit> savedUnits;
        std::vector<SavedStructure> savedStructures;
        auto checkBindings = [&](Game& game) {
            coopSmoke::require(context.slots[0].house != context.slots[1].house, "same faction shares ownership");
            for(const auto& slot : context.slots) {
                auto* house = game.getHouse(slot.house);
                coopSmoke::require(house && house->getFactionID() == slot.faction && house->getHouseID() == slot.house
                        && house->getTeamID() == slot.team() && !house->getPlayerList().empty(), "saved house/team/faction binding failed");
                const auto& setup = init.getHouseInfoList();
                const auto requested = std::find_if(setup.begin(), setup.end(), [&](const auto& info) { return info.houseID == slot.house; });
                coopSmoke::require(requested != setup.end() && getHouseVisualHouse(slot.house) == requested->colorOfHouse,
                        "player-selected color was replaced by faction color");
            }
            auto* first = pGFXManager->getZoomedObjPic(ObjPic_Tank_Base, context.slots[0].house, 0);
            auto* second = pGFXManager->getZoomedObjPic(ObjPic_Tank_Base, context.slots[1].house, 0);
            coopSmoke::require(first && second, "same-faction Tank atlas missing");
            SDL_RenderSetClipRect(renderer, nullptr); SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderClear(renderer);
            SDL_Rect source{0, 0, D2_TILESIZE, D2_TILESIZE};
            SDL_Rect left = source, right{D2_TILESIZE, 0, D2_TILESIZE, D2_TILESIZE};
            coopSmoke::require(SDL_RenderCopy(renderer, first, &source, &left) == 0 && SDL_RenderCopy(renderer, second, &source, &right) == 0,
                    "same-faction palette rendering failed");
            auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, 2 * D2_TILESIZE, D2_TILESIZE, 32, SDL_PIXELFORMAT_ARGB8888));
            SDL_Rect area{0, 0, 2 * D2_TILESIZE, D2_TILESIZE};
            coopSmoke::require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format, pixels->pixels, pixels->pitch) == 0,
                    "palette pixel capture failed");
            bool different = false;
            for(int y = 0; y < D2_TILESIZE; ++y) {
                auto* row = reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(pixels->pixels) + y * pixels->pitch);
                for(int x = 0; x < D2_TILESIZE; ++x) different = different || (row[x] & 0xffffff) != (row[x + D2_TILESIZE] & 0xffffff);
            }
            coopSmoke::require(different, "same-faction armies have one shared palette");
        };
        {
            auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
            // Checkpoint the initial layout: Area Guard units may reserve the MCV
            // footprint once simulated. The full mission matrix advances separately.
            checkBindings(*game); coopSmoke::verifyMission(*game, init, mod, chosen, 1, false);
            for(int i = 0; i < 2; ++i)
                coopSmoke::require(House::factionForRuntimeHouse(context.slots[i].house) == chosen && pSFXManager->getVoice(Reporting, chosen),
                        "chosen faction voice identity missing");
            for(auto* unit : unitList) savedUnits.push_back({unit->getObjectID(), unit->getOwner()->getHouseID(), unit->getProductionHouseID(), unit->getItemID()});
            for(auto* structure : structureList) savedStructures.push_back({structure->getObjectID(), structure->getOwner()->getHouseID(), structure->getProductionHouseID()});
            coopSmoke::require(game->saveGame(path), "same-faction checkpoint failed");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        {
            auto game = std::make_unique<Game>(); currentGame = game.get();
            GameInitSettings load(getBasename(path, true), readCompleteFile(path), "coop-faction-matrix");
            for(const auto& info : init.getHouseInfoList()) load.addHouseInfo(info);
            game->initGame(load); checkBindings(*game);
            for(const auto& saved : savedUnits) {
                const auto* unit = game->getObjectManager().getObject(saved.id);
                coopSmoke::require(unit && unit->getItemID() == saved.item && unit->getOwner()->getHouseID() == saved.house
                        && unit->getOriginalHouseID() == saved.house && unit->getProductionHouseID() == saved.faction,
                        "unit ownership/faction changed after checkpoint load");
            }
            for(const auto& saved : savedStructures) {
                const auto* structure = game->getObjectManager().getObject(saved.id);
                coopSmoke::require(structure && structure->getOwner()->getHouseID() == saved.house && structure->getProductionHouseID() == saved.faction,
                        "structure ownership/faction changed after checkpoint load");
            }
            auto* guest = game->getHouse(context.slots[1].house);
            MCV* mcv = nullptr;
            auto input = sdl2::RWops_ptr{SDL_RWFromConstMem(init.getFiledata().data(), static_cast<int>(init.getFiledata().size()))};
            INIFile map(input.get());
            std::string owner, name, health, position, angle, mode;
            splitString(map.getStringValue("UNITS", "IDCOOPMCV"), owner, name, health, position, angle, mode);
            int originalPosition = 0;
            coopSmoke::require(parseString(position, originalPosition), "extra MCV checkpoint provenance is missing");
            const auto extraLocation = coopSmoke::Geometry(map).location(originalPosition);
            for(auto* unit : unitList)
                if(unit->getOwner() == guest && unit->getItemID() == Unit_MCV && unit->getLocation() == extraLocation)
                    { mcv = dynamic_cast<MCV*>(unit); break; }
            coopSmoke::require(mcv && mcv->canDeploy(), "guest MCV lost its deployable footprint after checkpoint load");
            const auto location = mcv->getLocation();
            coopSmoke::require(mcv->doDeploy(), "guest cannot actually deploy its starting MCV after checkpoint load");
            bool deployed = false;
            for(auto* structure : structureList)
                if(structure->getOwner() == guest && structure->getItemID() == Structure_ConstructionYard
                        && structure->getLocation() == location && structure->getProductionHouseID() == chosen) deployed = true;
            coopSmoke::require(deployed, "guest MCV deployed a yard with another player's ownership or faction");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        SDL_Log("COOP FACTION MATRIX PASS: %s faction=%d same-faction original opening, independent ownership/colors/arsenal/voices and checkpoint", mod.c_str(), chosen);
    }
    coopSmoke::verifyHarvest(output, mod, roster);
    verifyCoopMissionStages(mod);
    verified.insert(mod);
}

inline void verifyCoopFactionRules(const std::string& output, const std::string& mod) {
    if(mod != "Tornie" && mod != "Jericho") return;
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Coop faction check: " + message);
    };
    std::vector<int> roster;
    for(int h = 0; h < NUM_HOUSES; ++h)
        if(isCustomGameHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
    const auto chosen = getRuntimeHouseForIdentity(HOUSE_WILDSPADE);
    const auto session = coop::CoopCampaignSession::create("runtime-same-faction", mod, roster,
        {chosen, chosen}, {settings.general.playerName, "Second Coop Human"}, 0x434f4f50, false, "legacy");
    const auto& context = session.context();
    require(context.slots[0].house != context.slots[1].house
            && context.slots[0].faction == chosen && context.slots[1].faction == chosen,
            "same faction shared an ownership slot");
    std::string map = "[BASIC]\nVersion=2\nTechLevel=1\n[MAP]\nSizeX=64\nSizeY=48\n";
    for(int y = 0; y < 48; ++y) map += fmt::sprintf("%03d=", y) + std::string(64, '%') + "\n";
    for(int i = 1; i <= coop::SlotCount; ++i)
        map += "[Player" + std::to_string(i) + "]\nCredits=100000\n";
    map += "[COOP_TEMPLATE]\nStage=1\n";
    auto options = effectiveGameOptions;
    options.chaosMode = false;
    options.easyMode = false;
    options.instantBuild = true;
    GameInitSettings init(session.missionFilename(), session.prepareMap(map), "coop-smoke", false, options);
    for(const auto& slot : context.slots) {
        GameInitSettings::HouseInfo info(static_cast<HOUSETYPE>(slot.house), slot.team());
        info.addPlayerInfo(GameInitSettings::PlayerInfo(slot.playerName, slot.playerClass));
        init.addHouseInfo(info);
    }
    const auto savePath = (std::filesystem::path(output) / (mod + "-coop-factions.sav")).string();
    std::array<Uint32, 2> units{};
    Uint32 borrowedUnitID = NONE_ID, borrowedYardID = NONE_ID;
    {
        auto game = std::make_unique<Game>();
        currentGame = game.get();
        game->initGame(init);
        game->techLevel = 9;
        for(int i = 0; i < 2; ++i) {
            auto* house = game->getHouse(context.slots[i].house);
            require(house && house->getFactionID() == chosen && house->getHouseID() == context.slots[i].house,
                    "chosen faction and ownership were conflated");
            const int x = i == 0 ? 1 : 31;
            auto* yard = dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID, Structure_ConstructionYard, x, 1, true, true));
            auto* factory = dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID, Structure_HeavyFactory, x, 6, true, true));
            auto* palace = dynamic_cast<Palace*>(house->placeStructure(NONE_ID, Structure_Palace, x, 11, true, true));
            require(yard && factory && palace && factory->getTechnologyHouseID() == chosen,
                    "factory used the surrogate technology tree");
            factory->updateBuildList();
            require(factory->isAvailableToBuild(Unit_Tank), "same faction lost its Tank purchase");
            for(const auto& item : factory->getBuildList())
                require(item.price == static_cast<Uint32>(game->objectData.data[item.itemID][chosen].price),
                        "builder price came from the surrogate faction");
            require(palace->usesJerichoOrnithopterStrike(), "Palace used the surrogate ability");
            auto* unit = house->createUnit(Unit_Tank, true);
            unit->deploy(Coord(x, 19));
            units[i] = unit->getObjectID();
            require(unit->getProductionHouseID() == chosen
                    && unit->getOriginalHouseID() == context.slots[i].house
                    && unit->getMaxHealth() == game->objectData.data[Unit_Tank][chosen].hitpoints,
                    "unit rules changed its original control slot");
            require(house->getMilitaryValue() == game->objectData.data[Unit_Tank][chosen].price,
                    "faction score used the surrogate unit price");
            require(House::factionForRuntimeHouse(house->getHouseID()) == chosen
                    && pSFXManager->getVoice(Reporting, chosen) != nullptr,
                    "chosen faction voice is unavailable");
        }
        require(game->getHouse(context.slots[0].house)->getNumUnits() == 1
                && game->getHouse(context.slots[1].house)->getNumUnits() == 1,
                "same-faction armies share their counters");
        auto* firstUnit = static_cast<UnitBase*>(game->getObjectManager().getObject(units[0]));
        auto* firstHouse = game->getHouse(context.slots[0].house);
        auto* secondHouse = game->getHouse(context.slots[1].house);
        firstUnit->deviate(secondHouse);
        require(firstUnit->wasDeviated() && firstUnit->getOwner() == secondHouse
                && firstUnit->getProductionHouseID() == chosen,
                "same-faction deviation lost independent ownership");
        firstUnit->deviate(firstHouse);
        require(!firstUnit->wasDeviated() && firstUnit->getOwner() == firstHouse,
                "same-faction unit could not return to its original owner");
        AchievementEvents::begin(*game, {}, false, false);
        require(achievements::AchievementManager::instance().match().info.house == "Wildspade",
                "achievement identity names the surrogate faction");

        // This explicit donor is also a simulation slot assigned Wildspade.
        // A second runtime->faction conversion would corrupt borrowed tech.
        const int donor = context.slots[1].house;
        auto* second = game->getHouse(context.slots[1].house);
        auto* borrowed = second->createUnit(Unit_Tank, false, donor);
        borrowed->deploy(Coord(35, 20));
        borrowedUnitID = borrowed->getObjectID();
        require(borrowed->getProductionHouseID() == donor
                && borrowed->getMaxHealth() == game->objectData.data[Unit_Tank][donor].hitpoints,
                "explicit captured/Chaos donor was mapped twice");
        auto* borrowedYard = second->placeStructure(NONE_ID, Structure_ConstructionYard, 40, 24, true, true);
        require(borrowedYard != nullptr, "donor yard missing");
        borrowedYardID = borrowedYard->getObjectID();
        borrowedYard->setOriginalHouseID(donor);
        require(borrowedYard->getProductionHouseID() == donor
                && borrowedYard->getTechnologyHouseID() == donor,
                "captured structure donor was mapped twice");
        require(game->saveGame(savePath), "coop faction save failed");
    }
    currentGame = nullptr;
    pLocalHouse = nullptr;
    pLocalPlayer = nullptr;
    {
        auto game = std::make_unique<Game>();
        currentGame = game.get();
        GameInitSettings load(getBasename(savePath, true), readCompleteFile(savePath), "coop-smoke");
        for(const auto& info : init.getHouseInfoList()) load.addHouseInfo(info);
        game->initGame(load);
        for(int i = 0; i < 2; ++i) {
            auto* house = game->getHouse(context.slots[i].house);
            auto* unit = game->getObjectManager().getObject(units[i]);
            require(house && house->getFactionID() == chosen && unit
                    && unit->getOwner() == house && unit->getProductionHouseID() == chosen
                    && unit->getOriginalHouseID() == context.slots[i].house,
                    "save/load lost independent same-faction rules or ownership");
        }
        const auto* borrowedUnit = game->getObjectManager().getObject(borrowedUnitID);
        const auto* borrowedYard = game->getObjectManager().getObject(borrowedYardID);
        require(borrowedUnit && borrowedYard
                && borrowedUnit->getProductionHouseID() == context.slots[1].house
                && borrowedYard->getProductionHouseID() == context.slots[1].house,
                "save/load remapped an explicit donor as a simulation slot");
    }
    currentGame = nullptr;
    pLocalHouse = nullptr;
    pLocalPlayer = nullptr;
    SDL_Log("COOP FACTION PASS: %s, two independent Wildspade armies, selected tech/prices/Palace/voice/score, donor provenance and save/load", mod.c_str());
}
