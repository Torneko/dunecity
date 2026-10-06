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

#include <globals.h>
#include <mod/ModManager.h>

#include <SoundPlayer.h>
#include <FileClasses/music/MusicPlayer.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Palfile.h>
#include <Network/NetworkManager.h>

// Explicit definitions of global variables (instead of relying on EXTERN macro)
// SDL stuff
SDL_Window*          window = nullptr;
SDL_Renderer*        renderer = nullptr;
SDL_Texture*         screenTexture = nullptr;
Palette              palette;
Palette              customPalette;
bool                 customPaletteLoaded = false;
std::array<SDL_Color, 8> rebelsColorRamp{{
    SDL_Color{ 82, 82, 82, 255 },
    SDL_Color{ 72, 72, 72, 255 },
    SDL_Color{ 62, 62, 62, 255 },
    SDL_Color{ 52, 52, 52, 255 },
    SDL_Color{ 42, 42, 42, 255 },
    SDL_Color{ 34, 34, 34, 255 },
    SDL_Color{ 27, 27, 27, 255 },
    SDL_Color{ 20, 20, 20, 255 }
}};
int                  drawnMouseX = 0;
int                  drawnMouseY = 0;
int                  currentZoomlevel = 0;

// abstraction layers
std::unique_ptr<SoundPlayer>         soundPlayer;
std::unique_ptr<MusicPlayer>         musicPlayer;
std::unique_ptr<FileManager>         pFileManager;
std::unique_ptr<GFXManager>          pGFXManager;
std::unique_ptr<SFXManager>          pSFXManager;
std::unique_ptr<FontManager>         pFontManager;
std::unique_ptr<TextManager>         pTextManager;
std::unique_ptr<NetworkManager>      pNetworkManager;

// game stuff
Game*                currentGame = nullptr;
ScreenBorder*        screenborder = nullptr;
Map*                 currentGameMap = nullptr;
House*               pLocalHouse = nullptr;
HumanPlayer*         pLocalPlayer = nullptr;

RobustList<UnitBase*>       unitList;
RobustList<StructureBase*>  structureList;
RobustList<Bullet*>         bulletList;

// misc
SettingsClass    settings;
SettingsClass::GameOptionsClass effectiveGameOptions;
bool debug = false;
std::array<int, NUM_HOUSES> houseToVisualHouse = {
    HOUSE_HARKONNEN,
    HOUSE_ATREIDES,
    HOUSE_ORDOS,
    HOUSE_FREMEN,
    HOUSE_SARDAUKAR,
    HOUSE_MERCENARY,
    HOUSE_NEUTRAL,
    HOUSE_REBELS,
    HOUSE_CUSTOM,
    HOUSECOLOR_GUEST_1,
    HOUSECOLOR_GUEST_2,
    HOUSECOLOR_GUEST_3
};

namespace {

bool crossModCustomGameHousesActive() {
    if(!ModManager::instance().isInitialized()) return false;
    const std::string activeMod = ModManager::instance().getActiveModName();
    return activeMod == "Tornie" || activeMod == "Jericho";
}

const std::array<SDL_Color, 8> wildspadeRamp{{
    {168, 20, 96, 255}, {148, 12, 84, 255}, {128, 12, 72, 255}, {108, 8, 60, 255},
    {88, 4, 48, 255}, {68, 4, 40, 255}, {48, 0, 28, 255}, {28, 0, 16, 255}
}};
const std::array<SDL_Color, 8> kleshmershOrangeRamp{{
    {255, 126, 91, 255}, {255, 96, 58, 255}, {248, 65, 24, 255}, {233, 45, 0, 255},
    {196, 35, 0, 255}, {158, 27, 0, 255}, {120, 20, 0, 255}, {82, 13, 0, 255}
}};
const std::array<SDL_Color, 8> kleshmershBrownRamp{{
    {112, 84, 64, 255}, {100, 76, 56, 255}, {88, 64, 52, 255}, {76, 56, 44, 255},
    {64, 48, 36, 255}, {52, 36, 28, 255}, {40, 28, 24, 255}, {28, 20, 16, 255}
}};
const std::array<SDL_Color, 8> tharpiqueRamp{{
    {108, 176, 228, 255}, {72, 156, 212, 255}, {48, 136, 196, 255}, {36, 120, 180, 255},
    {20, 96, 152, 255}, {12, 76, 120, 255}, {8, 52, 88, 255}, {4, 32, 52, 255}
}};
const std::array<SDL_Color, 8> darkGreyRamp{{
    {82, 82, 82, 255}, {72, 72, 72, 255}, {62, 62, 62, 255}, {52, 52, 52, 255},
    {42, 42, 42, 255}, {34, 34, 34, 255}, {27, 27, 27, 255}, {20, 20, 20, 255}
}};

}

