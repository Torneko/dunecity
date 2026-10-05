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
#include "EditorHouseColorSmoke.h"
#include "SpiceAndMapLoadingSmoke.h"
#include <Menu/MapChoice.h>
#include <Menu/HouseChoiceInfoMenu.h>
#include <MapEditor/PlayerSettingsWindow.h>
#include <ObjectData.h>

inline void verifyChaosFactoryGraphics(const std::string& output, const std::string& mod,
                                      const std::string& stage) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Mod runtime check: Chaos Factory " + message);
    };
    constexpr int width = 3 * D2_TILESIZE, height = 2 * D2_TILESIZE;
    for(int house = 0; house < NUM_HOUSES; ++house) {
        auto* preview = pGFXManager->getUIGraphicSurface(UI_MapEditor_ChaosFactory, house);
        require(preview && preview->w == width && preview->h == height,
                mod + " " + stage + " editor preview is using a fallback");
        for(unsigned zoom = 0; zoom < NUM_ZOOMLEVEL; ++zoom) {
            auto* texture = pGFXManager->getZoomedObjPic(ObjPic_ChaosFactory, house, zoom);
            int w = 0, h = 0;
            require(texture && SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) == 0
                    && w == 4 * width * static_cast<int>(zoom + 1)
                    && h == height * static_cast<int>(zoom + 1),
                    mod + " " + stage + " atlas dimensions/zoom do not match custom art");
        }
    }
    // Compare actual rendered pixels, so a non-null but stale preview cannot pass.
    auto* atlas = pGFXManager->getZoomedObjPic(ObjPic_ChaosFactory, HOUSE_HARKONNEN, 0);
    auto* preview = pGFXManager->getUIGraphic(UI_MapEditor_ChaosFactory, HOUSE_HARKONNEN);
    SDL_RenderSetClipRect(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Rect source{2 * width, 0, width, height};
    SDL_Rect left{0, 0, width, height}, right{width, 0, width, height};
    require(SDL_RenderCopy(renderer, atlas, &source, &left) == 0
            && SDL_RenderCopy(renderer, preview, nullptr, &right) == 0, "rendering");
    auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(
        0, 2 * width, height, 32, SDL_PIXELFORMAT_ARGB8888));
    SDL_Rect area{0, 0, 2 * width, height};
    require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format,
                                         pixels->pixels, pixels->pitch) == 0, "pixel capture");
    bool visible = false;
    for(int y = 0; y < height; ++y) {
        auto* row = reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(pixels->pixels) + y * pixels->pitch);
        for(int x = 0; x < width; ++x) {
            visible = visible || (row[x] & 0xffffff) != 0;
            require((row[x] & 0xffffff) == (row[x + width] & 0xffffff),
                    mod + " " + stage + " editor preview differs from the dedicated sprite");
        }
    }
    require(visible, mod + " " + stage + " sprite is invisible");
    require(SDL_SaveBMP(pixels.get(), (std::filesystem::path(output)
            / (mod + "-chaos-" + stage + ".bmp")).string().c_str()) == 0, "preview output");
    SDL_Log("CHAOS GRAPHICS PASS: %s %s, all faction slots/zooms and exact editor pixels",
            mod.c_str(), stage.c_str());
}

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
    verifyEditorHouseColors(output, previousMod, "startup");
    if(mods.isTornieContentActive()) verifyChaosFactoryGraphics(output, previousMod, "startup");
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
        verifyEditorHouseColors(output, mod, "switch");
        verifyEditorHouseColorRoundtrip(output, mod);
        verifyEditorMapLoading(output, mod);
        verifyGeneratedSpice(output, mod);
        if(mod=="Tornie") {
            auto* herald=pGFXManager->getUIGraphicSurface(UI_Herald_Colored,HOUSE_FREMEN);
            require(herald && herald->w==84 && herald->h==91,"attached Fremen banner dimensions");
            require(SDL_SaveBMP(herald,(std::filesystem::path(output)/"tornie-fremen-banner.bmp").string().c_str())==0,"Fremen banner output");
        }
        if(mod=="Jericho") {
            auto* herald=pGFXManager->getUIGraphicSurface(UI_Herald_Colored,getRuntimeHouseForIdentity(HOUSE_CUSTOM));
            require(herald!=nullptr,"yellow Corruptique herald missing");
            require(SDL_SaveBMP(herald,(std::filesystem::path(output)/"jericho-corruptique-banner.bmp").string().c_str())==0,"Corruptique banner output");
        }
        // Every actual campaign region file is parsed by the game UI, including
        // all eight progress stages and Jericho's identity-to-runtime mapping.
        for(int h=0;h<NUM_HOUSES;++h) {
            const auto house=static_cast<HOUSETYPE>(h);
            if(!isCampaignHouseAvailable(house))continue;
            const auto name=getHouseNameByNumber(house);
            require(pTextManager->getBriefingText(0,MISSION_DESCRIPTION,h).find(name)!=std::string::npos,
                    "faction description uses another house name: "+name);
            HouseChoiceInfoMenu confirmation(h);
            for(unsigned mission:{1U,4U,7U,10U,13U,16U,19U,21U}) {
                MapChoice choice(house,mission,0);choice.drawSpecificStuff();
            }
        }
        {
            MapEditor editor;editor.setMap(MapData(64,32,Terrain_Rock),MapInfo());
            PlayerSettingsWindow window(&editor,HOUSE_HARKONNEN);window.draw();
            require(window.getSize().y<=getRendererHeight(),"player settings viewport exceeds screen");
        }
        if(mod == "vanilla") {
            require(!pFileManager->exists("HeraldWildspade.png"), "Jericho resource leaked into vanilla");
            require(isCampaignHouseAvailable(HOUSE_KLESHMERSH),"vanilla Kleshmersh unavailable");
            // Version 1.0.530 reads this legacy slot from Custom_IBM.PAL when
            // available, otherwise IBM.PAL. Keep that optional-palette behavior.
            require(getHouseColorPaletteIndexFromSlot(HOUSECOLOR_CUSTOM_BRIGHT_YELLOW)==PALCOLOR_FREMEN,
                    "older vanilla custom color moved to another palette ramp");
            const auto& legacyPalette=customPaletteLoaded ? customPalette : palette;
            for(int shade=0;shade<8;++shade) {
                const auto original=legacyPalette[PALCOLOR_FREMEN+shade];
                const auto actual=getHouseColorSDL(HOUSECOLOR_CUSTOM_BRIGHT_YELLOW,shade);
                require(actual.r==original.r && actual.g==original.g && actual.b==original.b,
                        "older vanilla custom color changed meaning");
            }
            SDL_Log("VANILLA LEGACY COLOR PASS: unchanged slot/ramp, optional palette=%d",customPaletteLoaded);
            auto* herald=pGFXManager->getUIGraphicSurface(UI_Herald_Colored,HOUSE_KLESHMERSH);
            require(herald && herald->w==84 && herald->h==91,"brown attached banner sizing");
            require(SDL_SaveBMP(herald,(std::filesystem::path(output)/"vanilla-kleshmersh-banner.bmp").string().c_str())==0,"brown banner output");
            for(int mission=1;mission<=22;++mission) {
                const auto name=GameInitSettings(HOUSE_KLESHMERSH,mission,effectiveGameOptions).getFilename();
                auto file=pFileManager->openCampaignFile(name);
                std::string content(static_cast<size_t>(SDL_RWsize(file.get())),'\0');
                require(SDL_RWread(file.get(),content.data(),1,content.size())==content.size(),"Kleshmersh scenario read");
                // Load the actual campaign map as a custom fixture so no
                // interactive briefing waits for clicks during automation.
                GameInitSettings init(name,content,false,effectiveGameOptions);
                const auto opponent=mission<=10 ? HOUSE_HARKONNEN : mission<=21 ? HOUSE_SARDAUKAR : HOUSE_REBELS;
                for(auto house:{HOUSE_KLESHMERSH,opponent}) {
                    const bool human=house==HOUSE_KLESHMERSH;
                    GameInitSettings::HouseInfo info(house,human ? 1 : 2);
                    info.addPlayerInfo(GameInitSettings::PlayerInfo(human ? settings.general.playerName : "Kleshmersh Smoke AI",
                        human ? HUMANPLAYERCLASS : DEFAULTAIPLAYERCLASS));init.addHouseInfo(info);
                }
                auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
                require(pLocalHouse && pLocalHouse->getHouseID()==HOUSE_KLESHMERSH,"vanilla Kleshmersh campaign owner");
                require(game->getHouse(opponent)!=nullptr,"vanilla Kleshmersh campaign opponent");
                for(int item=0;item<Num_ItemID;++item) {
                    const auto& k=currentGame->objectData.data[item][HOUSE_KLESHMERSH];
                    const auto& n=currentGame->objectData.data[item][HOUSE_NEUTRAL];
                    require(k.enabled==n.enabled && k.techLevel==n.techLevel && k.upgradeLevel==n.upgradeLevel
                        && k.builder==n.builder && k.prerequisiteStructuresSet==n.prerequisiteStructuresSet
                        && k.hitpoints==n.hitpoints && k.price==n.price,"vanilla Kleshmersh tech mismatch");
                }
                game->processObjects();++scenarios;
                game.reset();currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
            }
            SDL_Log("VANILLA KLESHMERSH PASS: 22 Neutral map clones, opponents, brown banner and matching tech");
            continue;
        }
        verifyChaosFactoryGraphics(output, mod, "switch");
        // Load the opening and final scenarios of every bundled campaign through
        // the actual INI loader, object factory and active-mod search path.
        const int previousScenarios = scenarios;
        const int campaignHouseCount = NUM_HOUSES;
        for(int selectedHouse = 0; selectedHouse < campaignHouseCount; ++selectedHouse) {
          if(!isCampaignHouseAvailable(static_cast<HOUSETYPE>(selectedHouse))) continue;
          for(int mission : {1, 22}) {
            const GameInitSettings campaign(static_cast<HOUSETYPE>(selectedHouse), mission, effectiveGameOptions);
            const std::string name = campaign.getFilename();
            auto resolvedFile = pFileManager->openCampaignFile(name);
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
            for(int h = 0; h < NUM_HOUSES; ++h) {
                const auto house = static_cast<HOUSETYPE>(h);
                const std::string section = getHouseNameByNumber(house);
                if(!isCampaignHouseAvailable(house) || !scenario.hasSection(section)) continue;
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
        require(scenarios - previousScenarios == (mod == "TornieLite" ? 12 : 24),
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
            verifyChaosFactoryGraphics(output, mod, "game");
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
            verifyChaosFactoryGraphics(output, mod, "save-load");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        SDL_Log("MOD SMOKE PASS: %s activation, campaign loading, new objects, save/load", mod.c_str());
    }
    require(mods.setActiveMod(previousMod), "cannot restore previous mod");
    SDL_Log("MOD SMOKE COMPLETE: %d opening/final campaign scenarios loaded", scenarios);
}
