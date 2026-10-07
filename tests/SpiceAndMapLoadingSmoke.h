#pragma once

#include <MapEditor/LoadMapWindow.h>
#include <MapEditor/NewMapWindow.h>
#include <misc/SpiceGeneration.h>
#include <array>

inline void verifyEditorSpiceDefaults(const std::string& output, const std::string& mod) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Editor spice defaults: " + message);
    };
    const bool jericho = mod == "Jericho" || mod == "JerichoLite";
    const bool tornie = mod == "Tornie" || mod == "TornieLite";
    const auto& mods = ModManager::instance();
    require(mods.getActiveContentOptions().spiceMask() == (mod == "vanilla" ? 0u : 15u)
            && effectiveGameOptions.content == mods.getActiveContentOptions(),
            mod + " installed defaults or effective options dropped a spice family");
    if(mod == "JerichoLite") {
        const auto jerichoDefaults = mods.getModInfo("Jericho").content;
        require(mods.getActiveContentOptions().spiceMask() == jerichoDefaults.spiceMask(),
                "Jericho Lite defaults differ from Jericho");
    }
    MapEditor editor;
    editor.setMap(MapData(32, 32, Terrain_Rock), MapInfo());
    MapEditorInterface ui(&editor);
    ui.onModeButton(1);
    require(ui.editorModeTerrain_HBox4.isVisible() == tornie
            && ui.editorModeTerrain_HBox6.isVisible() == tornie
            && ui.editorModeTerrain_HBox5.isVisible() == jericho
            && ui.editorModeTerrain_HBox7.isVisible() == jericho,
            mod + " custom spice palette rows are hidden or belong to another mod");
    if(jericho) {
        require(ui.editorModeTerrain_RedSpice.isVisible() && ui.editorModeTerrain_RedSpice.isEnabled()
                && ui.editorModeTerrain_WhiteSpice.isVisible() && ui.editorModeTerrain_WhiteSpice.isEnabled(),
                "Jericho red/blue painting tools are unavailable");
        ui.editorModeTerrain_WhiteSpice.handleMouseLeft(ui.editorModeTerrain_WhiteSpice.getSize().x / 2,
            ui.editorModeTerrain_WhiteSpice.getSize().y / 2, true);
        ui.editorModeTerrain_WhiteSpice.handleMouseLeft(ui.editorModeTerrain_WhiteSpice.getSize().x / 2,
            ui.editorModeTerrain_WhiteSpice.getSize().y / 2, false);
        require(ui.currentTerrainType == Terrain_WhiteSpice, "blue spice painting tool cannot be selected");
    }
    SDL_RenderSetClipRect(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); SDL_RenderClear(renderer);
    ui.draw(Point(0, 0));
    SDL_Rect area{getRendererWidth() - SIDEBARWIDTH, 0, SIDEBARWIDTH, getRendererHeight()};
    auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, area.w, area.h, 32, SDL_PIXELFORMAT_ARGB8888));
    require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format,
            pixels->pixels, pixels->pitch) == 0, "spice palette capture failed");
    require(SDL_SaveBMP(pixels.get(), (std::filesystem::path(output) / (mod + "-editor-spices.bmp")).string().c_str()) == 0,
            "spice palette preview failed");

    NewMapWindow generator(HOUSE_ATREIDES);
    generator.mapSizeXDropDownBox.setSelectedItem(0);
    generator.mapSizeYDropDownBox.setSelectedItem(0);
    generator.emptyMapRadioButton.setChecked(false);
    generator.randomMapRadioButton.setChecked(true);
    generator.seedMapRadioButton.setChecked(false);
    generator.rngSeedTextBox.setValue(1234567);
    generator.greenSpiceCheckbox.setChecked(true);
    generator.redSpiceCheckbox.setChecked(true);
    generator.greenSpiceDigitsTextBox.setValue(3);
    generator.redSpiceDigitsTextBox.setValue(3);
    generator.onMapTypeChanged(1);
    int first = 0, second = 0, unexpected = 0;
    const int firstTerrain = jericho ? Terrain_RedSpice : Terrain_GreenSpice;
    const int secondTerrain = jericho ? Terrain_WhiteSpice : Terrain_PaleLilacSpice;
    for(int y = 0; y < generator.mapdata.getSizeY(); ++y)
        for(int x = 0; x < generator.mapdata.getSizeX(); ++x) {
            int type = generator.mapdata(x, y);
            if(type == Terrain_ThickGreenSpice || type == Terrain_GreenSpiceBloom) type = Terrain_GreenSpice;
            else if(type == Terrain_ThickRedSpice || type == Terrain_RedSpiceBloom) type = Terrain_RedSpice;
            else if(type == Terrain_ThickPaleLilacSpice || type == Terrain_PaleLilacSpiceBloom) type = Terrain_PaleLilacSpice;
            else if(type == Terrain_ThickWhiteSpice || type == Terrain_WhiteSpiceBloom) type = Terrain_WhiteSpice;
            if(type == firstTerrain) ++first;
            else if(type == secondTerrain) ++second;
            else if(type == Terrain_GreenSpice || type == Terrain_RedSpice
                    || type == Terrain_PaleLilacSpice || type == Terrain_WhiteSpice) ++unexpected;
        }
    require(unexpected == 0 && (mod == "vanilla" ? first == 0 && second == 0 : first > 0 && second > 0),
            mod + " random map custom fields use the wrong spice palette");
    SDL_Log("EDITOR SPICE DEFAULTS PASS: %s effective mask, visible/selectable palette, generated custom fields and sidebar pixels", mod.c_str());
}

