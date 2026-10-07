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

#include <GameInitSettings.h>
#include <Campaign/CoopCampaignSession.h>
#include <FileClasses/INIFile.h>
#include <misc/SDL2pp.h>

#include <misc/IFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/InputStream.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <mmath.h>

#include <globals.h>
#include <mod/ModManager.h>
#include <sand.h>

namespace {
constexpr Uint32 GAMEINIT_MOD_MARKER = 0x4D4F4421;   // "MOD!"
constexpr Uint32 GAMEINIT_MOD3_MARKER = 0x4D4F4433;  // "MOD3"
constexpr Uint32 GAMEINIT_MOD5_MARKER = 0x4D4F4435;  // "MOD5": campaign options and provenance
constexpr Uint32 GAMEINIT_MOD4_MARKER = 0x4D4F4434;  // "MOD4": Chaos Mode
constexpr Uint32 GAMEINIT_MOD2_MARKER = 0x4D4F4432;  // "MOD2"
}

// Helper to capture current mod info
static void setModInfo(std::string& modName, std::string& modChecksum) {
    if (ModManager::instance().isInitialized()) {
        modName = ModManager::instance().getActiveModName();
        modChecksum = ModManager::instance().getEffectiveChecksums().combined;
    } else {
        modName = "vanilla";
        modChecksum = "";
    }
}

bool GameInitSettings::isVanillaKleshmershCampaign() const {
    return modName == "vanilla" && ((gameType == GameType::Campaign && houseID == HOUSE_KLESHMERSH)
        || strToUpper(filename).rfind("SCENK", 0) == 0);
}

int GameInitSettings::getFactionColorSlot(HOUSETYPE house) const {
    return getCampaignHouseColorSlot(house, isVanillaKleshmershCampaign() ? HOUSE_KLESHMERSH : HOUSE_INVALID);
}

GameInitSettings::GameInitSettings() {
    randomSeed = getRandomInt();
    setModInfo(modName, modChecksum);
    this->gameOptions.content = ModManager::instance().isInitialized() ? ModManager::instance().getActiveContentOptions() : ModContentOptions::legacy(modName);
    if(modName == "vanilla") this->gameOptions.chaosMode = false;
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::Campaign), houseID(newHouseID), mission(gameOptions.easyMode ? 2 : 1), alreadyShownTutorialHints(0), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
    setModInfo(modName, modChecksum);
    this->gameOptions.content = ModManager::instance().isInitialized() ? ModManager::instance().getActiveContentOptions() : ModContentOptions::legacy(modName);
    if(modName == "vanilla") this->gameOptions.chaosMode = false;
    chaosCampaignEligible = gameType == GameType::Campaign && isChaosModeEnabled();
}

GameInitSettings::GameInitSettings(const GameInitSettings& prevGameInitInfoClass, int nextMission, Uint32 alreadyPlayedRegions, Uint32 alreadyShownTutorialHints) {
    *this = prevGameInitInfoClass;
    if(gameType == GameType::Campaign) {
        // Older campaign saves can predate added factions. Keep their player,
        // support and chosen enemy AI, but register missing opponents for the
        // next map so the loader can give them credits and an AI controller.
        std::string enemyAIClass = DEFAULTAIPLAYERCLASS;
        int enemyTeam = 2;
        bool foundEnemyAI = false;
        for(const HouseInfo& houseInfo : houseInfoList) {
            if(houseInfo.houseID == houseID) continue;
            for(const PlayerInfo& playerInfo : houseInfo.playerInfoList) {
                if(playerInfo.playerClass == HUMANPLAYERCLASS) continue;
                enemyAIClass = playerInfo.playerClass;
                enemyTeam = houseInfo.team;
                foundEnemyAI = true;
                break;
            }
            if(foundEnemyAI) break;
        }
        for(int h = 0; h < NUM_HOUSES; ++h) {
            const auto house = static_cast<HOUSETYPE>(h);
            if(house == houseID || !isCampaignHouseAvailable(house)) continue;
            bool exists = false;
            for(const HouseInfo& houseInfo : houseInfoList) {
                if(houseInfo.houseID == house) {
                    exists = true;
                    break;
                }
            }
            if(exists) continue;
            HouseInfo opponent(house, enemyTeam);
            opponent.addPlayerInfo(PlayerInfo(getHouseDisplayNameByNumber(house), enemyAIClass));
            houseInfoList.push_back(std::move(opponent));
        }
    }
    mission = nextMission;
    this->alreadyPlayedRegions = alreadyPlayedRegions;
    this->alreadyShownTutorialHints = alreadyShownTutorialHints;
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, int newMission, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::Skirmish), houseID(newHouseID), mission(newMission), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
    setModInfo(modName, modChecksum);
    this->gameOptions.content = ModManager::instance().isInitialized() ? ModManager::instance().getActiveContentOptions() : ModContentOptions::legacy(modName);
    if(modName == "vanilla") this->gameOptions.chaosMode = false;
    chaosCampaignEligible = gameType == GameType::Campaign && mission == 1 && isChaosModeEnabled();
}

