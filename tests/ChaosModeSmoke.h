#pragma once
#include <GUI/dune/GameOptionsWindow.h>
#include <misc/OMemoryStream.h>
#include <misc/IMemoryStream.h>
#include <FileClasses/LoadSavePNG.h>
#include <structures/BuilderBase.h>
#include <Menu/HouseChoiceInfoMenu.h>

inline void exportAdvancedWindtrapImages(const std::string& output) {
    if(ModManager::instance().getActiveModName() != "Tornie") return;
    for(const auto& item : {std::pair<int, const char*>{UI_MapEditor_AdvancedWindTrapMK2, "AdvancedWindtrap2x3.png"},
                           std::pair<int, const char*>{UI_MapEditor_AdvancedWindTrapMK3, "AdvancedWindtrap3x2.png"}}) {
        auto* image = pGFXManager->getUIGraphicSurface(item.first, HOUSE_ATREIDES);
        if(!image || SavePNG(image, (std::filesystem::path(output) / item.second).string().c_str()) != 0)
            throw std::runtime_error("Advanced windtrap image export failed");
    }
}

inline void verifyFremenConfirmation(const std::string& output, const std::string& stage) {
    if(ModManager::instance().getActiveModName() != "Tornie") return;
    exportAdvancedWindtrapImages(output);
    HouseChoiceInfoMenu menu(HOUSE_FREMEN);
    menu.setText("");
    menu.onMentatTextFinished();
    auto* animation = menu.getPlanetAnimation();
    auto* frame = animation ? animation->getFrame() : nullptr;
    auto* herald = pGFXManager->getUIGraphicSurface(UI_Herald_ColoredLarge, HOUSE_FREMEN);
    if(!frame || !herald) throw std::runtime_error("Fremen confirmation check: missing banner/animation");
    auto expected = sdl2::surface_ptr(SDL_ConvertSurfaceFormat(herald, SDL_PIXELFORMAT_RGB888, 0));
    auto actual = sdl2::surface_ptr(SDL_ConvertSurfaceFormat(frame, SDL_PIXELFORMAT_RGB888, 0));
    int visible = 0;
    for(int y = 0; y < std::min(126, expected->h); ++y) {
        for(int x = 0; x < std::min(expected->w, actual->w - 12); ++x) {
            const auto a = getPixel(expected.get(), x, y) & 0xffffff;
            if(a == 0) continue;
            if(a != (getPixel(actual.get(), x + 12, y + 66) & 0xffffff))
                throw std::runtime_error("Fremen confirmation check: original banner not composited");
            ++visible;
        }
    }
    if(visible < 1000) throw std::runtime_error("Fremen confirmation check: invisible banner");
    SDL_RenderSetClipRect(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    menu.draw();
    auto screen = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(
        0, getRendererWidth(), getRendererHeight(), 32, SDL_PIXELFORMAT_ARGB8888));
    if(!screen || SDL_RenderReadPixels(renderer, nullptr, screen->format->format, screen->pixels, screen->pitch) != 0)
        throw std::runtime_error("Fremen confirmation check: screen capture failed");
    int rendered = 0;
    for(int y = 0; y < std::min(126, expected->h); ++y) for(int x = 0; x < expected->w; ++x) {
        const auto colour = getPixel(expected.get(), x, y) & 0xffffff;
        const int sx = menu.getPosition().x + 256 + 12 + x;
        const int sy = menu.getPosition().y + 96 + 66 + y;
        if(colour && sx >= 0 && sy >= 0 && sx < screen->w && sy < screen->h
           && (getPixel(screen.get(), sx, sy) & 0xffffff) == colour) ++rendered;
    }
    if(rendered < 1000) throw std::runtime_error("Fremen confirmation check: banner absent in actual menu rendering");
    const auto filename = (std::filesystem::path(output) / ("Fremen-confirmation-" + stage + ".png")).string();
    auto file = sdl2::RWops_ptr(SDL_RWFromFile(filename.c_str(), "wb"));
    auto png = sdl2::surface_ptr(SDL_ConvertSurfaceFormat(screen.get(), SDL_PIXELFORMAT_RGBA32, 0));
    if(!file || !png || SavePNG_RW(png.get(), file.get()) != 0) throw std::runtime_error("Fremen confirmation check: PNG export");
    SDL_Log("FREMEN CONFIRMATION PASS: %s, original banner in actual menu (%d pixels)", stage.c_str(), rendered);
}

inline std::string chaosSmokeMap() {
    std::string map = "[BASIC]\nVersion=2\nTechLevel=9\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y = 0; y < 32; ++y) map += fmt::sprintf("%03d=", y) + std::string(32, '%') + "\n";
    return map + "[Player1]\nCredits=1000000\n";
}

inline GameInitSettings chaosSmokeInit(HOUSETYPE house, const SettingsClass::GameOptionsClass& options) {
    GameInitSettings init("chaos-mode-smoke", chaosSmokeMap(), false, options);
    GameInitSettings::HouseInfo info(house, 1);
    info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS));
    init.addHouseInfo(info);
    return init;
}

