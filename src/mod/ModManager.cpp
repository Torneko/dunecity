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

#include <mod/ModManager.h>
#include <mod/CustomHouseConfig.h>
#include <mod/ModMentatConfig.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/TextManager.h>
#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/exceptions.h>
#include <Definitions.h>
#include <globals.h>
#include <config.h>

#include <SDL.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstdio>
#include <climits>
#include <list>

// File names
static const char* ACTIVE_MOD_FILE = "active_mod.txt";
static const char* MOD_INI_FILE = "mod.ini";
static const char* OBJECT_DATA_FILE = "ObjectData.ini";
static const char* QUANTBOT_CONFIG_FILE = "QuantBot Config.ini";
static const char* GAME_OPTIONS_FILE = "GameOptions.ini";
static const char* VANILLA_MOD_NAME = "vanilla";
static const char* TORNIE_MOD_NAME = "Tornie";
static const char* TORNIE_LITE_MOD_NAME = "TornieLite";
static const char* JERICHO_MOD_NAME = "Jericho";

// Install config file names (with .default suffix)
static const char* OBJECT_DATA_DEFAULT = "ObjectData.ini.default";
static const char* QUANTBOT_CONFIG_DEFAULT = "QuantBot Config.ini.default";
static const char* CUSTOM_HOUSE_CONFIG = "CustomHouse.ini";

static CustomHouseInfo makeGuestCustomHouse(const std::string& activeModName) {
    CustomHouseInfo info;
    if(activeModName == TORNIE_MOD_NAME) {
        info.enabled = true;
        info.displayName = "Tharpique";
        info.scenarioLetter = 'T';
        info.regionPrefix = "THA";
        info.paletteIndex = 136;
        info.fallbackHouse = HOUSE_MERCENARY;
        info.heraldAsset = "HeraldTharpique.png";
        info.houseNameVoiceAsset = "OTHARP.VOC";
        info.voicePlaybackRate = 1.06;
        info.voiceGain = 1.15;
    } else if(activeModName == JERICHO_MOD_NAME) {
        info.enabled = true;
        info.displayName = "Corruptique";
        info.scenarioLetter = 'C';
        info.regionPrefix = "COR";
        info.paletteIndex = 192;
        info.fallbackHouse = HOUSE_ORDOS;
        info.heraldAsset = "HeraldCorruptique.png";
        info.houseNameVoiceAsset = "CORRUPT.VOC";
        info.voicePlaybackRate = 1.06;
        info.voiceGain = 1.15;
    }
    return info;
}

ModManager& ModManager::instance() {
    static ModManager instance;
    return instance;
}

ModManager::ModManager()
    : activeMod(VANILLA_MOD_NAME)
    , checksumsDirty(true)
    , initialized(false)
{
}

bool ModManager::isInitialized() const {
    return initialized;
}

ModManager::~ModManager() = default;

void ModManager::initialize() {
    // Get mods base path in user config directory
    char tmp[FILENAME_MAX];
    if (fnkdat("mods", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT) < 0) {
        THROW(std::runtime_error, "fnkdat() failed for mods directory!");
    }
    modsBasePath = std::string(tmp);

    // Create mods directory if it doesn't exist
    // Note: createDir() handles "already exists" case gracefully
    createDir(modsBasePath);

    // Seed vanilla mod if needed
    if (!modExists(VANILLA_MOD_NAME) || vanillaNeedsReseed()) {
        seedVanillaFromDefaults();
    }


    // Keep the three Tornie distributions shipped with this build complete in
    // the user profile. Extra user files are preserved; only bundled files are
    // restored when a payload is absent, incomplete, or from another build.
    const char* bundledTornieMods[] = {
        TORNIE_MOD_NAME,
        TORNIE_LITE_MOD_NAME,
        JERICHO_MOD_NAME
    };
    for(const char* modName : bundledTornieMods) {
        if(!modExists(modName) || bundledTornieModNeedsReseed(modName)) {
            seedBundledTornieModFromDefaults(modName);
        }
    }


    // Load active mod from file
    loadActiveMod();

    // Verify active mod exists, fall back to vanilla if not
    if (!modExists(activeMod)) {
        SDL_Log("ModManager: Active mod '%s' not found, falling back to vanilla", activeMod.c_str());
        activeMod = VANILLA_MOD_NAME;
        saveActiveMod();
    }

    const ModInfo activeInfo = readModIni(getModPath(activeMod));
    activeCustomHouse = activeInfo.customHouse;
    activeGuestCustomHouse = makeGuestCustomHouse(activeMod);
    activeMentats = activeInfo.mentats;
    initialized = true;
    // Resolve faction colors on cold startup too, before any menu or editor
    // graphics are requested. Jericho uses different identities in slots 6-8.
    resetHouseVisualHouseMapping();
    SDL_Log("ModManager: Initialized with active mod '%s'", activeMod.c_str());
}

std::string ModManager::getActiveModName() const {
    return activeMod;
}

const CustomHouseInfo& ModManager::getActiveCustomHouseInfo() const {
    return activeCustomHouse;
}

const CustomHouseInfo& ModManager::getCustomHouseInfo(int house) const {
    static const CustomHouseInfo noHouse;
    if(house == HOUSE_CUSTOM) return activeCustomHouse;
    if(house == HOUSE_THARPIQUE) return activeGuestCustomHouse;
    return noHouse;
}

bool ModManager::isCustomHouseRegistered() const {
    return initialized && activeCustomHouse.enabled;
}

bool ModManager::isCustomHouseRegistered(int house) const {
    return initialized && getCustomHouseInfo(house).enabled;
}

const ModMentatInfo& ModManager::getActiveMentatInfo(int house) const {
    static const ModMentatInfo noOverride;
    static const ModMentatInfo vanillaChani = [] {
        ModMentatInfo info;
        info.enabled = true;
        info.identityHouse = HOUSE_FREMEN;
        info.backgroundAsset = "ChaniMentat.png";
        info.eyesAsset = "ChaniMentatEyes.png";
        info.mouthAsset = "ChaniMentatMouth.png";
        info.eyesFrames = 5;
        info.eyesFrameRate = 0.5;
        info.doubleEyes = true;
        info.eyesX = 128;
        info.eyesY = 160;
        info.mouthFrames = 5;
        info.mouthFrameRate = 5.0;
        info.doubleMouth = true;
        info.mouthX = 112;
        info.mouthY = 192;
        info.useBaseExtras = false;
        return info;
    }();
    if(house >= HOUSE_WILDSPADE && house <= HOUSE_THARPIQUE
       && (activeMod == TORNIE_MOD_NAME || activeMod == JERICHO_MOD_NAME)) {
        house = HOUSE_NEUTRAL + (house - HOUSE_WILDSPADE);
    }
    if(initialized && activeMod == VANILLA_MOD_NAME
       && (house == HOUSE_NEUTRAL || house == HOUSE_REBELS)) {
        return vanillaChani;
    }
    if(!initialized || house < 0 || static_cast<std::size_t>(house) >= activeMentats.size()) {
        return noOverride;
    }
    return activeMentats[house];
}