void loadCustomPalette() {
    customPaletteLoaded = false;

    if(pFileManager == nullptr || !pFileManager->exists("Custom_IBM.PAL")) {
        return;
    }

    try {
        customPalette = LoadPalette_RW(pFileManager->openFile("Custom_IBM.PAL").get());
        customPaletteLoaded = customPalette.getNumColors() >= 256;
    } catch(const std::exception& e) {
        SDL_Log("Warning: Could not load Custom_IBM.PAL: %s", e.what());
        customPalette = Palette();
        customPaletteLoaded = false;
    }
}

bool isJerichoHouseColorSlot(int colorSlot) {
    if(!ModManager::instance().isInitialized()) return false;
    const std::string activeMod = ModManager::instance().getActiveModName();
    return (activeMod == "Jericho" && (colorSlot == HOUSE_REBELS))
        || (activeMod == "Tornie" && (colorSlot == HOUSECOLOR_GUEST_1 || colorSlot == HOUSECOLOR_GUEST_2));
}

bool isTornieRebelsColorSlot(int colorSlot) {
    if(!ModManager::instance().isInitialized()) return colorSlot == HOUSE_REBELS;
    const std::string activeMod = ModManager::instance().getActiveModName();
    return (activeMod == "Tornie" && colorSlot == HOUSE_REBELS)
        || (activeMod == "Jericho" && colorSlot == HOUSE_REBELS);
}

bool isVanillaRebelsColorSlot(int colorSlot) {
    return colorSlot == HOUSE_REBELS
        && ModManager::instance().isInitialized()
        && ModManager::instance().getActiveModName() == "vanilla";
}

int getCampaignHouseColorSlot(HOUSETYPE house, HOUSETYPE campaignHouse) {
    if(ModManager::instance().getActiveModName() == "vanilla" && campaignHouse == HOUSE_KLESHMERSH) {
        if(house == HOUSE_HARKONNEN) return HOUSECOLOR_KLESH_HARKONNEN;
        if(house == HOUSE_SARDAUKAR) return HOUSECOLOR_KLESH_SARDAUKAR;
        if(house == HOUSE_MERCENARY) return HOUSECOLOR_KLESH_MERCENARY;
    }
    return getDefaultHouseColorSlot(house);
}

int getHouseColorPaletteIndexFromSlot(int colorSlot) {
    if(colorSlot >= HOUSECOLOR_KLESH_HARKONNEN && colorSlot <= HOUSECOLOR_KLESH_MERCENARY)
        return PALCOLOR_HARKONNEN;
    if(!isValidHouseColorSlot(colorSlot)) {
        return PALCOLOR_HARKONNEN;
    }

    // Color slot 8 belongs to Corruptique's identity even though Jericho keeps
    // that faction in runtime slot 11 for existing save/profile compatibility.
    if(colorSlot==HOUSE_CUSTOM && ModManager::instance().isInitialized()
       && ModManager::instance().getActiveModName()=="Jericho") return PALCOLOR_FREMEN;

    const bool tornieMainActive = ModManager::instance().isInitialized()
        && ModManager::instance().getActiveModName() == "Tornie";
    if(tornieMainActive && colorSlot == HOUSECOLOR_CUSTOM_BRIGHT_YELLOW) {
        return PALCOLOR_NEUTRAL;
    }
    if(tornieMainActive && colorSlot == HOUSECOLOR_CUSTOM_LIGHT_PINK) {
        return PALCOLOR_FREMEN;
    }

    if(colorSlot >= HOUSECOLOR_GUEST_1 && colorSlot <= HOUSECOLOR_GUEST_3) {
        const bool jerichoActive = ModManager::instance().isInitialized()
            && ModManager::instance().getActiveModName() == "Jericho";
        if(jerichoActive) {
            static const int jerichoGuestPalette[3] = {
                PALCOLOR_NEUTRAL, PALCOLOR_REBELS, PALCOLOR_FREMEN
            };
            return jerichoGuestPalette[colorSlot - HOUSECOLOR_GUEST_1];
        }
        static const int tornieGuestPalette[3] = {
            PALCOLOR_NEUTRAL, PALCOLOR_REBELS, 136
        };
        return tornieGuestPalette[colorSlot - HOUSECOLOR_GUEST_1];
    }

    return colorSlot == HOUSE_CUSTOM
        ? getHousePaletteIndex(HOUSE_CUSTOM)
        : houseColorToPaletteIndex[colorSlot];
}

