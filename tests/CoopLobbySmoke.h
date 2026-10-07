#pragma once

#include <Campaign/CoopCampaignRuntime.h>
#include <Menu/CustomGamePlayers.h>
#include <Menu/CoopCampaignMenu.h>
#include <Menu/MainMenu.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/INIFile.h>
#include <misc/SaveGameLobbySetup.h>
#include <misc/FileSystem.h>
#include <misc/OMemoryStream.h>
#include <array>

inline void verifyCoopLobby(const std::string& output, const std::string& mod) {
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Co-op lobby check: " + message);
    };
    require(!pNetworkManager, "isolated check has an active network connection");
    std::vector<int> roster;
    for(int h = 0; h < NUM_HOUSES; ++h)
        if(isCampaignHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
    require(roster.size() >= coop::SlotCount, "available faction roster");
    const std::string peer = settings.general.playerName == "Coop Peer" ? "Coop Peer 2" : "Coop Peer";
    const auto session = coop::CoopCampaignSession::create("lobby-check", mod, roster,
        {roster[0], roster[1]}, {settings.general.playerName, peer}, 54321, false);
    auto options = effectiveGameOptions;
    options.chaosMode = false;
    auto init = coop::makeGameSettings(session, options, "Coop lobby check");
    const auto capture = [&](MenuBase& menu, const std::string& suffix) {
        SDL_RenderSetClipRect(renderer, nullptr);
        SDL_RenderClear(renderer);
        menu.draw();
        auto pixels = renderReadSurface(renderer);
        require(pixels && SavePNG(pixels.get(), (std::filesystem::path(output)
            / (mod + "-coop-" + suffix + ".png")).string().c_str()) == 0, "menu capture");
    };
    {
        MainMenu menu;
        capture(menu, "main-menu");
    }
    {
        CoopCampaignMenu menu(true);
        capture(menu, "setup");
    }
    for(const bool pendingSource : {true, false}) {
        auto probe = init;
        if(pendingSource) probe.setMapData(probe.getFiledata() + "\nSourcePending=1\n");
        CustomGamePlayers lobby(probe, false);
        ChangeEventList changes;
        changes.changeEventList.emplace_back(0, settings.general.playerName);
        changes.changeEventList.emplace_back(2, peer);
        changes.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, 0, roster[2]);
        lobby.onReceiveChangeEventList(changes);
        lobby.addAllPlayersToGameInitSettings();
        const auto committed = coop::readContext(lobby.gameInitSettings.getFiledata());
        require(committed && committed->sourceFaction == (pendingSource ? roster[2] : roster[0])
                && committed->slots[0].faction == roster[2],
                "new lobby source ignored the host selection or a resumed stage-one source changed");
        auto stream = sdl2::RWops_ptr{SDL_RWFromConstMem(lobby.gameInitSettings.getFiledata().data(),
            static_cast<int>(lobby.gameInitSettings.getFiledata().size()))};
        INIFile map(stream.get());
        require(!map.hasKey("COOP", "SourcePending"), "pending source flag leaked into committed game settings");
    }
    {
        CustomGamePlayers lobby(init, false);
        ChangeEventList changes;
        changes.changeEventList.emplace_back(0, settings.general.playerName);
        changes.changeEventList.emplace_back(2, peer);
        changes.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, 0, roster[0]);
        changes.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, 1, roster[0]);
        lobby.onReceiveChangeEventList(changes);
        // With both human slots filled this returns the state without placing
        // the probe in a slot. The three opponents must remain AI-controlled.
        const auto state = lobby.getChangeEventListForNewPlayer("Unused probe");
        int humans = 0, sameFaction = 0, opponents = 0;
        for(const auto& event : state.changeEventList) {
            if(event.eventType == ChangeEventList::ChangeEvent::EventType::SetHumanPlayer) {
                require(event.slot == 0 || event.slot == 2, "a human replaced an opponent");
                ++humans;
            }
            if(event.eventType == ChangeEventList::ChangeEvent::EventType::ChangeHouse
               && event.slot < 2 && event.newValue == static_cast<Uint32>(roster[0])) ++sameFaction;
            if(event.eventType == ChangeEventList::ChangeEvent::EventType::ChangePlayer
               && event.slot >= 4 && event.slot % 2 == 0) {
                require(event.newValue != static_cast<Uint32>(-1)
                    && event.newValue != static_cast<Uint32>(-2), "opponent slot is open");
                ++opponents;
            }
        }
        require(humans == 2 && sameFaction == 2 && opponents == 3,
            "same-faction choice or locked opponent slots were lost");
        capture(lobby, "same-faction-lobby");
        ChangeEventList late;
        late.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, 0, roster[1]);
        lobby.bWaitingForModAcks = true;
        lobby.enforceCoopLobby();
        require(!lobby.houseInfo[0].houseDropDown.isEnabled(), "choices remain enabled during synchronization");
        lobby.onReceiveChangeEventList(late);
        require(lobby.houseInfo[0].houseDropDown.getSelectedEntryIntData() == roster[0],
            "late edit changed the committed host snapshot");
        lobby.bWaitingForModAcks = false;
        lobby.startGameTime = SDL_GetTicks() + 3000;
        lobby.onReceiveChangeEventList(late);
        require(lobby.houseInfo[0].houseDropDown.getSelectedEntryIntData() == roster[0], "late countdown edit");
        lobby.onPeerDisconnected(peer, false, 0);
        require(lobby.startGameTime == 0 && !lobby.bWaitingForModAcks
            && lobby.houseInfo[1].player1DropDown.getSelectedEntryIntData() == -1,
            "departing partner did not cancel launch");
        lobby.startGameTime = SDL_GetTicks() + 3000;
        lobby.update();
        require(lobby.startGameTime == 0, "launch was not revalidated before entering the game");
    }
    {
        const auto path = std::filesystem::path(output) /
            (mod + "-same-faction-" + std::to_string(roster[0]) + ".sav");
        const auto bytes = readCompleteFile(path.string());
        const auto saved = readSaveGameLobbySetup(bytes);
        require(saved.modName == mod && saved.settings.getGameType() == GameType::CustomMultiplayer,
            "checkpoint settings cursor is misaligned");
        const auto context = coop::readContext(saved.settings.getFiledata());
        require(context && saved.houses.size() == context->slots.size(), "checkpoint lost co-op setup");
        GameInitSettings load(path.filename().string(), bytes, "Coop lobby resume check");
        CustomGamePlayers lobby(load, false);
        require(lobby.coopSession && lobby.numHouses == static_cast<int>(context->slots.size()), "saved co-op lobby is not recognized");
        for(int i = 0; i < lobby.numHouses; ++i) {
            require(lobby.houseInfo[i].houseDropDown.getSelectedEntryIntData() == context->slots[i].house,
                "checkpoint ownership slots changed in the lobby");
            require(lobby.houseInfo[i].colorDropDown.getSelectedEntryIntData() == saved.houses[i].colorOfHouse,
                "checkpoint colors were not restored in the lobby");
        }
        capture(lobby, "resume-lobby");
    }
    if(mod == "vanilla") {
        // OPENSD2 Sardaukar's real finale has five opponents. Its seven rows
        // must survive both the initial lobby and the saved-game setup cursor.
        SDL_Log("COOP LOBBY FINAL: preparing seven-participant OPENSD2 fixture");
        auto finalSession = coop::CoopCampaignSession::create("lobby-seven-participants", mod, roster,
            {HOUSE_SARDAUKAR, HOUSE_ORDOS}, {settings.general.playerName, peer}, 54321, false);
        for(int mission = 1; mission < coop::StageCount; ++mission) finalSession.completeMission(true);
        const auto finalInit = coop::makeGameSettings(finalSession, options, "Seven-participant finale");
        const auto finalContext = coop::readContext(finalInit.getFiledata());
        require(finalContext && finalContext->slots.size() == 7, "real OPENSD2 finale lost an opponent before the lobby");
        {
            CustomGamePlayers lobby(finalInit, false);
            require(lobby.coopSession && lobby.numHouses == 7, "initial finale lobby truncated participants");
            // An isolated client has no network manager. Occupy both humans
            // before probing a third join so that no network send is requested.
            ChangeEventList changes;
            changes.changeEventList.emplace_back(0, settings.general.playerName);
            changes.changeEventList.emplace_back(2, peer);
            lobby.onReceiveChangeEventList(changes);
            require(lobby.houseInfo[0].player1DropDown.getSelectedEntryIntData() == 0
                    && lobby.houseInfo[1].player1DropDown.getSelectedEntryIntData() == 0,
                    "finale probe lacks its two occupied human slots");
            const auto state = lobby.getChangeEventListForNewPlayer("Unused probe");
            int enemies = 0;
            for(const auto& event : state.changeEventList)
                if(event.eventType == ChangeEventList::ChangeEvent::EventType::ChangePlayer
                        && event.slot >= 4 && event.slot % 2 == 0) {
                    require(event.newValue != static_cast<Uint32>(-1) && event.newValue != static_cast<Uint32>(-2),
                            "finale enemy row became an open human slot");
                    ++enemies;
                }
            require(enemies == 5, "finale lobby does not preserve five campaign AIs");
            capture(lobby, "seven-participant-finale");
            SDL_Log("COOP LOBBY FINAL: seven participant rows and five locked opponents verified");
        }
        const auto path = (std::filesystem::path(output) / "vanilla-coop-seven-participants.sav").string();
        {
            auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(finalInit);
            require(game->saveGame(path), "seven-participant finale checkpoint failed");
        }
        currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        const auto bytes = readCompleteFile(path);
        const auto saved = readSaveGameLobbySetup(bytes);
        require(saved.houses.size() == 7, "save lobby setup omitted final-mission participants");
        GameInitSettings load(getBasename(path, true), bytes, "Seven-participant finale resume");
        CustomGamePlayers lobby(load, false);
        require(lobby.coopSession && lobby.numHouses == 7, "resumed finale lobby truncated participants");
        for(int i = 0; i < 7; ++i)
            require(lobby.houseInfo[i].houseDropDown.getSelectedEntryIntData() == finalContext->slots[i].house
                    && lobby.houseInfo[i].colorDropDown.getSelectedEntryIntData() == saved.houses[i].colorOfHouse,
                    "resumed finale lobby lost ownership or player-selected colors");
        capture(lobby, "seven-participant-resume");
        SDL_Log("COOP LOBBY FINAL PASS: seven-participant checkpoint ownership/colors restored");
    }
    {
        // Ordinary multiplayer saves made before cooperative campaigns must
        // still open the standard lobby, including their explicit colors.
        auto ordinary = init;
        ordinary.setMapData(coop::readMissionTemplate(session));
        const auto path = (std::filesystem::path(output) / (mod + "-ordinary-multiplayer.sav")).string();
        currentGame = new Game();
        currentGame->initGame(ordinary);
        require(currentGame->saveGame(path), "ordinary multiplayer checkpoint write");
        delete currentGame;
        currentGame = nullptr;
        resetHouseVisualHouseMapping();
        const auto bytes = readCompleteFile(path);
        const auto saved = readSaveGameLobbySetup(bytes);
        require(!coop::readContext(saved.settings.getFiledata()), "ordinary save became a co-op campaign");
        GameInitSettings load(getBasename(path, true), bytes, "Ordinary resume check");
        CustomGamePlayers lobby(load, false);
        require(!lobby.coopSession && lobby.numHouses == static_cast<int>(saved.houses.size()),
            "ordinary multiplayer lobby regression");
        for(int i = 0; i < lobby.numHouses; ++i)
            require(lobby.houseInfo[i].colorDropDown.getSelectedEntryIntData() == saved.houses[i].colorOfHouse,
                "ordinary saved color lost");
    }
    {
        // A pre-mod multiplayer header must not lose its outer setup count to
        // the optional MOD read. Write its historical base layout explicitly.
        OMemoryStream legacy;
        legacy.writeUint32(SAVEMAGIC);
        legacy.writeUint32(9805);
        legacy.writeString("dunelegacy0.99.4");
        const auto settingsOffset = legacy.getDataLength();
        legacy.writeSint8(static_cast<Sint8>(GameType::CustomMultiplayer));
        legacy.writeSint8(HOUSE_INVALID);
        legacy.writeString("legacy-multiplayer.ini");
        legacy.writeString(coop::readMissionTemplate(session));
        legacy.writeUint8(0);
        legacy.writeUint32(0); // played regions
        legacy.writeUint32(0); // tutorial hints
        legacy.writeUint32(54321);
        legacy.writeBool(false); // shared house control
        legacy.writeUint32(4); // game speed
        legacy.writeBool(true); // concrete required
        legacy.writeBool(false); // concrete degradation
        legacy.writeBool(true); // fog
        legacy.writeBool(false); // explored map
        legacy.writeBool(false); // instant build
        legacy.writeBool(true); // one palace
        legacy.writeBool(true); // powered rocket turrets
        legacy.writeBool(true); // worm respawn
        legacy.writeBool(true); // killed worms drop spice
        legacy.writeBool(false); // manual carryall drops
        legacy.writeSint32(-1); // unit limit override
        legacy.writeSint32(-1); // harvester limit override
        legacy.writeBool(false); // immortal player
        GameInitSettings::HouseInfo active(HOUSE_HARKONNEN, 1);
        active.addPlayerInfo({settings.general.playerName, "HumanPlayer"});
        GameInitSettings::HouseInfo closed(HOUSE_UNUSED, 2);
        GameInitSettings::HouseInfo random(HOUSE_INVALID, 2);
        random.addPlayerInfo({"Legacy AI", "CampaignAIPlayer"});
        const std::array<GameInitSettings::HouseInfo, 3> rows{active, closed, random};
        legacy.writeUint32(rows.size());
        for(const auto& house : rows) house.save(legacy);
        legacy.writeUint32(rows.size()); // actual setup follows without a MOD marker
        for(const auto& house : rows) house.save(legacy);
        constexpr Uint32 nextField = 0x13572468;
        legacy.writeUint32(nextField);
        const std::string bytes(legacy.getData(), legacy.getDataLength());
        const auto saved = readSaveGameLobbySetup(bytes);
        require(saved.version == 9805 && saved.modName == "vanilla"
            && saved.settings.getHouseID() == HOUSE_INVALID
            && saved.settings.getGameOptions().gameSpeed == 4,
            "pre-mod multiplayer settings were misread");
        require(saved.houses.size() == rows.size()
            && saved.houses[0].houseID == HOUSE_HARKONNEN
            && saved.houses[0].playerInfoList.size() == 1
            && saved.houses[0].playerInfoList[0].playerName == settings.general.playerName
            && saved.houses[1].houseID == HOUSE_UNUSED && saved.houses[1].playerInfoList.empty()
            && saved.houses[2].houseID == HOUSE_INVALID,
            "legacy active, closed or Random rows were lost");
        IMemoryStream settingsStream(bytes.data() + settingsOffset,
            static_cast<int>(bytes.size() - settingsOffset));
        GameInitSettings legacySettings(settingsStream, false);
        require(settingsStream.readUint32() == rows.size(), "pre-mod settings consumed the setup count");
        for(unsigned int i = 0; i < rows.size(); ++i)
            require(GameInitSettings::HouseInfo(settingsStream).houseID == rows[i].houseID,
                "pre-mod settings cursor changed ownership");
        require(settingsStream.readUint32() == nextField, "pre-mod settings consumed the following game field");
        GameInitSettings load("legacy-multiplayer.sav", bytes, "Legacy resume check");
        CustomGamePlayers lobby(load, false);
        require(!lobby.coopSession && lobby.numHouses == static_cast<int>(rows.size())
            && lobby.houseInfo[1].player1DropDown.getSelectedEntryIntData() == -2,
            "legacy closed row cannot reopen in the ordinary lobby");
    }
    SDL_Log("COOP LOBBY PASS: %s, same-faction choices, locked opponents, checkpoint colors, frozen countdown, disconnect cancellation and legacy header", mod.c_str());
}
