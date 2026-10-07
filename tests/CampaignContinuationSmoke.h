#pragma once

#include <GameInitSettings.h>
#include <Game.h>
#include <House.h>
#include <misc/OFileStream.h>
#include <players/PlayerFactory.h>
#include <sand.h>
#include <filesystem>
#include <stdexcept>

inline void verifyCampaignContinuation(const std::string& output, const std::string& mod) {
    if(mod != "Tornie" && mod != "Jericho") return;
    auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("Campaign continuation check: " + message);
    };
    const auto playerHouse = getRuntimeHouseForIdentity(HOUSE_WILDSPADE);
    const auto firstEnemy = getRuntimeHouseForIdentity(mod == "Tornie" ? HOUSE_ATREIDES : HOUSE_ORDOS);
    const auto secondEnemy = getRuntimeHouseForIdentity(mod == "Tornie" ? HOUSE_KLESHMERSH : HOUSE_THARPIQUE);
    const auto finalEnemy = getRuntimeHouseForIdentity(mod == "Tornie" ? HOUSE_SARDAUKAR : HOUSE_NEUTRAL);
    auto options = effectiveGameOptions;
    options.easyMode = false;
    options.chaosMode = false;
    GameInitSettings legacy(playerHouse, options);
    legacy.setMultiplePlayersPerHouse(true);
    GameInitSettings::HouseInfo human(playerHouse, 1);
    human.colorOfHouse = 0;
    human.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS));
    human.addPlayerInfo(GameInitSettings::PlayerInfo("Campaign Support", "qBotSupportMedium"));
    legacy.addHouseInfo(human);
    for(const auto enemy : {firstEnemy, secondEnemy}) {
        GameInitSettings::HouseInfo opponent(enemy, 7);
        opponent.addPlayerInfo(GameInitSettings::PlayerInfo("Existing Campaign AI " + std::to_string(enemy),
            enemy == firstEnemy ? "qBotHard" : "qBotMedium"));
        legacy.addHouseInfo(opponent);
    }
    const GameInitSettings next(legacy, 22, 0x12, 0x34);
    require(legacy.getHouseInfoList().size() == 3, "previous saved settings were changed");
    require(next.getGameType() == GameType::Campaign && next.getMission() == 22
            && next.getFilename() == "SCENW022.INI", "wrong final mission or faction alias");
    require(next.getAlreadyPlayedRegions() == 0x12 && next.getAlreadyShownTutorialHints() == 0x34,
            "region/tutorial progress was changed");
    require(next.isMultiplePlayersPerHouse(), "support player mode was removed");
    require(next.getHouseInfoList().size() == static_cast<size_t>(getNumCustomGameHouses()),
            "new campaign factions are missing");
    for(size_t i = 0; i < legacy.getHouseInfoList().size(); ++i) {
        const auto& before = legacy.getHouseInfoList()[i];
        const auto& after = next.getHouseInfoList()[i];
        require(before.houseID == after.houseID && before.team == after.team
                && before.colorOfHouse == after.colorOfHouse
                && before.playerInfoList.size() == after.playerInfoList.size(),
                "existing house setup was changed");
        for(size_t p = 0; p < before.playerInfoList.size(); ++p)
            require(before.playerInfoList[p].playerName == after.playerInfoList[p].playerName
                    && before.playerInfoList[p].playerClass == after.playerInfoList[p].playerClass,
                    "human, support or chosen enemy AI was changed");
    }
    for(size_t i = legacy.getHouseInfoList().size(); i < next.getHouseInfoList().size(); ++i) {
        const auto& added = next.getHouseInfoList()[i];
        require(added.team == 7 && added.playerInfoList.size() == 1
                && added.playerInfoList.front().playerClass == "qBotHard",
                "new opponent did not inherit the chosen enemy team and AI");
    }
    const GameInitSettings repeated(next, 22, 0x12, 0x34);
    require(repeated.getHouseInfoList().size() == next.getHouseInfoList().size(),
            "repeating the mission duplicated opponents");
    GameInitSettings skirmish(playerHouse, 1, options);
    skirmish.addHouseInfo(human);
    const GameInitSettings nonCampaign(skirmish, 22, 0, 0);
    require(nonCampaign.getHouseInfoList().size() == 1,
            "noncampaign player setup was expanded");
    GameInitSettings noEnemy(playerHouse, options);
    noEnemy.addHouseInfo(human);
    const GameInitSettings fallback(noEnemy, 22, 0, 0);
    for(size_t i = 1; i < fallback.getHouseInfoList().size(); ++i)
        require(fallback.getHouseInfoList()[i].team == 2
                && fallback.getHouseInfoList()[i].playerInfoList.front().playerClass == DEFAULTAIPLAYERCLASS,
                "player support was mistaken for an enemy AI");

    // A replay with no commands loads the real Campaign scenario and bypasses
    // the interactive briefing, allowing this isolated check to run unattended.
    const auto replayPath = (std::filesystem::path(output) / (mod + "-wildspade-final.rpl")).string();
    {
        OFileStream replay;
        require(replay.open(replayPath), "cannot write isolated replay fixture");
        replay.writeString(settings.general.playerName);
        next.save(replay);
    }
    {
        auto game = std::make_unique<Game>();
        currentGame = game.get();
        game->initReplay(replayPath);
        require(pLocalHouse != nullptr && pLocalHouse->getHouseID() == playerHouse,
                "finale lost the human faction");
        for(const auto enemy : {firstEnemy, secondEnemy, finalEnemy}) {
            House* house = game->getHouse(enemy);
            require(house != nullptr && house->getTeamID() == 7 && house->getCredits() > 0,
                    "enemy was created without its starting setup");
            require(house->getNumStructures() > 0 && house->getNumUnits() > 0,
                    "enemy has no deployed base or army");
            require(house->getPlayerList().size() == 1, "enemy has no AI controller");
            require(house->getPlayerList().front()->getPlayerclass()
                    == (enemy == secondEnemy ? "qBotMedium" : "qBotHard"),
                    "runtime enemy has the wrong AI difficulty");
        }
        require(game->getHouse(finalEnemy)->getCredits() == 2500,
                "third opponent lost its scenario credits");
        game->processObjects();
    }
    currentGame = nullptr;
    pLocalHouse = nullptr;
    pLocalPlayer = nullptr;
    SDL_Log("CAMPAIGN CONTINUATION PASS: %s Wildspade finale, three active enemy bases/armies, legacy setup expansion and preserved AI/support", mod.c_str());
}