inline void verifyEditorMapLoading(const std::string& output, const std::string& mod) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Editor map loading check: " + message);
    };
    SDL_KeyboardEvent enter{}; enter.type = SDL_KEYDOWN; enter.keysym.sym = SDLK_RETURN;
    SDL_KeyboardEvent remove{}; remove.type = SDL_KEYDOWN; remove.keysym.sym = SDLK_DELETE;
    const std::string fixture = "[BASIC]\nVersion=2\n[MAP]\nSizeX=2\nSizeY=2\n000=%%\n001=%%\n[Player1]\nCredits=2000\n";
    for(const char* relative : {"maps/singleplayer/", "maps/multiplayer/"}) {
        char path[FILENAME_MAX];
        require(fnkdat(relative, path, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT) >= 0, "user map directory");
        require(writeCompleteFile((std::filesystem::path(path) / "EditorSmoke.INI").string(), fixture), "user map fixture");
    }
    const std::array<LoadMapWindow::MapSource, 5> sources{
        LoadMapWindow::MapSource::Singleplayer, LoadMapWindow::MapSource::Multiplayer,
        LoadMapWindow::MapSource::UserSingleplayer, LoadMapWindow::MapSource::UserMultiplayer,
        LoadMapWindow::MapSource::Campaign};
    for(const auto source : sources) {
        if(source == LoadMapWindow::MapSource::Campaign && mod == "vanilla") continue;
        LoadMapWindow chooser(getHouseInterfaceColor(HOUSE_HARKONNEN), source);
        chooser.handleKeyPress(enter);
        const auto selected = chooser.getLoadMapFilepath();
        require(!selected.empty() && std::filesystem::is_regular_file(selected),
                mod + " source " + std::to_string(static_cast<int>(source)) + " has no selectable map");
        require(chooser.isLoadMapSingleplayer() == (source != LoadMapWindow::MapSource::Multiplayer
                && source != LoadMapWindow::MapSource::UserMultiplayer), "SP/MP classification");
        if(source == LoadMapWindow::MapSource::UserSingleplayer || source == LoadMapWindow::MapSource::UserMultiplayer)
            require(std::filesystem::path(selected).filename() == "EditorSmoke.INI", "uppercase user filename lost");
        else {
            const auto before = readCompleteFile(selected);
            chooser.handleKeyPress(remove);
            require(readCompleteFile(selected) == before && !chooser.hasChildWindow(),
                    "bundled maps must not be deleted by the load dialog");
        }
        {
            MapEditor editor;
            editor.loadMap(selected);
            require(editor.getLastSaveName() == selected, "selected path did not reach editor");
            require(!editor.getPlayers().empty(), "empty map after loading");
        }
        if(source == LoadMapWindow::MapSource::Singleplayer || source == LoadMapWindow::MapSource::Campaign) {
            SDL_RenderSetClipRect(renderer, nullptr);
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); SDL_RenderClear(renderer);
            chooser.draw(); chooser.drawOverlay();
            const int width = getRendererWidth(), height = getRendererHeight();
            auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888));
            require(pixels && SDL_RenderReadPixels(renderer, nullptr, pixels->format->format, pixels->pixels, pixels->pitch) == 0,
                    "load dialog capture");
            require(SDL_SaveBMP(pixels.get(), (std::filesystem::path(output) /
                    (mod + "-map-dialog-" + std::to_string(static_cast<int>(source)) + ".bmp")).string().c_str()) == 0,
                    "load dialog output");
        }
    }
    SDL_Log("EDITOR MAPS PASS: %s built-in SP/MP, uppercase user maps, mod campaigns and deletion protection", mod.c_str());
}

