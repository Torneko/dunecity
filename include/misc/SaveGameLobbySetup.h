#pragma once

#include <GameInitSettings.h>
#include <misc/IMemoryStream.h>
#include <stdexcept>

// The lobby only needs the saved settings and ownership/color setup. Keep its
// cursor aligned with Game::loadSaveGame, including the optional mod metadata.
struct SavedGameLobbySetup {
    GameInitSettings settings;
    GameInitSettings::HouseInfoList houses;
    std::string modName = "vanilla";
    Uint32 version = 0;
};

inline SavedGameLobbySetup readSaveGameLobbySetup(const std::string& bytes) {
    IMemoryStream stream(bytes.data(), bytes.size());
    if(stream.readUint32() != SAVEMAGIC) throw std::runtime_error("Invalid savegame magic.");
    SavedGameLobbySetup result;
    result.version = stream.readUint32();
    if(result.version < 9705 || result.version > SAVEGAMEVERSION)
        throw std::runtime_error("Unsupported savegame version.");
    stream.readString(); // Game version text.
    if(result.version >= 9806) {
        result.modName = stream.readString();
        stream.readString(); // Mod checksum.
    }
    result.settings = GameInitSettings(stream, result.version >= 9806);
    if(result.version <= 9820) result.settings.migrateLegacyHouseColorSlots();
    const auto count = stream.readUint32();
    if(count == 0 || count > MAX_CUSTOM_GAME_PLAYERS)
        throw std::runtime_error("Invalid saved house setup.");
    for(Uint32 i = 0; i < count; ++i) {
        auto house = GameInitSettings::HouseInfo(stream);
        if(house.houseID < HOUSE_UNUSED || house.houseID >= NUM_HOUSES || house.playerInfoList.size() > 2)
            throw std::runtime_error("Invalid saved player setup.");
        result.houses.push_back(std::move(house));
    }
    if(result.version >= 9814) {
        constexpr Uint32 setupColorMarker = 0x53434F4C; // SCOL, Game::saveGame.
        if(stream.readUint32() != setupColorMarker)
            throw std::runtime_error("Missing saved house colors.");
        const auto colors = stream.readUint32();
        if(colors != count) throw std::runtime_error("Invalid saved color count.");
        for(auto& house : result.houses) {
            house.colorOfHouse = stream.readSint32();
            if(result.version <= 9820)
                house.colorOfHouse = migrateLegacyHouseColorSlot(house.colorOfHouse);
        }
    }
    return result;
}
