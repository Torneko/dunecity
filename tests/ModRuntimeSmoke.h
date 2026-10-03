#pragma once

// Opt-in integration check using the real resource managers and game objects.
// Run a test-enabled build with --verify-mods in an isolated SDL/profile environment.
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <units/UnitBase.h>
#include <structures/StructureBase.h>
#include <players/PlayerFactory.h>
#include <sand.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>

inline void runModRuntimeSmoke() {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Mod runtime check: " + message);
    };
    const char* output = std::getenv("DUNELEGACY_SMOKE_DIR");
    require(output != nullptr && std::filesystem::is_directory(output),
            "set DUNELEGACY_SMOKE_DIR to an existing temporary directory");
    require(PlayerFactory::getByPlayerClass(HUMANPLAYERCLASS) != nullptr, "human player factory unavailable");
    auto& mods = ModManager::instance();
    const std::string previousMod = mods.getActiveModName();
    int scenarios = 0;
    for(const std::string mod : {"Tornie", "TornieLite", "Jericho", "vanilla", "Jericho", "TornieLite"}) {
        require(mods.modExists(mod), "missing selectable mod " + mod);
        require(mods.setActiveMod(mod), "cannot activate " + mod);
        effectiveGameOptions = mods.loadEffectiveGameOptions(settings.gameOptions);
        require(pSFXManager != nullptr, "audio manager unavailable");
        require(mods.isCustomHouseRegistered() == (mod == "Jericho" || mod == "Tornie"),
                "custom-house registration leaked across a mod switch");
        require(getNumAvailableHouses() == ((mod == "Tornie" || mod == "Jericho") ? 9 : 8),
                "registered house slots leaked across a mod switch");
        if(mod == "vanilla") {
            require(!pFileManager->exists("HeraldWildspade.png"), "Jericho resource leaked into vanilla");
            continue;
        }
        // Load the opening and final scenarios of every bundled campaign through
        // the actual INI loader, object factory and active-mod search path.
        const int previousScenarios = scenarios;
        const int campaignHouseCount = mod == "TornieLite" ? 6 : NUM_CAMPAIGN_HOUSES;
        for(int selectedHouse = 0; selectedHouse < campaignHouseCount; ++selectedHouse) {
          if(!isHouseAvailable(static_cast<HOUSETYPE>(selectedHouse))) continue;
          for(int mission : {1, 22}) {
            const GameInitSettings campaign(static_cast<HOUSETYPE>(selectedHouse), mission, effectiveGameOptions);
            const std::string name = campaign.getFilename();
            auto resolvedFile = pFileManager->openFile(name);
            const Sint64 size = SDL_RWsize(resolvedFile.get());
            require(size > 0, "empty campaign resource " + name);
            std::string data(static_cast<size_t>(size), '\0');
            require(SDL_RWread(resolvedFile.get(), data.data(), 1, data.size()) == data.size(), "truncated campaign resource " + name);
            std::string physicalName = name;
            convertToLower(physicalName);
            const auto entryPath = std::filesystem::path(mods.getModPath(mod)) / "campaign" / physicalName;
            INIFile scenario(entryPath.string());
            GameInitSettings init(name, data, false, effectiveGameOptions);
            bool humanAssigned = false;
            for(int h = 0; h < NUM_CAMPAIGN_HOUSES; ++h) {
                const auto house = static_cast<HOUSETYPE>(h);
                const std::string section = getHouseNameByNumber(house);
                if(!isHouseAvailable(house) || !scenario.hasSection(section)) continue;
                GameInitSettings::HouseInfo info(house, h + 1);
                const bool human = scenario.getStringValue(section, "Brain", "CPU") == "Human";
                info.addPlayerInfo(GameInitSettings::PlayerInfo(
                    human ? settings.general.playerName : "Smoke AI " + std::to_string(h),
                    human ? HUMANPLAYERCLASS : DEFAULTAIPLAYERCLASS));
                init.addHouseInfo(info);
                humanAssigned = humanAssigned || human;
            }
            require(humanAssigned, mod + " scenario has no recognized human house: " + name);
            {
                std::unique_ptr<Game> game = std::make_unique<Game>();
                currentGame = game.get();
                game->initGame(init);
                require(currentGameMap != nullptr && pLocalHouse != nullptr,
                        mod + " scenario did not initialize: " + name);
                require(!structureList.empty() || !unitList.empty(), "empty scenario " + name);
                game->processObjects();
                ++scenarios;
            }
            currentGame = nullptr;
            pLocalHouse = nullptr;
            pLocalPlayer = nullptr;
        }
        }
        require(scenarios - previousScenarios == (mod == "TornieLite" ? 12 : 18),
                mod + " opening/final campaign scenario coverage is incomplete");
        // Exercise the added factory classes and stable IDs through save/load.
        std::string terrain = "[BASIC]\nVersion=2\nTechLevel=9\n[MAP]\nSizeX=32\nSizeY=32\n";
        for(int y = 0; y < 32; ++y) terrain += fmt::sprintf("%03d=", y) + std::string(32, '%') + "\n";
        terrain += "[Player1]\nCredits=100000\n[Player2]\nCredits=100000\n";
        GameInitSettings init("runtime-smoke", terrain, false, effectiveGameOptions);
        GameInitSettings::HouseInfo info(HOUSE_ATREIDES, 1);
        info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS));
        init.addHouseInfo(info);
        const std::string save = (std::filesystem::path(output) / (mod + "-runtime.sav")).string();
        {
            auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
            House* house = game->getHouse(HOUSE_ATREIDES);
            require(house != nullptr, "fixture house missing");
            int x = 2;
            for(int item : {Unit_ChemicalSiegeTank, Unit_ChemicalCarryall, Unit_RebelHarvester}) {
                auto unit = house->createUnit(item);
                require(unit != nullptr && unit->getItemID() == static_cast<unsigned>(item), "unit factory/ID mismatch");
                unit->deploy(Coord(x++, 22));
            }
            x = 2;
            for(int item : {Structure_LoveFactory, Structure_ChaosFactory, Structure_Flamepost, Structure_Chemipost}) {
                auto structure = house->placeStructure(NONE_ID, item, x, 2, true, true);
                require(structure != nullptr && structure->getItemID() == static_cast<unsigned>(item), "structure factory/ID mismatch");
                x += 6;
            }
            game->processObjects();
            require(game->saveGame(save), "cannot save new mod objects");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        {
            auto game = std::make_unique<Game>(); currentGame = game.get();
            require(game->loadSaveGame(save), "cannot reload new mod objects");
            require(game->getHouse(HOUSE_ATREIDES)->getNumItems(Unit_ChemicalCarryall) == 1, "Carryall disappeared in save/load");
            require(game->getHouse(HOUSE_ATREIDES)->getNumItems(Structure_LoveFactory) == 1, "Love Factory disappeared in save/load");
            require(game->getHouse(HOUSE_ATREIDES)->getNumItems(Structure_ChaosFactory) == 1, "Chaos Factory disappeared in save/load");
            game->processObjects();
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        SDL_Log("MOD SMOKE PASS: %s activation, campaign loading, new objects, save/load", mod.c_str());
    }
    require(mods.setActiveMod(previousMod), "cannot restore previous mod");
    SDL_Log("MOD SMOKE COMPLETE: %d opening/final campaign scenarios loaded", scenarios);
}