int ModManager::getEffectiveMentatIdentity(int house) const {
    const ModMentatInfo& overrideInfo = getActiveMentatInfo(house);
    int identity = overrideInfo.enabled && overrideInfo.identityHouse >= 0
        ? overrideInfo.identityHouse
        : house;

    if(identity == HOUSE_CUSTOM) {
        const CustomHouseInfo& info = getCustomHouseInfo(house);
        identity = info.enabled ? info.fallbackHouse : HOUSE_HARKONNEN;
    } else if(identity >= HOUSE_WILDSPADE && identity <= HOUSE_THARPIQUE) {
        const int localIdentity = HOUSE_NEUTRAL + (identity - HOUSE_WILDSPADE);
        if(localIdentity == HOUSE_CUSTOM) {
            identity = activeGuestCustomHouse.enabled
                ? activeGuestCustomHouse.fallbackHouse
                : HOUSE_HARKONNEN;
        } else {
            identity = localIdentity;
        }
    }

    switch(identity) {
        case HOUSE_HARKONNEN:
        case HOUSE_ATREIDES:
        case HOUSE_ORDOS:
        case HOUSE_FREMEN:
        case HOUSE_SARDAUKAR:
        case HOUSE_MERCENARY:
            return identity;
        case HOUSE_NEUTRAL:
            return activeMod == VANILLA_MOD_NAME ? HOUSE_FREMEN : HOUSE_ATREIDES;
        case HOUSE_REBELS:
            return activeMod == VANILLA_MOD_NAME ? HOUSE_FREMEN : HOUSE_HARKONNEN;
        default:
            return HOUSE_HARKONNEN;
    }
}



bool ModManager::setActiveMod(const std::string& name) {
    if (!modExists(name)) {
        SDL_Log("ModManager: Cannot activate mod '%s' - does not exist", name.c_str());
        return false;
    }

    activeMod = name;
    const ModInfo activeInfo = readModIni(getModPath(activeMod));
    activeCustomHouse = activeInfo.customHouse;
    activeGuestCustomHouse = makeGuestCustomHouse(activeMod);
    activeMentats = activeInfo.mentats;
    checksumsDirty = true;
    saveActiveMod();

    // Active-mod palettes and runtime visual slots must change atomically.
    // Otherwise cached Jericho ramps leak into Tornie and other mods.
    resetHouseVisualHouseMapping();
    loadCustomPalette();
    applyCustomPaletteRuntimeHouseRamps();

    if(pTextManager != nullptr) {
        pTextManager->loadData();
    }
    if(pGFXManager != nullptr) {
        pGFXManager->reloadAllObjectGraphicsForActiveMod();
    }
    if(pSFXManager != nullptr) {
        pSFXManager->reloadVoices();
    }

    SDL_Log("ModManager: Activated mod '%s'", name.c_str());
    return true;
}

bool ModManager::modExists(const std::string& name) const {
    std::string modPath = getModPath(name);
    // Just check if mod.json exists - if file exists, directory must exist
    return existsFile(modPath + "/" + MOD_INI_FILE);
}

std::vector<ModInfo> ModManager::listMods() const {
    std::vector<ModInfo> mods;

    SDL_Log("ModManager::listMods() - modsBasePath: %s", modsBasePath.c_str());

    // List directories in mods folder
    std::list<std::string> entries = getDirectoryList(modsBasePath);

    SDL_Log("ModManager::listMods() - found %zu directory entries", entries.size());

    for (const std::string& entry : entries) {
        std::string modPath = modsBasePath + "/" + entry;
        std::string jsonPath = modPath + "/" + MOD_INI_FILE;
        SDL_Log("ModManager::listMods() - checking entry '%s', json exists: %d",
                entry.c_str(), existsFile(jsonPath) ? 1 : 0);
        if (existsFile(jsonPath)) {
            mods.push_back(getModInfo(entry));
        }
    }

    SDL_Log("ModManager::listMods() - returning %zu mods", mods.size());
    return mods;
}

ModInfo ModManager::getModInfo(const std::string& name) const {
    ModInfo info;
    info.name = name;

    std::string modPath = getModPath(name);

    // Check which files exist
    info.hasObjectData = existsFile(modPath + "/" + OBJECT_DATA_FILE);
    info.hasQuantBotConfig = existsFile(modPath + "/" + QUANTBOT_CONFIG_FILE);
    info.hasGameOptions = existsFile(modPath + "/" + GAME_OPTIONS_FILE);

    // Read mod.json if it exists
    std::string jsonPath = modPath + "/" + MOD_INI_FILE;
    if (existsFile(jsonPath)) {
        info = readModIni(modPath);
        info.name = name;  // Ensure name matches folder
    } else {
        info.displayName = name;
        info.author = "Unknown";
        info.description = "";
        info.gameVersion = "";
    }

    // Ensure string fields are never null (defensive)
    if(info.displayName.empty()) info.displayName = name;
    if(info.author.empty()) info.author = "Unknown";
    if(info.description.empty()) info.description = "";
    if(info.gameVersion.empty()) info.gameVersion = "";

    return info;
}

std::string ModManager::getActiveObjectDataPath() const {
    std::string modPath = getModPath(activeMod);
    std::string filePath = modPath + "/" + OBJECT_DATA_FILE;

    // Fall back to vanilla if mod doesn't have this file
    if (!existsFile(filePath) && activeMod != VANILLA_MOD_NAME) {
        filePath = getModPath(VANILLA_MOD_NAME) + "/" + OBJECT_DATA_FILE;
    }

    // Final fallback to install template if vanilla also missing
    if (!existsFile(filePath)) {
        std::string templatePath = getInstallConfigPath() + "/" + OBJECT_DATA_DEFAULT;
        if (existsFile(templatePath)) {
            SDL_Log("ModManager: ObjectData.ini missing, using template: %s", templatePath.c_str());
            return templatePath;
        }
    }

    return filePath;
}