SDL_Color getHouseColorSDL(int colorSlot, int shadeOffset) {
    if(!isValidHouseColorSlot(colorSlot) || shadeOffset < 0 || shadeOffset >= 8) {
        return SDL_Color{ 0, 0, 0, 255 };
    }

    static const std::array<SDL_Color, 8> campaignGreen{{
        {60,140,64,255},{48,124,52,255},{36,108,40,255},{28,92,32,255},
        {20,76,24,255},{12,60,16,255},{8,44,12,255},{4,28,8,255}
    }};
    static const std::array<SDL_Color, 8> campaignFuchsia{{
        {255,76,212,255},{236,52,192,255},{216,32,172,255},{192,20,148,255},
        {160,12,120,255},{124,8,92,255},{88,4,64,255},{52,0,36,255}
    }};
    static const std::array<SDL_Color, 8> campaignTurquoise{{
        {76,240,220,255},{52,220,204,255},{32,196,184,255},{20,172,164,255},
        {12,140,136,255},{8,108,108,255},{4,76,80,255},{0,44,48,255}
    }};
    if(colorSlot == HOUSECOLOR_KLESH_HARKONNEN) return campaignGreen[shadeOffset];
    if(colorSlot == HOUSECOLOR_KLESH_SARDAUKAR) return campaignFuchsia[shadeOffset];
    if(colorSlot == HOUSECOLOR_KLESH_MERCENARY) return campaignTurquoise[shadeOffset];

    if(isVanillaRebelsColorSlot(colorSlot)) {
        return darkGreyRamp[shadeOffset];
    }

    if(colorSlot == HOUSECOLOR_CUSTOM_APPLE_GREEN) {
        return darkGreyRamp[shadeOffset];
    }

    if(colorSlot == HOUSECOLOR_CUSTOM_BRIGHT_YELLOW
       && ModManager::instance().isInitialized()
       && ModManager::instance().isTornieContentActive()) {
        return kleshmershBrownRamp[shadeOffset];
    }

    if(isTornieRebelsColorSlot(colorSlot)) {
        return rebelsColorRamp[shadeOffset];
    }

    if(colorSlot >= HOUSECOLOR_GUEST_1 && colorSlot <= HOUSECOLOR_GUEST_3
       && ModManager::instance().isInitialized())
       {
        if(colorSlot==HOUSECOLOR_GUEST_2 && ModManager::instance().getActiveModName()=="vanilla")
            return kleshmershBrownRamp[shadeOffset];
        if(colorSlot == HOUSECOLOR_GUEST_1) return wildspadeRamp[shadeOffset];
        if(colorSlot == HOUSECOLOR_GUEST_2) return kleshmershOrangeRamp[shadeOffset];
        return tharpiqueRamp[shadeOffset];
    }

    const Palette& sourcePalette = getPaletteForHouseColorSlot(colorSlot);
    const int paletteIndex = getHouseColorPaletteIndexFromSlot(colorSlot) + shadeOffset;
    if(paletteIndex >= 0 && paletteIndex < sourcePalette.getNumColors()) {
        SDL_Color color = sourcePalette[paletteIndex];
        if(colorSlot == HOUSECOLOR_CUSTOM_DARK_VIOLET) {
            color.r = static_cast<Uint8>((static_cast<unsigned int>(color.r) * 9U) / 10U);
            color.g = static_cast<Uint8>((static_cast<unsigned int>(color.g) * 9U) / 10U);
            color.b = static_cast<Uint8>((static_cast<unsigned int>(color.b) * 9U) / 10U);
        }
        return color;
    }
    return SDL_Color{ 0, 0, 0, 255 };
}

Uint32 getHouseColorRGB(int colorSlot, int shadeOffset) {
    return SDL2RGB(getHouseColorSDL(colorSlot, shadeOffset));
}