inline std::string chaosSmokeData(const ObjectData& data) {
    OMemoryStream stream; stream.open(); data.save(stream);
    return std::string(reinterpret_cast<const char*>(stream.getData()), stream.getDataLength());
}

inline void verifyWildspadeTechnology(const std::string& output, const std::string& mod) {
    if(mod != "Tornie" && mod != "Jericho") return;
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Wildspade technology check: " + message);
    };
    auto options = settings.gameOptions; options.chaosMode = false; options.instantBuild = true;
    {
        auto game = std::make_unique<Game>(); currentGame = game.get();
        game->initGame(chaosSmokeInit(getRuntimeHouseForIdentity(HOUSE_WILDSPADE), options));
        auto* house = pLocalHouse;
        auto place = [&](int item, int x, int y) { return house->placeStructure(NONE_ID, item, x, y, true, true); };
        auto* yard = dynamic_cast<BuilderBase*>(place(Structure_ConstructionYard, 1, 1));
        place(Structure_WindTrap, 5, 1); place(Structure_Radar, 9, 1);
        place(Structure_LightFactory, 13, 1); place(Structure_Refinery, 17, 1);
        place(Structure_StarPort, 21, 1);
        auto* factory = dynamic_cast<BuilderBase*>(place(Structure_HighTechFactory, 5, 8));
        require(yard && factory, "production buildings missing");
        game->techLevel = 5; factory->updateBuildList();
        require(!factory->isAvailableToBuild(Unit_ChemicalCarryall), "Chemical below level 6");
        game->techLevel = 6; factory->updateBuildList();
        require(!factory->isAvailableToBuild(Unit_ChemicalCarryall), "Chemical without upgrade");
        require(factory->getMaxUpgradeLevel() == 1, "first Hightech upgrade at level 6");
        require(factory->doUpgrade(), "first upgrade unavailable"); factory->update();
        factory->updateBuildList();
        require(factory->isAvailableToBuild(Unit_ChemicalCarryall), "Chemical needs IX or a second upgrade");
        require(house->getNumItems(Structure_IX) == 0, "unexpected IX");
        require(!factory->isAvailableToBuild(Unit_Ornithopter), "Ornithopter below level 7");
        yard->updateBuildList(); require(!yard->isAvailableToBuild(Structure_IX), "IX before level 7");
        game->techLevel = 7; yard->updateBuildList();
        require(yard->isAvailableToBuild(Structure_IX), "IX absent at level 7");
        place(Structure_IX, 13, 9);
        factory->updateBuildList();
        require(!factory->isAvailableToBuild(Unit_Ornithopter), "Ornithopter after only first upgrade");
        require(factory->doUpgrade(), "second upgrade unavailable at level 7"); factory->update();
        factory->updateBuildList();
        require(factory->isAvailableToBuild(Unit_Ornithopter), "Ornithopter absent after second upgrade");
        GameOptionsWindow window(options); window.draw();
        require(window.getSize().y <= getRendererHeight(), "options dialog exceeds screen");
    }
    currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
    SDL_Log("WILDSPADE TECH PASS: %s, Chemical level 6/first upgrade/no IX; IX and Ornithopter level 7", mod.c_str());
}

