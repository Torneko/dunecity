#pragma once

#include <Campaign/CoopCampaignRuntime.h>
#include <Menu/CustomGamePlayers.h>
#include <Menu/CoopCampaignMenu.h>
#include <Menu/MainMenu.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/INIFile.h>
#include <GUI/dune/LoadSaveWindow.h>
#include <misc/fnkdat.h>
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
        auto ownedLobby = std::make_unique<CustomGamePlayers>(probe, false);
        auto& lobby = *ownedLobby;
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
    if(mod == "Tornie") {
        pNetworkManager = std::make_unique<NetworkManager>(0, "", true);
        {
            auto ownedLobby = std::make_unique<CustomGamePlayers>(init, true, true);
            auto& lobby = *ownedLobby;
            ChangeEventList choice;
            choice.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer,
                2, PlayerFactory::getIndexByPlayerClass("qBotMedium"));
            lobby.onReceiveChangeEventList(choice);
            require(lobby.isCoopLobbyReady(), "AI guest lobby still requires a human network peer");
            lobby.onNext();
            require(lobby.startGameTime > 0 && !lobby.bWaitingForModAcks,
                "AI guest launch waits for an absent player's acknowledgement");
            lobby.startGameTime = 0;
        }
        pNetworkManager.reset();
    }
    {
        auto ownedLobby = std::make_unique<CustomGamePlayers>(init, false);
        auto& lobby = *ownedLobby;
        const int ally = PlayerFactory::getIndexByPlayerClass("qBotMedium");
        const int enemy = PlayerFactory::getIndexByPlayerClass("mentatEasy");
        ChangeEventList choices;
        choices.changeEventList.emplace_back(0, settings.general.playerName);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, 2, ally);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, 4, enemy);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeAlliedControl, 0, 1);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeCoopEasyMode, 0, 1);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeExtraEnemyForces, 0, 1);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeColor, 0, HOUSECOLOR_CUSTOM_FUCHSIA);
        choices.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeColor, 1, HOUSECOLOR_CUSTOM_TEAL);
        lobby.onReceiveChangeEventList(choices);
        lobby.addAllPlayersToGameInitSettings();
        const auto context = coop::readContext(lobby.gameInitSettings.getFiledata());
        require(context && context->alliedControl && context->slots[1].playerClass == "qBotMedium"
            && context->slots[2].playerClass == "mentatEasy", "AI/control choices lost during lobby commit");
        require(context->easyMode && context->extraEnemyForces && context->stage == 2 && context->completedMask == 1,
            "lobby options or Easy Mode mission-two start lost");
        const auto& setup = lobby.gameInitSettings;
        for(int slot = 0; slot < static_cast<int>(context->slots.size()); ++slot) {
            const int owner = context->slots[slot].house;
            require(setup.campaignPurchasePrice(100, owner) == (slot < 2 ? 75 : 100), "co-op discount escaped allied slots");
            require(setup.campaignStartingCredits(1000, owner) == (slot < 2 ? 1500 : 1000), "co-op starting funds escaped allied slots");
        }
        require(context->slots[0].color == HOUSECOLOR_CUSTOM_FUCHSIA
            && context->slots[1].color == HOUSECOLOR_CUSTOM_TEAL, "chosen colors reverted to faction defaults");
        auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(lobby.gameInitSettings);
        auto* host = game->getHouse(context->slots[0].house);
        auto* guest = game->getHouse(context->slots[1].house);
        require(guest->isAI() && guest->getTeamID() == host->getTeamID(), "AI guest joined the enemy team");
        require(game->canControlHouse(guest) && !game->canControlHouse(game->getHouse(context->slots[2].house)),
            "allied control includes enemies or excludes ally");
        UnitBase* target = nullptr;
        for(auto* unit : unitList) if(unit->getOwner() == guest && unit->isRespondable()) { target = unit; break; }
        require(target, "AI ally has no controllable starting unit");
        Command(pLocalPlayer->getPlayerID(), CMD_UNIT_SETMODE, target->getObjectID(), HUNT).executeCommand();
        require(target->getAttackMode() == HUNT, "host could not issue an allied unit order");
        auto map = lobby.gameInitSettings.getFiledata();
        const auto at = map.find("AlliedControl=1"); require(at != std::string::npos, "missing control flag");
        map.replace(at, 15, "AlliedControl=0");
        game->updateCoopParticipants(map, lobby.gameInitSettings.getHouseInfoList());
        Command(pLocalPlayer->getPlayerID(), CMD_UNIT_SETMODE, target->getObjectID(), GUARD).executeCommand();
        require(target->getAttackMode() == HUNT && !game->canControlHouse(guest), "disabled allied control still accepts orders");
        game.reset(); currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
        capture(lobby, "ai-ally-control-colors");
    }
    {
        auto ownedLobby = std::make_unique<CustomGamePlayers>(init, false);
        auto& lobby = *ownedLobby;
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
        auto ownedLobby = std::make_unique<CustomGamePlayers>(load, false);
        auto& lobby = *ownedLobby;
        require(lobby.coopSession && lobby.numHouses == static_cast<int>(context->slots.size()), "saved co-op lobby is not recognized");
        require(!lobby.loadCoopSaveButton.isVisible() && !lobby.loadCoopSaveButton.isEnabled(),
            "a guest can choose a host save");
        for(int i = 0; i < lobby.numHouses; ++i) {
            require(lobby.houseInfo[i].houseDropDown.getSelectedEntryIntData() == context->slots[i].house,
                "checkpoint ownership slots changed in the lobby");
            require(lobby.houseInfo[i].colorDropDown.getSelectedEntryIntData() == saved.houses[i].colorOfHouse,
                "checkpoint colors were not restored in the lobby");
        }
        capture(lobby, "resume-lobby");
    }
    {
        const auto fixture = std::filesystem::path(output) /
            (mod + "-same-faction-" + std::to_string(roster[0]) + ".sav");
        char manual[FILENAME_MAX], automatic[FILENAME_MAX];
        fnkdat("mpsave/", manual, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        fnkdat("coop/", automatic, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        const auto name = mod + "-manual-lobby-check";
        const auto autoName = mod + "-automatic-lobby-check";
        std::filesystem::copy_file(fixture, std::filesystem::path(manual) / (name + ".dls"),
            std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy_file(fixture, std::filesystem::path(automatic) / (autoName + ".dls"),
            std::filesystem::copy_options::overwrite_existing);
        for(int directory = 0; directory < 2; ++directory) {
            auto ownedLobby = std::make_unique<CustomGamePlayers>(init, true);
            auto& lobby = *ownedLobby;
            require(lobby.loadCoopSaveButton.isVisible() && lobby.loadCoopSaveButton.isEnabled(),
                "host has no accessible save loader");
            lobby.bWaitingForModAcks = true;
            lobby.enforceCoopLobby();
            lobby.onLoadCoopSave();
            require(!lobby.hasChildWindow() && !lobby.loadCoopSaveButton.isEnabled(),
                "save selection is allowed during synchronization");
            lobby.bWaitingForModAcks = false;
            lobby.enforceCoopLobby();
            lobby.onLoadCoopSave();
            auto* picker = dynamic_cast<LoadSaveWindow*>(lobby.pChildWindow);
            require(picker, "host save button did not open the picker");
            picker->onDirectoryChange(directory);
            const auto expected = directory == 0 ? name : autoName;
            int selected = -1;
            for(int i = 0; i < picker->fileList.getNumEntries(); ++i)
                if(picker->fileList.getEntry(i) == expected) selected = i;
            require(selected >= 0, "manual/automatic save is absent from its directory tab");
            picker->fileList.setSelectedItem(selected);
            capture(lobby, directory == 0 ? "manual-save-picker" : "automatic-save-picker");
            picker->onOK();
            lobby.processChildWindowOpenCloses();
            require(lobby.isQuiting() && lobby.selectedCoopSave
                && lobby.selectedCoopSave->getGameType() == GameType::LoadMultiplayer,
                "selected save did not request a new host lobby");
            require(lobby.selectedCoopSave->getFiledata() == readCompleteFile(fixture.string()),
                "save selection changed the saved mission state");
            auto ownedResumed = std::make_unique<CustomGamePlayers>(*lobby.selectedCoopSave, true);
            auto& resumed = *ownedResumed;
            require(resumed.coopSession && resumed.numHouses == lobby.numHouses,
                "manual selection did not reopen a co-op lobby");
            capture(resumed, "manually-loaded-host-lobby");
        }
        auto ownedCancelled = std::make_unique<CustomGamePlayers>(init, true);
        auto& cancelled = *ownedCancelled;
        cancelled.onLoadCoopSave();
        auto* picker = dynamic_cast<LoadSaveWindow*>(cancelled.pChildWindow);
        require(picker, "cancel test has no save picker");
        picker->onCancel();
        cancelled.processChildWindowOpenCloses();
        require(!cancelled.isQuiting() && !cancelled.selectedCoopSave, "cancel switched the lobby map");
        bool wrongModRejected = false;
        try { coop::makeSavedGameSettings(fixture.string(), "Wrong mod", "unavailable-mod"); }
        catch(const std::exception&) { wrongModRejected = true; }
        require(wrongModRejected, "co-op loader accepted a different mod");
        SDL_Log("COOP SAVE PICKER PASS: %s manual/automatic tabs, frozen launch, cancel, wrong mod and lobby restoration", mod.c_str());
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
            auto ownedLobby = std::make_unique<CustomGamePlayers>(finalInit, false);
            auto& lobby = *ownedLobby;
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
        auto ownedLobby = std::make_unique<CustomGamePlayers>(load, false);
        auto& lobby = *ownedLobby;
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
        bool rejected = false;
        try { coop::makeSavedGameSettings(path, "Co-op only", mod); }
        catch(const std::exception&) { rejected = true; }
        require(rejected, "co-op picker accepted an ordinary multiplayer save");
        GameInitSettings load(getBasename(path, true), bytes, "Ordinary resume check");
        auto ownedLobby = std::make_unique<CustomGamePlayers>(load, false);
        auto& lobby = *ownedLobby;
        require(!lobby.coopSession && lobby.numHouses == static_cast<int>(saved.houses.size()),
            "ordinary multiplayer lobby regression");
        require(!lobby.loadCoopSaveButton.isVisible(), "ordinary lobby gained a co-op save button");
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
        auto ownedLobby = std::make_unique<CustomGamePlayers>(load, false);
        auto& lobby = *ownedLobby;
        require(!lobby.coopSession && lobby.numHouses == static_cast<int>(rows.size())
            && lobby.houseInfo[1].player1DropDown.getSelectedEntryIntData() == -2,
            "legacy closed row cannot reopen in the ordinary lobby");
    }
    SDL_Log("COOP LOBBY PASS: %s, same-faction choices, locked opponents, checkpoint colors, frozen countdown, disconnect cancellation and legacy header", mod.c_str());
}