void applyCustomPaletteRuntimeHouseRamps() {
    static const Uint8 rebelsGreyRamp[8] = { 82, 72, 62, 52, 42, 34, 27, 20 };
    const bool tornieMainActive =
        ModManager::instance().isInitialized()
        && ModManager::instance().getActiveModName() == "Tornie";
    const bool crossModHouseColorsActive =
        ModManager::instance().isInitialized()
        && (tornieMainActive || ModManager::instance().getActiveModName() == "Jericho")
        && customPaletteLoaded
        && customPalette.getNumColors() >= PALCOLOR_SARDAUKAR + 8;

    for(int k = 0; k < 8; ++k) {
        const SDL_Color greyColor{
            rebelsGreyRamp[k], rebelsGreyRamp[k], rebelsGreyRamp[k], 255
        };
        SDL_Color rebelsColor = greyColor;

        if(crossModHouseColorsActive) {
            const SDL_Color customRebelsColor = customPalette[PALCOLOR_SARDAUKAR + k];
            const bool customSlotAlreadySwapped = customRebelsColor.r == greyColor.r
                && customRebelsColor.g == greyColor.g
                && customRebelsColor.b == greyColor.b;
            rebelsColor = customSlotAlreadySwapped ? rebelsColorRamp[k] : customRebelsColor;
            rebelsColor.a = 255;
            if(tornieMainActive) {
                customPalette[PALCOLOR_SARDAUKAR + k] = greyColor;
            }
        }

        rebelsColorRamp[k] = rebelsColor;
    }
}
bool isHouseAvailable(HOUSETYPE house) {
    if(house >= HOUSE_HARKONNEN && house < NUM_LEGACY_HOUSES) return true;
    return house == HOUSE_CUSTOM
        && ModManager::instance().isInitialized()
        && ModManager::instance().isCustomHouseRegistered();
}

int getNumAvailableHouses() {
    return NUM_LEGACY_HOUSES + (isHouseAvailable(HOUSE_CUSTOM) ? 1 : 0);
}

bool isCustomGameHouseAvailable(HOUSETYPE house) {
    if(house==HOUSE_KLESHMERSH && ModManager::instance().isInitialized()
       && ModManager::instance().getActiveModName()=="vanilla") return true;
    if(isHouseAvailable(house)) return true;
    return crossModCustomGameHousesActive()
        && house >= HOUSE_WILDSPADE && house <= HOUSE_THARPIQUE;
}

int getNumCustomGameHouses() {
    int count=0;for(int h=0;h<NUM_HOUSES;++h)count+=isCustomGameHouseAvailable(static_cast<HOUSETYPE>(h));return count;
}

bool isCampaignHouseAvailable(HOUSETYPE house) {
    if(ModManager::instance().isInitialized() && ModManager::instance().isTornieLiteActive()
       && house>=HOUSE_NEUTRAL) return false;
    return isCustomGameHouseAvailable(house);
}

HOUSETYPE getHouseFactionIdentity(HOUSETYPE house) {
    if(!crossModCustomGameHousesActive()
       || ModManager::instance().getActiveModName() != "Jericho") {
        return house;
    }
    switch(house) {
        case HOUSE_NEUTRAL:    return HOUSE_WILDSPADE;
        case HOUSE_REBELS:     return HOUSE_KLESHMERSH;
        case HOUSE_CUSTOM:     return HOUSE_THARPIQUE;
        case HOUSE_WILDSPADE:  return HOUSE_NEUTRAL;
        case HOUSE_KLESHMERSH: return HOUSE_REBELS;
        case HOUSE_THARPIQUE:  return HOUSE_CUSTOM;
        default:               return house;
    }
}

HOUSETYPE getRuntimeHouseForIdentity(HOUSETYPE identity) {
    if(!crossModCustomGameHousesActive()
       || ModManager::instance().getActiveModName() != "Jericho") {
        return identity;
    }
    switch(identity) {
        case HOUSE_NEUTRAL:    return HOUSE_WILDSPADE;
        case HOUSE_REBELS:     return HOUSE_KLESHMERSH;
        case HOUSE_CUSTOM:     return HOUSE_THARPIQUE;
        case HOUSE_WILDSPADE:  return HOUSE_NEUTRAL;
        case HOUSE_KLESHMERSH: return HOUSE_REBELS;
        case HOUSE_THARPIQUE:  return HOUSE_CUSTOM;
        default:               return identity;
    }
}