inline void verifyChaosMode(const std::string& output, const std::string& mod) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Chaos Mode runtime check: " + message);
    };
    auto options = settings.gameOptions; options.chaosMode = true; options.instantBuild = true;
    GameInitSettings init = chaosSmokeInit(HOUSE_ATREIDES, options);
    OMemoryStream setup; setup.open(); init.save(setup);
    IMemoryStream setupInput(setup.getData(), setup.getDataLength());
    GameInitSettings transmitted(setupInput);
    require(transmitted.isChaosModeEnabled() == (mod != "vanilla"), "multiplayer option transport/Vanilla gate");
    for(const auto& candidate : {GameInitSettings(HOUSE_ATREIDES, options),
            GameInitSettings("chaos-network", chaosSmokeMap(), "chaos-server", false, options)}) {
        OMemoryStream network; network.open(); candidate.save(network);
        IMemoryStream networkInput(network.getData(), network.getDataLength());
        GameInitSettings received(networkInput);
        require(received.getGameType() == candidate.getGameType()
            && received.getRandomSeed() == candidate.getRandomSeed()
            && received.isChaosModeEnabled() == (mod != "vanilla"), "campaign/multiplayer seed and option transport");
    }
    const auto filename = (std::filesystem::path(output) / (mod + "-chaos-mode.sav")).string();
    std::array<std::array<int, ChaosMode::Structures.size()>, NUM_HOUSES> savedDonors{};
    int heavyDonor = HOUSE_ATREIDES;
    Uint32 queuedItem = ItemID_Invalid;
    std::string savedDataHash;
    {
        auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(transmitted);
        require(game->getChaosMode().isEnabled() == (mod != "vanilla"), "activation gate");
        if(mod == "vanilla") {
            game.reset(); currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
            SDL_Log("CHAOS MODE PASS: Vanilla correctly disables Chaos Mode"); return;
        }
        ObjectData base; base.loadFromINIFile(ModManager::instance().getActiveObjectDataPath(), false);
        for(int house = 0; house < NUM_HOUSES; ++house) {
            for(size_t index = 0; index < ChaosMode::Structures.size(); ++index) {
                const int item = ChaosMode::Structures[index];
                const int donor = game->getChaosMode().getTechnologyHouse(house, item);
                savedDonors[house][index] = donor;
                require(base.data[item][donor].techLevel == base.data[item][house].techLevel, "building crossed a technology tier");
            }
            for(int item = 0; item < Num_ItemID; ++item) if(!ChaosMode::includesStructure(item))
                require(game->objectData.data[item][house].price == base.data[item][house].price, "economy or unit catalogue changed");
        }
        auto* house = pLocalHouse;
        const int colour = getHouseVisualHouse(house->getHouseID());
        auto* voice = pSFXManager->getVoice(HouseAtreides, house->getHouseID());
        int position = 0;
        for(int item : {Structure_WindTrap, Structure_Radar, Structure_LightFactory, Structure_Refinery, Structure_StarPort, Structure_IX}) {
            require(house->placeStructure(NONE_ID, item, 1 + position * 5, 1, true, true) != nullptr, "prerequisite building missing");
            ++position;
        }
        auto* heavy = dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID, Structure_HeavyFactory, 5, 15, true, true));
        require(heavy != nullptr, "heavy factory missing");
        heavyDonor = heavy->getTechnologyHouseID();
        require(heavyDonor == savedDonors[house->getHouseID()][3], "factory donor differs from setup");
        while(heavy->isAllowedToUpgrade()) { require(heavy->doUpgrade(), "borrowed factory upgrade"); heavy->update(); }
        heavy->updateBuildList();
        for(const auto& item : heavy->getBuildList()) if(isUnit(item.itemID) && !isInfantryUnit(item.itemID)) { queuedItem = item.itemID; break; }
        require(queuedItem != ItemID_Invalid, "borrowed production list empty");
        heavy->doProduceItem(queuedItem);
        require(heavy->getCurrentProducedItem() == queuedItem, "borrowed order not queued");
        auto* unit = house->createUnit(queuedItem, false, heavyDonor);
        require(unit && unit->getOwner() == house && unit->getProductionHouseID() == heavyDonor, "unit ownership/technology inheritance");
        unit->deploy(Coord(25, 20));
        require(getHouseVisualHouse(house->getHouseID()) == colour && pSFXManager->getVoice(HouseAtreides, house->getHouseID()) == voice, "faction colour/voice changed");
        auto* palace = dynamic_cast<Palace*>(house->placeStructure(NONE_ID, Structure_Palace, 15, 15, true, true));
        require(palace && palace->getTechnologyHouseID() == game->getChaosMode().getTechnologyHouse(house->getHouseID(), Structure_Palace), "palace ignores donor");
        require(palace->usesJerichoOrnithopterStrike() == isHouseFaction(static_cast<HOUSETYPE>(palace->getTechnologyHouseID()), HOUSE_WILDSPADE), "palace strike ignores donor identity");
        savedDataHash = chaosSmokeData(game->objectData);
        require(game->saveGame(filename), "save failed");
    }
    currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
    {
        auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(GameInitSettings(filename));
        require(game->getChaosMode().isEnabled(), "loaded option disabled");
        require(chaosSmokeData(game->objectData) == savedDataHash, "saved data changed");
        for(int house = 0; house < NUM_HOUSES; ++house) for(size_t index = 0; index < ChaosMode::Structures.size(); ++index)
            require(game->getChaosMode().getTechnologyHouse(house, ChaosMode::Structures[index]) == savedDonors[house][index], "draw changed on load");
        BuilderBase* heavy = nullptr;
        for(auto* structure : structureList) if(structure->getItemID() == Structure_HeavyFactory) heavy = static_cast<BuilderBase*>(structure);
        require(heavy && heavy->getTechnologyHouseID() == heavyDonor && heavy->getCurrentProducedItem() == queuedItem, "saved borrowed queue lost");
        const int before = pLocalHouse->getNumItems(queuedItem);
        for(int tick = 0; tick < 100 && pLocalHouse->getNumItems(queuedItem) == before; ++tick) heavy->update();
        require(pLocalHouse->getNumItems(queuedItem) == before + 1, "saved production did not deploy one new unit");
        bool finished = true;
        for(auto* unit : unitList) if(unit->getItemID() == queuedItem && unit->getOwner() == pLocalHouse)
            finished = finished && unit->getProductionHouseID() == heavyDonor;
        require(finished, "saved borrowed unit production failed");
    }
    currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
    SDL_Log("CHAOS MODE PASS: %s, same-level donors, transmitted option, faction identity, units, palace and saved queue", mod.c_str());
}