std::string ModManager::getActiveQuantBotConfigPath() const {
    std::string modPath = getModPath(activeMod);
    std::string filePath = modPath + "/" + QUANTBOT_CONFIG_FILE;

    // Fall back to vanilla if mod doesn't have this file
    if (!existsFile(filePath) && activeMod != VANILLA_MOD_NAME) {
        filePath = getModPath(VANILLA_MOD_NAME) + "/" + QUANTBOT_CONFIG_FILE;
    }

    // Final fallback to install template if vanilla also missing
    if (!existsFile(filePath)) {
        std::string templatePath = getInstallConfigPath() + "/" + QUANTBOT_CONFIG_DEFAULT;
        if (existsFile(templatePath)) {
            SDL_Log("ModManager: QuantBot Config.ini missing, using template: %s", templatePath.c_str());
            return templatePath;
        }
    }

    return filePath;
}

std::string ModManager::getActiveGameOptionsPath() const {
    std::string modPath = getModPath(activeMod);
    std::string filePath = modPath + "/" + GAME_OPTIONS_FILE;

    // Fall back to vanilla if mod doesn't have this file
    if (!existsFile(filePath) && activeMod != VANILLA_MOD_NAME) {
        filePath = getModPath(VANILLA_MOD_NAME) + "/" + GAME_OPTIONS_FILE;
    }

    return filePath;
}

SettingsClass::GameOptionsClass ModManager::loadEffectiveGameOptions(
    const SettingsClass::GameOptionsClass& baseOptions) const {

    // Start with base options
    SettingsClass::GameOptionsClass result = baseOptions;

    // Random spice blooms are enabled by default in the Tornie-family mods.
    // The in-game Game Options window can still override this per match.
    if(initialized && (activeMod == "Tornie" || activeMod == "TornieLite"
                       || activeMod == "Tornie Lite" || activeMod == "Jericho")) {
        result.randomSpiceBlooms = true;
    }

    // If vanilla mod or no mod system, just return base options
    if (!initialized || activeMod == VANILLA_MOD_NAME) {
        return result;
    }

    // Try to load mod's GameOptions.ini
    std::string gameOptionsPath = getActiveGameOptionsPath();
    if (!existsFile(gameOptionsPath)) {
        return result;
    }

    try {
        std::ifstream file(gameOptionsPath);
        if (!file.is_open()) {
            return result;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            if (line[0] == '[') continue; // Skip section headers

            // Parse key = value
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            // Trim whitespace
            auto trim = [](std::string& s) {
                while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(0, 1);
                while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
            };
            trim(key);
            trim(value);

            // Apply overrides
            auto parseBool = [](const std::string& v) {
                return v == "true" || v == "1" || v == "yes";
            };

            if (key == "Game Speed") result.gameSpeed = std::stoi(value);
            else if (key == "Concrete Required") result.concreteRequired = parseBool(value);
            else if (key == "Structures Degrade On Concrete") result.structuresDegradeOnConcrete = parseBool(value);
            else if (key == "Fog of War") result.fogOfWar = parseBool(value);
            else if (key == "Start with Explored Map") result.startWithExploredMap = parseBool(value);
            else if (key == "Instant Build") result.instantBuild = parseBool(value);
            else if (key == "Only One Palace") result.onlyOnePalace = parseBool(value);
            else if (key == "Rocket-Turrets Need Power") result.rocketTurretsNeedPower = parseBool(value);
            else if (key == "Sandworms Respawn") result.sandwormsRespawn = parseBool(value);
            else if (key == "Killed Sandworms Drop Spice") result.killedSandwormsDropSpice = parseBool(value);
            else if (key == "Random Spice Blooms") result.randomSpiceBlooms = parseBool(value);
            else if (key == "Manual Carryall Drops") result.manualCarryallDrops = parseBool(value);
            else if (key == "Maximum Number of Units Override") result.maximumNumberOfUnitsOverride = std::stoi(value);
            else if (key == "Maximum Number of Harvesters Override") result.maximumNumberOfHarvestersOverride = std::stoi(value);
            else if (key == "Immortal Human Player") result.immortalHumanPlayer = parseBool(value);
        }

        file.close();
        SDL_Log("ModManager: Loaded game options from mod '%s'", activeMod.c_str());

    } catch (const std::exception& e) {
        SDL_Log("ModManager: Warning - failed to load game options from mod: %s", e.what());
    }

    return result;
}

ModChecksums ModManager::getEffectiveChecksums() const {
    if (checksumsDirty) {
        // Force recalculation by casting away const (cache pattern)
        const_cast<ModManager*>(this)->updateChecksums();
    }
    return cachedChecksums;
}

std::string ModManager::hashFileCanonical(const std::string& path) {
    // Compute canonical FNV-1a hash of an INI-style file.
    // "Canonical" means: skip comments, skip empty lines, trim whitespace,
    // so cosmetic edits don't change the hash.
    std::ifstream file(path);
    if (!file.is_open()) {
        return "FILE_NOT_FOUND";
    }

    std::string contents;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            continue;
        }
        size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Strip inline " ;" comments (whitespace-prefixed semicolon).
        size_t commentPos = line.find(" ;");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
            end = line.find_last_not_of(" \t");
            if (end != std::string::npos) {
                line = line.substr(0, end + 1);
            }
        }

        if (!line.empty()) {
            contents += line + '\n';
        }
    }
    file.close();

    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;
    for (char c : contents) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= prime;
    }

    char hashStr[17];
    snprintf(hashStr, sizeof(hashStr), "%016llx", (unsigned long long)hash);
    return std::string(hashStr);
}

bool ModManager::installedObjectDataDiffersFromDefaults(const std::string& modName) const {
    const std::string installed = getModPath(modName) + "/" + OBJECT_DATA_FILE;
    std::string defaults = getInstallConfigPath() + "/" + OBJECT_DATA_DEFAULT;

    if (modName == TORNIE_MOD_NAME) {
        const std::string candidatePaths[] = {
            getDuneLegacyDataDir() + "/mods/" + TORNIE_MOD_NAME,
            getDuneLegacyDataDir() + "/../mods/" + TORNIE_MOD_NAME,
            getDuneLegacyDataDir() + "/../../mods/" + TORNIE_MOD_NAME,
            getDuneLegacyDataDir() + "/../../../mods/" + TORNIE_MOD_NAME
        };
        for (const std::string& candidatePath : candidatePaths) {
            const std::string candidateObjectData = candidatePath + "/" + OBJECT_DATA_FILE;
            if (existsFile(candidateObjectData)) {
                defaults = candidateObjectData;
                break;
            }
        }
    }

    if (!existsFile(installed) || !existsFile(defaults)) {
        return false;  // Existence check already triggers reseed; don't double-fire.
    }

    return hashFileCanonical(installed) != hashFileCanonical(defaults);
}