inline void verifyGeneratedSpice(const std::string& output, const std::string& mod) {
    verifyEditorSpiceDefaults(output, mod);
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Generated spice check: " + message);
    };
    std::string terrain = "[BASIC]\nVersion=2\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y = 0; y < 32; ++y) terrain += fmt::sprintf("%03d=", y) + std::string(32, '-') + "\n";
    terrain += "[Player1]\nCredits=10000\n";
    GameInitSettings init("generated-spice-smoke", terrain, false, effectiveGameOptions);
    GameInitSettings::HouseInfo info(HOUSE_ATREIDES, 1);
    info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS));
    init.addHouseInfo(info);
    const std::array<std::pair<int, int>, 5> families{{
        {Terrain_Spice, Terrain_ThickSpice}, {Terrain_GreenSpice, Terrain_ThickGreenSpice},
        {Terrain_RedSpice, Terrain_ThickRedSpice}, {Terrain_PaleLilacSpice, Terrain_ThickPaleLilacSpice},
        {Terrain_WhiteSpice, Terrain_ThickWhiteSpice}}};
    const int blooms[] = {Terrain_SpiceBloom, Terrain_GreenSpiceBloom, Terrain_RedSpiceBloom,
                         Terrain_PaleLilacSpiceBloom, Terrain_WhiteSpiceBloom};
    {
        auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(init);
        auto sample = [&] {
            currentGame->randomGen.setSeed(1234567);
            std::array<int, 5> counts{};
            for(int i = 0; i < 4000; ++i) {
                const auto pair = currentGameMap->chooseGeneratedSpiceTerrain();
                const auto found = std::find(families.begin(), families.end(), pair);
                require(found != families.end(), "thin/thick family mismatch");
                ++counts[found - families.begin()];
            }
            return counts;
        };
        const auto counts = sample();
        require(counts == sample(), "same seed differs");
        for(int color = 0; color < 5; ++color)
            require((mod == "vanilla" && color > 0) ? counts[color] == 0 : counts[color] > 0,
                    mod + " missing/generated unexpected color " + std::to_string(color));
        const Uint32 seed = currentGame->randomGen.getSeed();
        require(currentGameMap->chooseGeneratedSpiceTerrain(Terrain_WhiteSpice, Terrain_ThickWhiteSpice) == families[4]
                && currentGame->randomGen.getSeed() == seed, "explicit field color consumed a random roll");
        if(mod != "vanilla") for(int color = 1; color < 5; ++color) {
            for(int x = 0; x < 32; ++x) for(int y = 0; y < 32; ++y) currentGameMap->getTile(x, y)->setType(Terrain_Sand);
            auto* center = currentGameMap->getTile(16, 16);
            center->setType(blooms[color]); center->triggerSpiceBloom(pLocalHouse);
            require(center->getType() == families[color].first && currentGameMap->getTile(17, 16)->getType() == families[color].first,
                    "colored bloom changed family");
            currentGameMap->createSpiceField(Coord(7, 7), 2, true, families[color].first, families[color].second);
            require(currentGameMap->getTile(7, 7)->getType() == families[color].second, "thick field changed family");
        }
        const unsigned graphics[] = {ObjPic_Terrain, ObjPic_Terrain_GreenSpice, ObjPic_Terrain_RedSpice,
                                     ObjPic_Terrain_PaleLilacSpice, ObjPic_Terrain_WhiteSpice};
        SDL_RenderSetClipRect(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); SDL_RenderClear(renderer);
        for(int color = 0; color < 5; ++color) for(int row = 0; row < 2; ++row) {
            const int tile = row == 0 ? Tile::TerrainTile_Spice + 15 : Tile::TerrainTile_ThickSpice + 15;
            SDL_Rect source{tile % NUM_TERRAIN_TILES_X * D2_TILESIZE, tile / NUM_TERRAIN_TILES_X * D2_TILESIZE, D2_TILESIZE, D2_TILESIZE};
            SDL_Rect target{color * 64, row * 64, 64, 64};
            require(SDL_RenderCopy(renderer, pGFXManager->getZoomedObjPic(graphics[color], HOUSE_HARKONNEN, 0), &source, &target) == 0,
                    "terrain graphic rendering");
        }
        SDL_Rect area{0, 0, 320, 128};
        auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, area.w, area.h, 32, SDL_PIXELFORMAT_ARGB8888));
        require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format, pixels->pixels, pixels->pitch) == 0,
                "spice graphic capture");
        require(SDL_SaveBMP(pixels.get(), (std::filesystem::path(output) / (mod + "-five-spices.bmp")).string().c_str()) == 0,
                "spice graphic output");
    }
    currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
    SDL_Log("GENERATED SPICE PASS: %s deterministic family selection, colored blooms and thick fields", mod.c_str());
}
