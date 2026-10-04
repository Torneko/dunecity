#pragma once

#include <MapEditor/MapEditor.h>

// Exercise the same UI resources and map serializer used by the editor, with
// both a cold mod startup and colors left behind by a custom game.
inline void verifyEditorHouseColors(const std::string& output, const std::string& mod,
                                   const std::string& stage) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Editor house color check: " + message);
    };
    const int count = ModManager::instance().isTornieLiteActive() ? 6 : getNumAvailableHouses();
    SDL_RenderSetClipRect(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);
    for(int i = 0; i < count; ++i) {
        const auto house = static_cast<HOUSETYPE>(i);
        int expected = i;
        if(mod == "Jericho" && i >= HOUSE_NEUTRAL) expected = HOUSECOLOR_GUEST_1 + i - HOUSE_NEUTRAL;
        if(mod == "vanilla" && i == HOUSE_REBELS) expected = HOUSECOLOR_CUSTOM_APPLE_GREEN;
        require(getHouseVisualHouse(house) == expected,
                mod + " " + stage + " wrong color for " + getHouseDisplayNameByNumber(house)
                + ": " + std::to_string(getHouseVisualHouse(house)) + " instead of " + std::to_string(expected));
        require(getHouseInterfaceColor(house) == getHouseColorRGB(expected),
                mod + " " + stage + " interface color mismatch");

        auto* surface = pGFXManager->getUIGraphicSurface(UI_MapEditor_Windtrap, house);
        require(surface != nullptr, "missing editor building icon");
        if(surface->format->palette) {
            const int base = expected == HOUSE_CUSTOM || isCustomHouseColorSlot(expected)
                || isTornieRebelsColorSlot(expected) || isVanillaRebelsColorSlot(expected)
                || isJerichoHouseColorSlot(expected) ? PALCOLOR_HARKONNEN : getHouseColorPaletteIndexFromSlot(expected);
            for(int shade = 0; shade < 8; ++shade) {
                const SDL_Color actual = surface->format->palette->colors[base + shade];
                const SDL_Color wanted = getHouseColorSDL(expected, shade);
                require(actual.r == wanted.r && actual.g == wanted.g && actual.b == wanted.b,
                        mod + " " + stage + " editor palette mismatch for " + getHouseDisplayNameByNumber(house));
            }
        }
        const auto color = getHouseColorSDL(expected);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_Rect swatch{64 * i + 4, 4, 56, 8};
        SDL_RenderFillRect(renderer, &swatch);
        for(const auto& icon : {std::pair<unsigned, int>{UI_MapEditor_Windtrap, 16},
                               std::pair<unsigned, int>{UI_MapEditor_Trike, 56}}) {
            auto* texture = pGFXManager->getUIGraphic(icon.first, house);
            int w = 0, h = 0;
            require(texture && SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) == 0, "missing icon texture");
            SDL_Rect destination{64 * i + (64 - w) / 2, icon.second, w, h};
            require(SDL_RenderCopy(renderer, texture, nullptr, &destination) == 0, "icon rendering");
        }
    }
    SDL_Rect area{0, 0, count * 64, 96};
    auto pixels = sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0, area.w, area.h, 32, SDL_PIXELFORMAT_ARGB8888));
    require(pixels && SDL_RenderReadPixels(renderer, &area, pixels->format->format, pixels->pixels, pixels->pitch) == 0,
            "editor color capture");
    require(SDL_SaveBMP(pixels.get(), (std::filesystem::path(output) / (mod + "-editor-colors-" + stage + ".bmp")).string().c_str()) == 0,
            "editor color output");
    SDL_Log("EDITOR COLORS PASS: %s %s, %d faction mappings, interface colors and editor palettes",
            mod.c_str(), stage.c_str(), count);
}

inline void verifyEditorHouseColorRoundtrip(const std::string& output, const std::string& mod) {
    // A custom game can assign any team color. Entering the editor must restore
    // the default faction colors before its first sidebar/icon is constructed.
    for(int i = 0; i < NUM_HOUSES; ++i) setHouseVisualHouse(static_cast<HOUSETYPE>(i), HOUSE_HARKONNEN);
    MapEditor editor;
    verifyEditorHouseColors(output, mod, "after-custom-game");
    editor.setMap(MapData(32, 32, Terrain_Rock), MapInfo());
    for(auto& player : editor.getPlayers()) {
        player.bActive = true;
        player.bAnyHouse = false;
        const int x = 1 + 3 * player.house;
        editor.getStructureList().emplace_back(100 + player.house, player.house, Structure_WindTrap, 256, Coord(x, 2));
        editor.getUnitList().emplace_back(200 + player.house, player.house, Unit_Trike, 256, Coord(x, 8), 0, GUARD);
    }
    const auto count = editor.getPlayers().size();
    const auto path = (std::filesystem::path(output) / (mod + "-editor-colors.ini")).string();
    editor.saveMap(path);
    editor.loadMap(path);
    if(editor.getStructureList().size() != count || editor.getUnitList().size() != count)
        throw std::runtime_error("Editor house color check: map objects lost in roundtrip");
    for(const auto& player : editor.getPlayers()) {
        if(!player.bActive || player.bAnyHouse)
            throw std::runtime_error("Editor house color check: faction identity lost in roundtrip");
        const auto* structure = editor.getStructure(100 + player.house);
        const auto* unit = editor.getUnit(200 + player.house);
        if(!structure || !unit || structure->house != player.house || unit->house != player.house)
            throw std::runtime_error("Editor house color check: object ownership changed in roundtrip");
    }
    verifyEditorHouseColors(output, mod, "map-reload");
}