void ModManager::updateChecksums() {
    cachedChecksums.objectData = hashFileCanonical(getActiveObjectDataPath());
    cachedChecksums.quantBotConfig = hashFileCanonical(getActiveQuantBotConfigPath());

    // Game options: always hash the mod's GameOptions.ini file (for all mods including vanilla).
    // This ensures all players with the same mod version get the same checksum.
    // Runtime game settings (from Dune Legacy.ini) are synced separately via the game lobby.
    const std::string gameOptionsPath = getActiveGameOptionsPath();
    if (existsFile(gameOptionsPath)) {
        cachedChecksums.gameOptions = hashFileCanonical(gameOptionsPath);
    } else {
        // No GameOptions.ini file - use a constant hash to indicate "no game options file"
        cachedChecksums.gameOptions = "0000000000000000";
    }

    const std::string customHousePath = getModPath(activeMod) + "/" + CUSTOM_HOUSE_CONFIG;
    cachedChecksums.customHouse = existsFile(customHousePath)
        ? hashFileCanonical(customHousePath)
        : std::string();

    // Appending an empty optional hash preserves the exact legacy checksum.
    std::string combined = cachedChecksums.objectData + cachedChecksums.quantBotConfig
        + cachedChecksums.gameOptions + cachedChecksums.customHouse;
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;
    for (char c : combined) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= prime;
    }
    char hashStr[17];
    snprintf(hashStr, sizeof(hashStr), "%016llx", (unsigned long long)hash);
    cachedChecksums.combined = std::string(hashStr);

    checksumsDirty = false;
    SDL_Log("ModManager: Updated checksums - OD:%s QB:%s GO:%s Combined:%s",
            cachedChecksums.objectData.c_str(), cachedChecksums.quantBotConfig.c_str(),
            cachedChecksums.gameOptions.c_str(), cachedChecksums.combined.c_str());
}

void ModManager::setChecksums(const std::string& objectDataHash,
                              const std::string& quantBotHash,
                              const std::string& gameOptionsHash) {
    // Set checksums from externally computed values (e.g., from loaded in-memory data)
    cachedChecksums.objectData = objectDataHash;
    cachedChecksums.quantBotConfig = quantBotHash;
    cachedChecksums.gameOptions = gameOptionsHash;

    // Compute combined hash
    std::string combined = objectDataHash + quantBotHash + gameOptionsHash;
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;
    for (char c : combined) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= prime;
    }
    char hashStr[17];
    snprintf(hashStr, sizeof(hashStr), "%016llx", (unsigned long long)hash);
    cachedChecksums.combined = std::string(hashStr);

    checksumsDirty = false;
}

bool ModManager::createMod(const std::string& name, const std::string& baseMod) {
    if (name.empty() || name == VANILLA_MOD_NAME) {
        SDL_Log("ModManager: Invalid mod name '%s'", name.c_str());
        return false;
    }

    if (modExists(name)) {
        SDL_Log("ModManager: Mod '%s' already exists", name.c_str());
        return false;
    }

    if (!modExists(baseMod)) {
        SDL_Log("ModManager: Base mod '%s' does not exist", baseMod.c_str());
        return false;
    }

    std::string newModPath = getModPath(name);
    std::string baseModPath = getModPath(baseMod);

    SDL_Log("ModManager::createMod - newModPath: %s", newModPath.c_str());
    SDL_Log("ModManager::createMod - baseModPath: %s", baseModPath.c_str());

    // Create mod directory
    if (!createDir(newModPath)) {
        SDL_Log("ModManager: Failed to create directory for mod '%s'", name.c_str());
        return false;
    }

    // Copy files from base mod
    const char* filesToCopy[] = {
        OBJECT_DATA_FILE,
        QUANTBOT_CONFIG_FILE,
        GAME_OPTIONS_FILE
    };

    for (const char* file : filesToCopy) {
        std::string srcPath = baseModPath + "/" + file;
        std::string dstPath = newModPath + "/" + file;

        SDL_Log("ModManager::createMod - copying %s -> %s (exists: %d)",
                srcPath.c_str(), dstPath.c_str(), existsFile(srcPath) ? 1 : 0);

        if (existsFile(srcPath)) {
            if (copyFile(srcPath, dstPath)) {
                SDL_Log("ModManager::createMod - copied %s successfully", file);
            } else {
                SDL_Log("ModManager::createMod - FAILED to copy %s", file);
            }
        } else {
            SDL_Log("ModManager::createMod - source file %s does not exist!", srcPath.c_str());
        }
    }

    // Create mod.ini
    ModInfo info;
    info.name = name;
    info.displayName = name;
    info.author = "User";
    info.description = "Custom mod based on " + baseMod;
    info.gameVersion = VERSION;
    writeModInfo(newModPath, info);

    SDL_Log("ModManager: Created mod '%s' from '%s'", name.c_str(), baseMod.c_str());
    return true;
}

bool ModManager::deleteMod(const std::string& name) {
    if (name == VANILLA_MOD_NAME) {
        SDL_Log("ModManager: Cannot delete vanilla mod");
        return false;
    }

    if (!modExists(name)) {
        SDL_Log("ModManager: Mod '%s' does not exist", name.c_str());
        return false;
    }

    // If this is the active mod, switch to vanilla first
    if (activeMod == name) {
        setActiveMod(VANILLA_MOD_NAME);
    }

    std::string modPath = getModPath(name);

    // Delete mod files
    const char* filesToDelete[] = {
        OBJECT_DATA_FILE,
        QUANTBOT_CONFIG_FILE,
        GAME_OPTIONS_FILE,
        MOD_INI_FILE
    };

    for (const char* file : filesToDelete) {
        std::string filePath = modPath + "/" + file;
        if (existsFile(filePath)) {
            deleteFile(filePath);
        }
    }

    // Delete mod directory
    deleteFile(modPath);

    SDL_Log("ModManager: Deleted mod '%s'", name.c_str());
    return true;
}