GameInitSettings::GameInitSettings(const std::string& mapfile, const std::string& filedata, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::CustomGame), filename(mapfile), filedata(filedata), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = getRandomInt();
    setModInfo(modName, modChecksum);
    this->gameOptions.content = ModManager::instance().isInitialized() ? ModManager::instance().getActiveContentOptions() : ModContentOptions::legacy(modName);
    if(modName == "vanilla") this->gameOptions.chaosMode = false;
}

GameInitSettings::GameInitSettings(const std::string& mapfile, const std::string& filedata, const std::string& serverName, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::CustomMultiplayer), filename(mapfile), filedata(filedata), servername(serverName), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = getRandomInt();
    setModInfo(modName, modChecksum);
    this->gameOptions.content = ModManager::instance().isInitialized() ? ModManager::instance().getActiveContentOptions() : ModContentOptions::legacy(modName);
    if(modName == "vanilla") this->gameOptions.chaosMode = false;
}

GameInitSettings::GameInitSettings(const std::string& savegame)
 : gameType(GameType::LoadSavegame) {
    checkSaveGame(savegame);
    filename = savegame;
}

GameInitSettings::GameInitSettings(const std::string& savegame, const std::string& filedata, const std::string& serverName)
 : gameType(GameType::LoadMultiplayer), filename(savegame), filedata(filedata), servername(serverName) {
    IMemoryStream memStream(filedata.c_str(), filedata.size());
    checkSaveGame(memStream);
}

