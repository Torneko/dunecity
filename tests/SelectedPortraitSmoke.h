#pragma once

#include <Game.h>
#include <House.h>
#include <GUI/ObjectInterfaces/ObjectInterface.h>
#include <GUI/dune/BuilderList.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/FileManager.h>
#include <filesystem>

// Exercise the selected-unit UI with real resources, including custom colours.
// Run a test-enabled build with --verify-portraits and DUNELEGACY_SMOKE_DIR set.
inline void runSelectedPortraitSmoke() {
    const auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Selected portraits: " + message);
    };
    const char* directory = std::getenv("DUNELEGACY_SMOKE_DIR");
    require(directory != nullptr, "missing output directory");
    const std::filesystem::path output(directory);
    const auto previous = ModManager::instance().getActiveModName();
    const std::pair<int, unsigned> portraits[] = {
        {Unit_Carryall, Picture_Carryall}, {Unit_Devastator, Picture_Devastator},
        {Unit_Deviator, Picture_Deviator}, {Unit_Harvester, Picture_Harvester},
        {Unit_RebelHarvester, Picture_Harvestank}, {Unit_Launcher, Picture_Launcher},
        {Unit_MCV, Picture_MCV}, {Unit_Ornithopter, Picture_Ornithopter},
        {Unit_Quad, Picture_Quad}, {Unit_RaiderTrike, Picture_RaiderTrike},
        {Unit_SiegeTank, Picture_SiegeTank}, {Unit_SonicTank, Picture_SonicTank},
        {Unit_Tank, Picture_Tank}, {Unit_Trike, Picture_Trike},
        {Unit_RocketTrike, Picture_RocketTrike}, {Unit_SonicTrike, Picture_SonicTrike},
        {Unit_FlameTank, Picture_FlameTank}, {Unit_EliteLauncher, Picture_EliteLauncher},
        {Unit_EliteSiegeTank, Picture_EliteSiegeTank}, {Unit_ChemicalSiegeTank, Picture_ChemicalSiegeTank},
        {Unit_ChemicalCarryall, Picture_ChemicalCarryall}, {Structure_Flamepost, Picture_Flamepost},
        {Unit_Soldier, Picture_Soldier}, {Unit_Infantry, Picture_Soldier},
        {Unit_Infantry5, Picture_Soldier}, {Unit_Trooper, Picture_Trooper},
        {Unit_Troopers, Picture_Trooper}, {Unit_Troopers5, Picture_Trooper}
    };
    const auto capture = [&](SDL_Texture* texture) {
        require(texture != nullptr, "missing portrait");
        int width = 0, height = 0;
        require(SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) == 0
            && width == 91 && height == 55, "portrait dimensions");
        SDL_RenderSetClipRect(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 3, 7, 11, 255);
        SDL_RenderClear(renderer);
        SDL_Rect area{0, 0, width, height};
        require(SDL_RenderCopy(renderer, texture, nullptr, &area) == 0, "portrait draw");
        auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888));
        require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format,
            pixels->pixels, pixels->pitch) == 0, "portrait capture");
        return pixels;
    };
    const auto samePixels = [](SDL_Surface* first, SDL_Surface* second) {
        for(int y = 0; y < 55; ++y)
            if(std::memcmp(static_cast<const Uint8*>(first->pixels) + y * first->pitch,
                           static_cast<const Uint8*>(second->pixels) + y * second->pitch, 91 * 4) != 0)
                return false;
        return true;
    };
    for(const std::string mod : {"vanilla", "Tornie", "Jericho", "TornieLite", "JerichoLite", "vanilla"}) {
        require(ModManager::instance().setActiveMod(mod), "mod activation");
        std::string terrain = "[BASIC]\nVersion=2\nTechLevel=9\n[MAP]\nSizeX=32\nSizeY=32\n";
        for(int y = 0; y < 32; ++y) terrain += fmt::sprintf("%03d=", y) + std::string(32, '%') + "\n";
        terrain += "[Player1]\nCredits=100000\n";
        for(int colour = 0; colour < NUM_HOUSE_COLOR_SLOTS; ++colour) {
            GameInitSettings init("portrait-colours", terrain, false, effectiveGameOptions);
            GameInitSettings::HouseInfo local(HOUSE_ATREIDES, 1);
            local.colorOfHouse = colour;
            local.addPlayerInfo({settings.general.playerName, HUMANPLAYERCLASS});
            init.addHouseInfo(local);
            {
                auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
                require(getHouseVisualHouse(HOUSE_ATREIDES) == colour, "colour fixture");
                for(const auto& portrait : portraits) {
                    auto expected = capture(pGFXManager->getSmallDetailPic(portrait.second));
                    auto actual = capture(resolveItemPicture(portrait.first, HOUSE_ATREIDES));
                    require(samePixels(expected.get(), actual.get()), mod + " colour "
                        + std::to_string(colour) + " item " + std::to_string(portrait.first)
                        + " replaced the painted portrait with an editor thumbnail");
                }
                if(colour == HOUSECOLOR_GUEST_1) {
                    auto* unit = pLocalHouse->createUnit(Unit_SonicTank, true);
                    require(unit != nullptr, "selected Sonic Tank fixture");
                    std::unique_ptr<ObjectInterface> panel(unit->getInterfaceContainer());
                    require(panel != nullptr, "selected-unit interface");
                    SDL_SetRenderDrawColor(renderer, 176, 126, 16, 255); SDL_RenderClear(renderer);
                    panel->draw(Point(20, 10));
                    auto scene = renderReadSurface(renderer);
                    require(scene && SavePNG(scene.get(), (output / (mod + "-selected-sonic-tank.png")).string().c_str()) == 0,
                        "selected-unit UI screenshot");
                    for(const int building : {Structure_Barracks, Structure_WOR, Structure_Worfinery}) {
                        if(mod == "vanilla" && building == Structure_Worfinery) continue;
                        auto* builder = pLocalHouse->placeStructure(NONE_ID, building, 2, 2, true, true);
                        require(builder != nullptr, "production fixture");
                        StaticContainer productionPanel;
                        auto* list = BuilderList::create(builder->getObjectID());
                        productionPanel.addWidget(list, Point(0, 0), list->getMinimumSize());
                        const std::pair<int, const char*> icons[] = {
                            {Unit_Soldier, "ProductionSoldier.png"}, {Unit_Infantry, "ProductionInfantry3.png"},
                            {Unit_Infantry5, "ProductionInfantry5.png"}, {Unit_Trooper, "ProductionTrooper.png"},
                            {Unit_Troopers, "ProductionTroopers3.png"}, {Unit_Troopers5, "ProductionTroopers5.png"}
                        };
                        for(const auto& icon : icons) {
                            auto* production = list->getProductionPortrait(icon.first);
                            require((production != nullptr) == (mod != "vanilla"), "production-only variants, Vanilla excluded");
                            if(production) {
                                auto actual = capture(production);
                                auto png = LoadPNG_RW(pFileManager->openFile(icon.second).get());
                                require(png != nullptr, "production source PNG");
                                auto reference = convertSurfaceToTexture(std::move(png));
                                auto expected = capture(reference.get());
                                require(samePixels(actual.get(), expected.get()), "production variant differs from supplied PNG");
                                require(production != resolveItemPicture(icon.first, HOUSE_ATREIDES), "production replaces selected-unit resource");
                            }
                        }
                    }
                }
            }
            currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        }
        SDL_Log("SELECTED PORTRAITS PASS: %s, 21 colour slots, 28 exact painted portraits, selected Sonic Tank UI, separate infantry production icons", mod.c_str());
    }
    require(ModManager::instance().setActiveMod(previous), "restore active mod");
}