bool ModManager::saveReceivedMod(const std::string& modName, const std::string& packagedData) {
    if (modName.empty() || modName == VANILLA_MOD_NAME) {
        SDL_Log("ModManager: Invalid mod name for save: '%s'", modName.c_str());
        return false;
    }

    SDL_Log("ModManager: Saving received mod '%s' (%zu bytes)", modName.c_str(), packagedData.size());

    // Create mod directory (createDir handles "already exists" gracefully)
    std::string modPath = getModPath(modName);
    createDir(modPath);

    // Unpack the packaged data
    // Format: numFiles (4 bytes) + [nameLen (4 bytes) + name + dataLen (4 bytes) + data] * numFiles
    if (packagedData.size() < sizeof(uint32_t)) {
        SDL_Log("ModManager: Packaged data too small");
        return false;
    }

    size_t offset = 0;

    // Read number of files
    uint32_t numFiles;
    memcpy(&numFiles, packagedData.data() + offset, sizeof(numFiles));
    offset += sizeof(numFiles);

    SDL_Log("ModManager: Unpacking %u files", numFiles);

    for (uint32_t i = 0; i < numFiles; i++) {
        if (offset + sizeof(uint32_t) > packagedData.size()) {
            SDL_Log("ModManager: Unexpected end of data at file %u (name length)", i);
            return false;
        }

        // Read file name length
        uint32_t nameLen;
        memcpy(&nameLen, packagedData.data() + offset, sizeof(nameLen));
        offset += sizeof(nameLen);

        if (offset + nameLen > packagedData.size()) {
            SDL_Log("ModManager: Unexpected end of data at file %u (name)", i);
            return false;
        }

        // Read file name
        std::string fileName(packagedData.data() + offset, nameLen);
        offset += nameLen;

        if (offset + sizeof(uint32_t) > packagedData.size()) {
            SDL_Log("ModManager: Unexpected end of data at file %u (data length)", i);
            return false;
        }

        // Read file data length
        uint32_t dataLen;
        memcpy(&dataLen, packagedData.data() + offset, sizeof(dataLen));
        offset += sizeof(dataLen);

        if (offset + dataLen > packagedData.size()) {
            SDL_Log("ModManager: Unexpected end of data at file %u (data)", i);
            return false;
        }

        // Read file data
        std::string fileData(packagedData.data() + offset, dataLen);
        offset += dataLen;

        // Validate filename (prevent path traversal)
        if (fileName.find('/') != std::string::npos ||
            fileName.find('\\') != std::string::npos ||
            fileName.find("..") != std::string::npos) {
            SDL_Log("ModManager: Invalid filename: '%s'", fileName.c_str());
            continue;
        }

        // Write file
        std::string filePath = modPath + "/" + fileName;
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            SDL_Log("ModManager: Failed to create file: %s", filePath.c_str());
            continue;
        }

        file.write(fileData.data(), fileData.size());
        file.close();

        SDL_Log("ModManager: Saved file '%s' (%u bytes)", fileName.c_str(), dataLen);
    }

    SDL_Log("ModManager: Mod '%s' saved successfully", modName.c_str());

    // Ensure mod.ini exists - create a basic one if missing
    std::string modIniPath = modPath + "/" + MOD_INI_FILE;
    if (!existsFile(modIniPath)) {
        SDL_Log("ModManager: Creating default mod.ini for received mod '%s'", modName.c_str());
        ModInfo defaultInfo;
        defaultInfo.name = modName;
        defaultInfo.displayName = modName;
        defaultInfo.author = "Downloaded from host";
        defaultInfo.description = "Mod received from multiplayer host";
        defaultInfo.gameVersion = VERSION;
        defaultInfo.version = "";
        writeModInfo(modPath, defaultInfo);
    }

    checksumsDirty = true;  // Checksums need recalculation
    return true;
}

void ModManager::seedVanillaFromDefaults() {
    SDL_Log("ModManager: Seeding vanilla mod from install defaults...");

    std::string vanillaPath = getModPath(VANILLA_MOD_NAME);
    std::string installConfigPath = getInstallConfigPath();

    // Create vanilla directory (createDir handles "already exists" gracefully)
    createDir(vanillaPath);

    // Copy ObjectData.ini.default -> ObjectData.ini
    std::string srcObjectData = installConfigPath + "/" + OBJECT_DATA_DEFAULT;
    std::string dstObjectData = vanillaPath + "/" + OBJECT_DATA_FILE;
    if (existsFile(srcObjectData)) {
        copyFile(srcObjectData, dstObjectData);
        SDL_Log("ModManager: Copied %s", OBJECT_DATA_FILE);
    } else {
        SDL_Log("ModManager: Warning - %s not found at %s", OBJECT_DATA_DEFAULT, srcObjectData.c_str());
    }

    // Copy QuantBot Config.ini.default -> QuantBot Config.ini
    std::string srcQuantBot = installConfigPath + "/" + QUANTBOT_CONFIG_DEFAULT;
    std::string dstQuantBot = vanillaPath + "/" + QUANTBOT_CONFIG_FILE;
    if (existsFile(srcQuantBot)) {
        copyFile(srcQuantBot, dstQuantBot);
        SDL_Log("ModManager: Copied %s", QUANTBOT_CONFIG_FILE);
    } else {
        SDL_Log("ModManager: Warning - %s not found at %s", QUANTBOT_CONFIG_DEFAULT, srcQuantBot.c_str());
    }

    // Create GameOptions.ini with defaults (extracted from Dune Legacy.ini defaults)
    std::string gameOptionsPath = vanillaPath + "/" + GAME_OPTIONS_FILE;
    std::ofstream gameOptionsFile(gameOptionsPath);
    if (gameOptionsFile.is_open()) {
        gameOptionsFile << "# Vanilla Game Options (default values)\n";
        gameOptionsFile << "[Game Options]\n";
        gameOptionsFile << "Game Speed = 16\n";
        gameOptionsFile << "Concrete Required = true\n";
        gameOptionsFile << "Structures Degrade On Concrete = true\n";
        gameOptionsFile << "Fog of War = false\n";
        gameOptionsFile << "Start with Explored Map = false\n";
        gameOptionsFile << "Instant Build = false\n";
        gameOptionsFile << "Only One Palace = false\n";
        gameOptionsFile << "Rocket-Turrets Need Power = false\n";
        gameOptionsFile << "Sandworms Respawn = false\n";
        gameOptionsFile << "Killed Sandworms Drop Spice = false\n";
        gameOptionsFile << "Manual Carryall Drops = false\n";
        gameOptionsFile << "Maximum Number of Units Override = 0\n";
        gameOptionsFile << "Maximum Number of Harvesters Override = -1\n";
        gameOptionsFile.close();
        SDL_Log("ModManager: Created %s", GAME_OPTIONS_FILE);
    }

    // Create mod.json
    ModInfo info;
    info.name = VANILLA_MOD_NAME;
    info.displayName = "Vanilla";
    info.author = "Dune Legacy Tornie";
    info.description = "Default game settings";
    info.gameVersion = VERSION;
    writeModInfo(vanillaPath, info);

    SDL_Log("ModManager: Vanilla mod seeded successfully");
}