GameInitSettings::GameInitSettings(InputStream& stream, bool hasModMetadata) {
    gameType = static_cast<GameType>(stream.readSint8());
    houseID = static_cast<HOUSETYPE>(stream.readSint8());

    filename = stream.readString();
    filedata = stream.readString();

    mission = stream.readUint8();
    alreadyPlayedRegions = stream.readUint32();
    alreadyShownTutorialHints = stream.readUint32();
    randomSeed = stream.readUint32();

    multiplePlayersPerHouse = stream.readBool();
    gameOptions.gameSpeed = stream.readUint32();
    gameOptions.concreteRequired = stream.readBool();
    gameOptions.structuresDegradeOnConcrete = stream.readBool();
    gameOptions.fogOfWar = stream.readBool();
    gameOptions.startWithExploredMap = stream.readBool();
    gameOptions.instantBuild = stream.readBool();
    gameOptions.onlyOnePalace = stream.readBool();
    gameOptions.rocketTurretsNeedPower = stream.readBool();
    gameOptions.sandwormsRespawn = stream.readBool();
    gameOptions.killedSandwormsDropSpice = stream.readBool();
    gameOptions.manualCarryallDrops = stream.readBool();
    gameOptions.maximumNumberOfUnitsOverride = stream.readSint32();
    gameOptions.maximumNumberOfHarvestersOverride = stream.readSint32();
    gameOptions.immortalHumanPlayer = stream.readBool();

    Uint32 numHouseInfo = stream.readUint32();
    for(Uint32 i=0;i<numHouseInfo;i++) {
        houseInfoList.push_back(HouseInfo(stream));
    }

    // Pre-mod saves end these settings here; the next word is the outer house setup.
    if(!hasModMetadata) return;

    // Read mod info (added in version with mod system)
    // Use marker to detect presence for backward compatibility
    try {
        Uint32 modMarker = stream.readUint32();
        if (modMarker == GAMEINIT_MOD_MARKER || modMarker == GAMEINIT_MOD2_MARKER || modMarker == GAMEINIT_MOD3_MARKER || modMarker == GAMEINIT_MOD4_MARKER || modMarker == GAMEINIT_MOD5_MARKER) {
            modName = stream.readString();
            modChecksum = stream.readString();

            if(modMarker == GAMEINIT_MOD2_MARKER || modMarker == GAMEINIT_MOD3_MARKER || modMarker == GAMEINIT_MOD4_MARKER || modMarker == GAMEINIT_MOD5_MARKER) {
                Uint32 numHouseColors = stream.readUint32();
                for(Uint32 i = 0; i < numHouseColors; i++) {
                    const int colorOfHouse = stream.readSint32();
                    if(i < houseInfoList.size()) {
                        houseInfoList[i].colorOfHouse = colorOfHouse;
                    }
                }
            }

            if(modMarker == GAMEINIT_MOD3_MARKER || modMarker == GAMEINIT_MOD4_MARKER || modMarker == GAMEINIT_MOD5_MARKER) {
                gameOptions.randomSpiceBlooms = stream.readBool();
            }
            if(modMarker == GAMEINIT_MOD4_MARKER || modMarker == GAMEINIT_MOD5_MARKER) gameOptions.chaosMode = stream.readBool();
            gameOptions.content = ModContentOptions::legacy(modName);
            if(modMarker == GAMEINIT_MOD5_MARKER) {
                gameOptions.easyMode = stream.readBool();
                chaosCampaignEligible = stream.readBool();
                gameOptions.content.customUnitsAndBuildings = stream.readBool();
                gameOptions.content.greenSpice = stream.readBool();
                gameOptions.content.redSpice = stream.readBool();
                gameOptions.content.purpleSpice = stream.readBool();
                gameOptions.content.blueSpice = stream.readBool();
            }
            if(modName == "vanilla") gameOptions.chaosMode = false;
        }
    } catch (InputStream::eof&) {
        // Old format without mod info - use defaults
        modName = "vanilla";
        modChecksum = "";
    }
}

GameInitSettings::~GameInitSettings() {
}

void GameInitSettings::save(OutputStream& stream) const {
    stream.writeSint8(static_cast<Sint8>(gameType));
    stream.writeSint8(houseID);

    stream.writeString(filename);
    stream.writeString(filedata);

    stream.writeUint8(mission);
    stream.writeUint32(alreadyPlayedRegions);
    stream.writeUint32(alreadyShownTutorialHints);
    stream.writeUint32(randomSeed);

    stream.writeBool(multiplePlayersPerHouse);
    stream.writeUint32(gameOptions.gameSpeed);
    stream.writeBool(gameOptions.concreteRequired);
    stream.writeBool(gameOptions.structuresDegradeOnConcrete);
    stream.writeBool(gameOptions.fogOfWar);
    stream.writeBool(gameOptions.startWithExploredMap);
    stream.writeBool(gameOptions.instantBuild);
    stream.writeBool(gameOptions.onlyOnePalace);
    stream.writeBool(gameOptions.rocketTurretsNeedPower);
    stream.writeBool(gameOptions.sandwormsRespawn);
    stream.writeBool(gameOptions.killedSandwormsDropSpice);
    stream.writeBool(gameOptions.manualCarryallDrops);
    stream.writeSint32(gameOptions.maximumNumberOfUnitsOverride);
    stream.writeSint32(gameOptions.maximumNumberOfHarvestersOverride);
    stream.writeBool(gameOptions.immortalHumanPlayer);

    stream.writeUint32(houseInfoList.size());
    for(const HouseInfo& houseInfo : houseInfoList) {
        houseInfo.save(stream);
    }

    // Write mod info with marker for forward compatibility
    stream.writeUint32(GAMEINIT_MOD5_MARKER);
    stream.writeString(modName);
    stream.writeString(modChecksum);

    stream.writeUint32(houseInfoList.size());
    for(const HouseInfo& houseInfo : houseInfoList) {
        stream.writeSint32(houseInfo.colorOfHouse);
    }
    stream.writeBool(gameOptions.randomSpiceBlooms);
    stream.writeBool(isChaosModeEnabled());
    stream.writeBool(gameOptions.easyMode);
    stream.writeBool(isChaosCampaignEligible());
    stream.writeBool(gameOptions.content.customUnitsAndBuildings);
    stream.writeBool(gameOptions.content.greenSpice);
    stream.writeBool(gameOptions.content.redSpice);
    stream.writeBool(gameOptions.content.purpleSpice);
    stream.writeBool(gameOptions.content.blueSpice);
}