bool isHouseFaction(HOUSETYPE house, HOUSETYPE identity) {
    const auto actual=getHouseFactionIdentity(house);
    return actual==identity || (identity==HOUSE_NEUTRAL && actual==HOUSE_KLESHMERSH
        && ModManager::instance().isInitialized() && ModManager::instance().getActiveModName()=="vanilla");
}

int getDefaultHouseColorSlot(HOUSETYPE house) {
    const HOUSETYPE identity = getHouseFactionIdentity(house);
    if(identity == HOUSE_REBELS
       && ModManager::instance().isInitialized()
       && ModManager::instance().getActiveModName() == "vanilla") {
        return HOUSECOLOR_CUSTOM_APPLE_GREEN;
    }

    if(identity >= HOUSE_WILDSPADE && identity <= HOUSE_THARPIQUE) {
        return HOUSECOLOR_GUEST_1 + (identity - HOUSE_WILDSPADE);
    }
    return identity >= HOUSE_HARKONNEN && identity < NUM_CAMPAIGN_HOUSES
        ? static_cast<int>(identity)
        : HOUSE_HARKONNEN;
}

char getHouseScenarioLetter(HOUSETYPE house) {
    switch(getHouseFactionIdentity(house)) {
        case HOUSE_HARKONNEN: return 'H';
        case HOUSE_ATREIDES: return 'A';
        case HOUSE_ORDOS: return 'O';
        case HOUSE_FREMEN: return 'F';
        case HOUSE_SARDAUKAR: return 'S';
        case HOUSE_MERCENARY: return 'M';
        case HOUSE_NEUTRAL: return 'N';
        case HOUSE_REBELS: return 'R';
        case HOUSE_CUSTOM: return 'C';
        case HOUSE_WILDSPADE: return 'W';
        case HOUSE_KLESHMERSH: return 'K';
        case HOUSE_THARPIQUE: return 'T';
        default: return '?';
    }
}

std::string getHouseRegionPrefix(HOUSETYPE house) {
    static const char* const prefixes[NUM_HOUSES] = {
        "HAR", "ATR", "ORD", "FRE", "SAR", "MER", "NEU", "REB", "COR", "WIL", "KLE", "THA"
    };
    const HOUSETYPE identity = getHouseFactionIdentity(house);
    return identity >= HOUSE_HARKONNEN && identity < NUM_HOUSES ? prefixes[identity] : std::string();
}

int getHousePaletteIndex(HOUSETYPE house) {
    const HOUSETYPE identity = getHouseFactionIdentity(house);
    if(identity == HOUSE_CUSTOM || identity == HOUSE_THARPIQUE) {
        const CustomHouseInfo& info = ModManager::instance().getCustomHouseInfo(house);
        return info.enabled ? info.paletteIndex : PALCOLOR_HARKONNEN;
    }
    return (identity >= HOUSE_HARKONNEN && identity < NUM_HOUSES)
        ? houseToPaletteIndex[identity]
        : PALCOLOR_HARKONNEN;
}

HOUSETYPE getHouseFallbackHouse(HOUSETYPE house) {
    const HOUSETYPE identity = getHouseFactionIdentity(house);
    if(identity == HOUSE_CUSTOM || identity == HOUSE_THARPIQUE) {
        const CustomHouseInfo& info = ModManager::instance().getCustomHouseInfo(house);
        return info.enabled ? static_cast<HOUSETYPE>(info.fallbackHouse) : HOUSE_HARKONNEN;
    }
    if(identity == HOUSE_WILDSPADE) return HOUSE_NEUTRAL;
    if(identity == HOUSE_KLESHMERSH) return ModManager::instance().getActiveModName()=="vanilla" ? HOUSE_NEUTRAL : HOUSE_REBELS;
    return identity;
}

void resetHouseVisualHouseMapping() {
    for(int house = 0; house < NUM_HOUSES; ++house) {
        houseToVisualHouse[house] = getDefaultHouseColorSlot(static_cast<HOUSETYPE>(house));
    }
}

void setHouseVisualHouse(HOUSETYPE house, int visualHouse) {
    if(house < 0 || house >= NUM_HOUSES) {
        return;
    }

    if(!isValidHouseColorSlot(visualHouse)) {
        houseToVisualHouse[house] = getDefaultHouseColorSlot(house);
        return;
    }

    houseToVisualHouse[house] = visualHouse;
}