static std::string findBundledTornieModPath(const std::string& modName) {
    const std::string candidatePaths[] = {
        getDuneLegacyDataDir() + "/mods/" + modName,
        getDuneLegacyDataDir() + "/../mods/" + modName,
        getDuneLegacyDataDir() + "/../../mods/" + modName,
        getDuneLegacyDataDir() + "/../../../mods/" + modName
    };
    for(const std::string& candidatePath : candidatePaths) {
        if(std::filesystem::is_directory(candidatePath)
           && existsFile(candidatePath + "/" + MOD_INI_FILE)) {
            return candidatePath;
        }
    }

    return std::string();
}

void ModManager::seedBundledTornieModFromDefaults(const std::string& modName) {
    SDL_Log("ModManager: Seeding bundled %s mod from install defaults...", modName.c_str());

    const std::string destinationPath = getModPath(modName);
    const std::string installModsPath = findBundledTornieModPath(modName);

    if(!std::filesystem::is_directory(installModsPath)) {
        SDL_Log("ModManager: Warning - bundled %s mod was not found in the install", modName.c_str());
        return;
    }

    try {
        // Overwrite bundled files so incomplete or stale managed payloads heal,
        // but do not remove additional files that the user may have added.
        std::filesystem::copy(installModsPath, destinationPath,
            std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing);
        SDL_Log("ModManager: %s mod seeded successfully from %s",
                modName.c_str(), installModsPath.c_str());
    } catch(const std::exception& e) {
        SDL_Log("ModManager: Warning - %s mod seed failed: %s", modName.c_str(), e.what());
    }
}

// Kept as the public compatibility entry point used by older callers.
void ModManager::seedTornieFromDefaults() {
    seedBundledTornieModFromDefaults(TORNIE_MOD_NAME);
}





bool ModManager::bundledTornieModNeedsReseed(const std::string& modName) const {
    namespace fs = std::filesystem;

    const fs::path installedPath(getModPath(modName));
    const std::string bundledPathString = findBundledTornieModPath(modName);
    if(bundledPathString.empty()) {
        SDL_Log("ModManager: Cannot verify bundled %s payload because its install source is missing",
                modName.c_str());
        return false;
    }
    const fs::path bundledPath(bundledPathString);

    try {
        // Every directory and regular file from the shipped payload must exist
        // in Roaming. File sizes catch interrupted and truncated copies without
        // hashing hundreds of large graphics and sound files on every startup.
        for(const fs::directory_entry& bundledEntry : fs::recursive_directory_iterator(bundledPath)) {
            const fs::path relativePath = fs::relative(bundledEntry.path(), bundledPath);
            const fs::path installedEntry = installedPath / relativePath;

            if(bundledEntry.is_directory()) {
                if(!fs::is_directory(installedEntry)) {
                    SDL_Log("ModManager: %s is missing directory %s, needs reseed",
                            modName.c_str(), relativePath.generic_string().c_str());
                    return true;
                }
            } else if(bundledEntry.is_regular_file()) {
                if(!fs::is_regular_file(installedEntry)
                   || fs::file_size(installedEntry) != bundledEntry.file_size()) {
                    SDL_Log("ModManager: %s is missing or has an incomplete file %s, needs reseed",
                            modName.c_str(), relativePath.generic_string().c_str());
                    return true;
                }
            }
        }

        // These small files define the mod and its gameplay. Hash them so a
        // same-sized stale or damaged copy is also repaired.
        const char* essentialFiles[] = {
            MOD_INI_FILE,
            OBJECT_DATA_FILE,
            QUANTBOT_CONFIG_FILE,
            GAME_OPTIONS_FILE,
            CUSTOM_HOUSE_CONFIG,
            "data/HeraldCorruptique.png",
            "manifest.json",
            "checksums.sha256"
        };
        for(const char* fileName : essentialFiles) {
            const std::string bundledFile = (bundledPath / fileName).string();
            const std::string installedFile = (installedPath / fileName).string();
            if(existsFile(bundledFile)
               && hashFileCanonical(installedFile) != hashFileCanonical(bundledFile)) {
                SDL_Log("ModManager: %s essential file %s differs from the bundled copy, needs reseed",
                        modName.c_str(), fileName);
                return true;
            }
        }
    } catch(const std::exception& e) {
        SDL_Log("ModManager: Cannot verify bundled %s payload: %s; needs reseed",
                modName.c_str(), e.what());
        return true;
    }

    return false;
}

bool ModManager::tornieNeedsReseed() const {
    return bundledTornieModNeedsReseed(TORNIE_MOD_NAME);
}


bool ModManager::vanillaNeedsReseed() const {
    std::string vanillaPath = getModPath(VANILLA_MOD_NAME);

    // Check if all required files exist
    std::string objectDataPath = vanillaPath + "/" + OBJECT_DATA_FILE;
    std::string quantBotPath = vanillaPath + "/" + QUANTBOT_CONFIG_FILE;
    std::string gameOptionsPath = vanillaPath + "/" + GAME_OPTIONS_FILE;
    std::string modIniPath = vanillaPath + "/" + MOD_INI_FILE;

    if (!existsFile(objectDataPath)) {
        SDL_Log("ModManager: Vanilla missing %s, needs reseed", OBJECT_DATA_FILE);
        return true;
    }
    if (!existsFile(quantBotPath)) {
        SDL_Log("ModManager: Vanilla missing %s, needs reseed", QUANTBOT_CONFIG_FILE);
        return true;
    }
    if (!existsFile(gameOptionsPath)) {
        SDL_Log("ModManager: Vanilla missing %s, needs reseed", GAME_OPTIONS_FILE);
        return true;
    }
    if (!existsFile(modIniPath)) {
        SDL_Log("ModManager: Vanilla missing %s, needs reseed", MOD_INI_FILE);
        return true;
    }

    // Also reseed if game version doesn't match
    ModInfo info = readModIni(vanillaPath);
    if (info.gameVersion != VERSION) {
        SDL_Log("ModManager: Vanilla version mismatch (%s vs %s), needs reseed",
                info.gameVersion.c_str(), VERSION);
        return true;
    }

    if (installedObjectDataDiffersFromDefaults(VANILLA_MOD_NAME)) {
        SDL_Log("ModManager: Vanilla %s drifted from defaults, needs reseed", OBJECT_DATA_FILE);
        return true;
    }

    return false;
}