const GameInitSettings::CoopHarvestObjective& GameInitSettings::getCoopHarvestObjective() const {
    if(!coopHarvestCached_) {
        coopHarvestObjective_ = {};
        if(const auto context = coop::readContext(filedata)) {
            auto source = sdl2::RWops_ptr{SDL_RWFromConstMem(filedata.data(), static_cast<int>(filedata.size()))};
            INIFile map(source.get());
            const int target = map.getIntValue("COOP_TEMPLATE", "SharedQuota", 0);
            if(target > 0 && (map.getIntValue("BASIC", "WinFlags", 3) & WINLOSEFLAGS_QUOTA)) {
                coopHarvestObjective_.quota = target;
                coopHarvestObjective_.houses = {{context->slots[0].house, context->slots[1].house}};
            }
        }
        coopHarvestCached_ = true;
    }
    return coopHarvestObjective_;
}

int GameInitSettings::campaignPurchasePrice(int price, int ownerHouse) const {
    return isEasyModeEnabled() && ownerHouse == houseID ? std::max(1, price - 25) : price;
}

int GameInitSettings::campaignStartingCredits(int credits, int ownerHouse) const {
    return isEasyModeEnabled() && ownerHouse == houseID && mission == 2 ? credits + 500 : credits;
}

void GameInitSettings::migrateLegacyHouseColorSlots() {
    for(HouseInfo& houseInfo : houseInfoList) {
        houseInfo.colorOfHouse = migrateLegacyHouseColorSlot(houseInfo.colorOfHouse);
    }
}

std::string GameInitSettings::getScenarioFilename(HOUSETYPE newHouse, int mission) {
    if((newHouse < 0) || (newHouse >= NUM_HOUSES) || !isCampaignHouseAvailable(newHouse)) {
        THROW(std::invalid_argument, "GameInitSettings::getScenarioFilename(): Invalid house id " + std::to_string(newHouse) + ".");
    }

    if( (mission < 0) || (mission > 22)) {
        THROW(std::invalid_argument, "GameInitSettings::getScenarioFilename(): There is no mission number " + std::to_string(mission) + ".");
    }

    std::string name = "SCEN?0??.INI";
    name[4] = getHouseScenarioLetter(newHouse);

    name[6] = '0' + (mission / 10);
    name[7] = '0' + (mission % 10);

    return name;
}

void GameInitSettings::checkSaveGame(const std::string& savegame) {
    IFileStream fs;

    if(fs.open(savegame) == false) {
        THROW(std::runtime_error, "Cannot open savegame. Make sure you have read access to this savegame!");
    }

    checkSaveGame(fs);

    fs.close();
}


void GameInitSettings::checkSaveGame(InputStream& stream) {
    Uint32 magicNum;
    Uint32 savegameVersion;
    std::string duneVersion;
    try {
        magicNum = stream.readUint32();
        savegameVersion = stream.readUint32();
        duneVersion = stream.readString();
    } catch (std::exception&) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it seems to be truncated!");
    }

    if(magicNum != SAVEMAGIC) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it has a wrong magic number!");
    }

    // Support backward compatibility: Accept version 9705 (pre-Original AI) and newer
    constexpr Uint32 MINIMUM_SUPPORTED_VERSION = 9705;

    if(savegameVersion < MINIMUM_SUPPORTED_VERSION) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it was created with an older version:\n" + duneVersion);
    }

    if(savegameVersion > SAVEGAMEVERSION) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it was created with a newer version:\n" + duneVersion);
    }
}