std::string ModManager::getModsBasePath() const {
    return modsBasePath;
}

std::string ModManager::getModPath(const std::string& name) const {
    if(name.empty()) {
        return modsBasePath;  // Safety: avoid trailing slash + empty name
    }
    return modsBasePath + "/" + name;
}

void ModManager::loadActiveMod() {
    std::string activeModFile = modsBasePath + "/" + ACTIVE_MOD_FILE;

    if (existsFile(activeModFile)) {
        std::ifstream file(activeModFile);
        if (file.is_open()) {
            std::getline(file, activeMod);
            file.close();

            // Trim whitespace
            while (!activeMod.empty() && (activeMod.back() == '\n' || activeMod.back() == '\r' || activeMod.back() == ' ')) {
                activeMod.pop_back();
            }
        }
    }

    // Default to vanilla if empty or invalid
    if (activeMod.empty()) {
        activeMod = VANILLA_MOD_NAME;
    }
}

void ModManager::saveActiveMod() const {
    std::string activeModFile = modsBasePath + "/" + ACTIVE_MOD_FILE;

    std::ofstream file(activeModFile);
    if (file.is_open()) {
        file << activeMod << "\n";
        file.close();
    }
}

ModInfo ModManager::readModIni(const std::string& modPath) const {
    ModInfo info;
    std::string iniPath = modPath + "/" + MOD_INI_FILE;

    std::ifstream file(iniPath);
    if (!file.is_open()) {
        return info;
    }

    // Simple INI parsing (key = value format)
    auto trim = [](std::string& s) {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(0, 1);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\n' || s.back() == '\r')) s.pop_back();
    };

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') continue;

        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos) continue;

        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);

        trim(key);
        trim(value);

        if (key == "Display Name") info.displayName = value;
        else if (key == "Author") info.author = value;
        else if (key == "Description") info.description = value;
        else if (key == "Version") info.version = value;
        else if (key == "Game Version") info.gameVersion = value;
        else if (key == "Enables City Mode") {
        }
    }

    file.close();

    info.mentats.resize(NUM_HOUSE_COLOR_SLOTS);
    std::vector<bool> requestedMentatEnabled(NUM_HOUSE_COLOR_SLOTS, false);
    std::vector<bool> mentatFieldsValid(NUM_HOUSE_COLOR_SLOTS, true);
    std::ifstream mentatFile(iniPath);
    if(mentatFile.is_open()) {
        int mentatHouse = -1;
        while(std::getline(mentatFile, line)) {
            trim(line);
            if(line.empty() || line[0] == '#' || line[0] == ';') continue;

            if(line.front() == '[' && line.back() == ']') {
                std::string section = line.substr(1, line.size() - 2);
                trim(section);
                mentatHouse = -1;
                const std::string prefix = "Mentat ";
                if(section.rfind(prefix, 0) == 0) {
                    int parsedHouse = -1;
                    if(CustomHouseConfig::parseInteger(section.substr(prefix.size()), parsedHouse)
                       && parsedHouse >= 0 && parsedHouse < NUM_HOUSE_COLOR_SLOTS) {
                        mentatHouse = parsedHouse;
                    }
                }
                continue;
            }

            if(mentatHouse < 0) continue;
            const size_t mentatEqualsPos = line.find('=');
            if(mentatEqualsPos == std::string::npos) continue;

            std::string mentatKey = line.substr(0, mentatEqualsPos);
            std::string mentatValue = line.substr(mentatEqualsPos + 1);
            trim(mentatKey);
            trim(mentatValue);
            ModMentatInfo& mentat = info.mentats[mentatHouse];

            auto parseIntegerField = [&](int& destination) {
                if(!CustomHouseConfig::parseInteger(mentatValue, destination)) {
                    mentatFieldsValid[mentatHouse] = false;
                }
            };
            auto parseDoubleField = [&](double& destination) {
                if(!ModMentatConfig::parseDouble(mentatValue, destination)) {
                    mentatFieldsValid[mentatHouse] = false;
                }
            };
            auto parseBooleanField = [&](bool& destination) {
                if(!ModMentatConfig::parseBoolean(mentatValue, destination)) {
                    mentatFieldsValid[mentatHouse] = false;
                }
            };

            if(mentatKey == "Enabled") {
                bool requestedEnabled = false;
                if(ModMentatConfig::parseBoolean(mentatValue, requestedEnabled)) {
                    requestedMentatEnabled[mentatHouse] = requestedEnabled;
                } else {
                    mentatFieldsValid[mentatHouse] = false;
                }
            }
            else if(mentatKey == "Identity House") parseIntegerField(mentat.identityHouse);
            else if(mentatKey == "Background") mentat.backgroundAsset = mentatValue;
            else if(mentatKey == "Foreground") mentat.foregroundAsset = mentatValue;
            else if(mentatKey == "Eyes") mentat.eyesAsset = mentatValue;
            else if(mentatKey == "Eyes Frames") parseIntegerField(mentat.eyesFrames);
            else if(mentatKey == "Eyes Frame Rate") parseDoubleField(mentat.eyesFrameRate);
            else if(mentatKey == "Eyes Double") parseBooleanField(mentat.doubleEyes);
            else if(mentatKey == "Eyes Transparent Color") parseIntegerField(mentat.eyesTransparentColor);
            else if(mentatKey == "Eyes X") parseIntegerField(mentat.eyesX);
            else if(mentatKey == "Eyes Y") parseIntegerField(mentat.eyesY);
            else if(mentatKey == "Mouth") mentat.mouthAsset = mentatValue;
            else if(mentatKey == "Mouth Frames") parseIntegerField(mentat.mouthFrames);
            else if(mentatKey == "Mouth Frame Rate") parseDoubleField(mentat.mouthFrameRate);
            else if(mentatKey == "Mouth Double") parseBooleanField(mentat.doubleMouth);
            else if(mentatKey == "Mouth Transparent Color") parseIntegerField(mentat.mouthTransparentColor);
            else if(mentatKey == "Mouth X") parseIntegerField(mentat.mouthX);
            else if(mentatKey == "Mouth Y") parseIntegerField(mentat.mouthY);
            else if(mentatKey == "Use Base Extras") parseBooleanField(mentat.useBaseExtras);
        }
    }

    for(int house = 0; house < NUM_HOUSE_COLOR_SLOTS; ++house) {
        ModMentatInfo& mentat = info.mentats[house];
        mentat.enabled = requestedMentatEnabled[house]
            && mentatFieldsValid[house]
            && ModMentatConfig::isValid(mentat);
        if(requestedMentatEnabled[house] && !mentat.enabled) {
            SDL_Log("ModManager: Ignoring invalid Mentat %d configuration in %s", house, modPath.c_str());
        }
    }

    std::ifstream customFile(modPath + "/" + CUSTOM_HOUSE_CONFIG);
    if(customFile.is_open()) {
        std::string section;
        bool requestedEnabled = false;
        bool numericFieldsValid = true;
        while(std::getline(customFile, line)) {
            trim(line);
            if(line.empty() || line[0] == '#' || line[0] == ';') continue;
            if(line.front() == '[' && line.back() == ']') {
                section = line.substr(1, line.size() - 2);
                trim(section);
                continue;
            }
            if(section != "House") continue;

            const size_t customEqualsPos = line.find('=');
            if(customEqualsPos == std::string::npos) continue;
            std::string customKey = line.substr(0, customEqualsPos);
            std::string customValue = line.substr(customEqualsPos + 1);
            trim(customKey);
            trim(customValue);

            if(customKey == "Enabled") {
                requestedEnabled = (customValue == "true" || customValue == "1" || customValue == "yes");
            } else if(customKey == "Display Name") {
                info.customHouse.displayName = customValue;
            } else if(customKey == "Scenario Letter" && customValue.size() == 1) {
                char value = customValue[0];
                info.customHouse.scenarioLetter = (value >= 'a' && value <= 'z') ? static_cast<char>(value - 'a' + 'A') : value;
            } else if(customKey == "Region Prefix") {
                info.customHouse.regionPrefix = customValue;
                for(char& value : info.customHouse.regionPrefix) {
                    if(value >= 'a' && value <= 'z') value = static_cast<char>(value - 'a' + 'A');
                }
            } else if(customKey == "Palette Index") {
                if(!CustomHouseConfig::parseInteger(customValue, info.customHouse.paletteIndex)) {
                    numericFieldsValid = false;
                }
            } else if(customKey == "Fallback House") {
                if(!CustomHouseConfig::parseInteger(customValue, info.customHouse.fallbackHouse)) {
                    numericFieldsValid = false;
                }
            } else if(customKey == "Herald") {
                if(CustomHouseConfig::isSafeAssetPath(customValue)) {
                    info.customHouse.heraldAsset = customValue;
                } else {
                    SDL_Log("ModManager: Ignoring unsafe custom-house herald path in %s", modPath.c_str());
                }
            } else if(customKey == "House Name Voice") {
                if(CustomHouseConfig::isSafeAssetPath(customValue)) {
                    info.customHouse.houseNameVoiceAsset = customValue;
                } else {
                    SDL_Log("ModManager: Ignoring unsafe custom-house voice path in %s", modPath.c_str());
                }
            } else if(customKey == "Voice Playback Rate") {
                double parsedValue = info.customHouse.voicePlaybackRate;
                if(CustomHouseConfig::parseDouble(customValue, parsedValue)
                   && CustomHouseConfig::isValidVoicePlaybackRate(parsedValue)) {
                    info.customHouse.voicePlaybackRate = parsedValue;
                } else {
                    SDL_Log("ModManager: Ignoring invalid custom-house voice playback rate in %s",
                            modPath.c_str());
                }
            } else if(customKey == "Voice Gain") {
                double parsedValue = info.customHouse.voiceGain;
                if(CustomHouseConfig::parseDouble(customValue, parsedValue)
                   && CustomHouseConfig::isValidVoiceGain(parsedValue)) {
                    info.customHouse.voiceGain = parsedValue;
                } else {
                    SDL_Log("ModManager: Ignoring invalid custom-house voice gain in %s", modPath.c_str());
                }
            }
        }

        const bool letterValid = info.customHouse.scenarioLetter >= 'A' && info.customHouse.scenarioLetter <= 'Z';
        const bool prefixValid = info.customHouse.regionPrefix.size() == 3
            && info.customHouse.regionPrefix.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos;
        const bool paletteValid = info.customHouse.paletteIndex >= 0 && info.customHouse.paletteIndex <= 248;
        const bool fallbackValid = info.customHouse.fallbackHouse >= 0
            && info.customHouse.fallbackHouse < NUM_LEGACY_HOUSES;
        info.customHouse.enabled = requestedEnabled && !info.customHouse.displayName.empty()
            && numericFieldsValid && letterValid && prefixValid && paletteValid && fallbackValid;
        if(requestedEnabled && !info.customHouse.enabled) {
            SDL_Log("ModManager: Ignoring invalid generic custom-house registration in %s", modPath.c_str());
        }
    }
    return info;
}

void ModManager::writeModInfo(const std::string& modPath, const ModInfo& info) const {
    std::string iniPath = modPath + "/" + MOD_INI_FILE;

    std::ofstream file(iniPath);
    if (!file.is_open()) {
        SDL_Log("ModManager: Failed to write mod.ini to %s", iniPath.c_str());
        return;
    }

    file << "[Mod]\n";
    file << "Display Name = " << info.displayName << "\n";
    file << "Author = " << info.author << "\n";
    file << "Description = " << info.description << "\n";
    file << "Version = " << info.version << "\n";
    file << "Game Version = " << info.gameVersion << "\n";

    file.close();
}

std::string ModManager::getInstallConfigPath() const {
    // Template files (.default) are in the game's data directory under config/
    return getDuneLegacyDataDir() + "/config";
}
