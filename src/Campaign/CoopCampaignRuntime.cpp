#include <Campaign/CoopCampaignRuntime.h>
#include <Campaign/CoopCampaignTransition.h>
#include <Game.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <GUI/StaticContainer.h>
#include <GUI/Label.h>
#include <GUI/TextButton.h>
#include <Menu/MenuBase.h>
#include <Menu/CustomGameStatsMenu.h>
#include <Network/NetworkManager.h>
#include <misc/FileSystem.h>
#include <misc/IMemoryStream.h>
#include <misc/OMemoryStream.h>
#include <mod/ModManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <globals.h>
#include <main.h>
#include <sand.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>

namespace coop {
namespace {

std::string serializeSettings(const GameInitSettings& settings) {
    OMemoryStream stream;
    settings.save(stream);
    return std::string(stream.getData(), stream.getDataLength());
}

void destroyGame() {
    delete currentGame;
    currentGame = nullptr;
    if(pNetworkManager) {
        // Game's destructor already clears input/chat callbacks. These two
        // performance callbacks also capture Game and must not outlive it.
        pNetworkManager->setOnReceiveClientStats({});
        pNetworkManager->setOnReceiveSetPathBudget({});
    }
    resetHouseVisualHouseMapping();
}

void persist(const CoopCampaignSession& session, const GameInitSettings& settings,
    const std::string& checkpoint = "") {
    try {
        // Templates bind original-campaign foes when settings are prepared.
        // Persist that concrete setup, except for a completed stage-10 record.
        auto bound = session;
        if(!session.isComplete()) {
            bound = CoopCampaignSession::fromMapData(settings.getFiledata());
            if(bound.context().stage != session.context().stage
                || bound.context().sessionId != session.context().sessionId
                || bound.context().mapLayout != session.context().mapLayout
                || bound.context().sourceFaction != session.context().sourceFaction)
                throw std::invalid_argument("Co-op progress does not match prepared settings");
        }
        bound.saveProgress(progressPath(bound.context()), serializeSettings(settings), checkpoint);
    } catch(const std::exception& error) {
        // A full disk must not make one peer leave the deterministic match.
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Co-op progression could not be saved: %s", error.what());
    }
}

bool checkpoint(Game& game, const Context& context) {
    const auto target = std::filesystem::u8path(checkpointPath(context));
    auto temporary = target;
    temporary += ".tmp";
    auto backup = target;
    backup += ".bak";
    std::error_code ignored;
    try {
        std::filesystem::create_directories(target.parent_path());
        if(!game.saveGame(temporary.u8string())) {
            std::filesystem::remove(temporary, ignored);
            return false;
        }
        const bool hadTarget = std::filesystem::exists(target);
        if(hadTarget) {
            std::filesystem::remove(backup, ignored);
            std::filesystem::rename(target, backup);
        }
        try {
            std::filesystem::rename(temporary, target);
        } catch(...) {
            if(hadTarget && !std::filesystem::exists(target))
                std::filesystem::rename(backup, target, ignored);
            throw;
        }
        std::filesystem::remove(backup, ignored);
        return true;
    } catch(const std::exception& error) {
        std::filesystem::remove(temporary, ignored);
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Co-op checkpoint could not be saved: %s", error.what());
        return false;
    }
}

bool sameParticipants(const Context& first, const Context& second) {
    if(first.sessionId != second.sessionId || first.modName != second.modName
        || first.campaign != second.campaign || first.seed != second.seed
        || first.mapLayout != second.mapLayout || first.sourceFaction != second.sourceFaction
        || first.enemyPresent != second.enemyPresent
        || first.slots.size() != second.slots.size()
        || first.roster != second.roster || first.stage != second.stage
        || first.completedMask != second.completedMask
        || first.chaosEligible != second.chaosEligible) return false;
    for(std::size_t i = 0; i < first.slots.size(); ++i) {
        const auto& a = first.slots[i];
        const auto& b = second.slots[i];
        if(a.house != b.house || a.faction != b.faction || a.color != b.color
            || a.playerName != b.playerName || a.playerClass != b.playerClass) return false;
    }
    return true;
}

// MenuBase keeps ENet serviced while either player reads the result. Readiness
// is explicit, and only an authenticated host packet starts the next mission.
class TransitionMenu final : public MenuBase {
public:
    TransitionMenu(CoopCampaignSession session, const GameInitSettings& settings,
        Outcome outcome, std::string partner)
        : session_(std::move(session)), settings_(settings), outcome_(outcome),
          partner_(std::move(partner)), barrier_(session_.context().sessionId,
              session_.context().stage, partner_) {
        auto* background = pGFXManager->getUIGraphic(UI_MenuBackground);
        setBackground(background);
        resize(getTextureSize(background));
        setWindowWidget(&container_);
        const bool french = ::settings.general.language == "fr";
        title_.setAlignment(Alignment_HCenter);
        title_.setText(french ? "Campagne coop commune" : "Common co-op campaign");
        container_.addWidget(&title_, Point(36, 40), Point(getRendererWidth() - 72, 36));
        message_.setAlignment(Alignment_HCenter);
        message_.setTextFontSize(16);
        message_.setText((french ? "Mission " : "Mission ")
            + std::to_string(session_.context().stage) + "/9\n\n"
            + (outcome == Outcome::Won
                ? (french ? "Victoire de votre équipe.\nLes deux joueurs doivent être prêts pour continuer."
                          : "Your team won.\nBoth players must be ready to continue.")
                : (french ? "Votre équipe a perdu.\nLa progression reste à cette mission."
                          : "Your team lost.\nProgress remains at this mission.")));
        container_.addWidget(&message_, Point(36, 110), Point(getRendererWidth() - 72, 160));
        continue_.setText(outcome == Outcome::Won
            ? (french ? "Prêt : continuer" : "Ready: continue")
            : (french ? "Prêt : réessayer" : "Ready: retry"));
        continue_.setOnClick([this] { ready(); });
        container_.addWidget(&continue_, Point(getRendererWidth() / 2 - 150, 300), Point(300, 32));
        cancel_.setText(french ? "Retour au menu" : "Return to menu");
        cancel_.setOnClick([this] { quit(); });
        container_.addWidget(&cancel_, Point(getRendererWidth() / 2 - 150, 360), Point(300, 32));
    }

    void quit(int value = MENU_QUIT_DEFAULT) override {
        if(!resolved_ && pNetworkManager) {
            if(pNetworkManager->isServer())
                pNetworkManager->sendCoopAdvance(session_.context().sessionId,
                    session_.context().stage, static_cast<Uint8>(Advance::Aborted));
            else pNetworkManager->sendCoopReady(session_.context().sessionId,
                session_.context().stage, static_cast<Uint8>(Outcome::Aborted));
        }
        MenuBase::quit(value);
    }

    void update() override {
        if(!pNetworkManager || isQuiting()) return;
        const auto peers = pNetworkManager->getConnectedPeers();
        if(peers.size() != 1 || peers.front() != partner_) { quit(); return; }
        if(pNetworkManager->isServer()) {
            for(const auto& message : pNetworkManager->takeCoopReady()) {
                if(message.epoch != pNetworkManager->getGameEpoch()) continue;
                barrier_.receive(message.playerName, message.sessionId, message.stage,
                    static_cast<Outcome>(message.outcome));
                // Leaving does not wait for the host's Ready button.
                if(message.playerName == partner_ && message.sessionId == session_.context().sessionId
                    && message.stage == session_.context().stage && message.outcome == 0) {
                    quit(); return;
                }
            }
            if(!ready_) return;
            const auto action = barrier_.decision(outcome_, StageCount);
            if(action) resolveHost(*action);
        } else {
            for(const auto& message : pNetworkManager->takeCoopAdvance()) {
                if(message.epoch != pNetworkManager->getGameEpoch()) continue;
                if(message.sessionId != session_.context().sessionId
                    || message.stage != session_.context().stage) continue;
                const auto action = static_cast<Advance>(message.action);
                if(!barrier_.accepts(action, outcome_, StageCount)
                    || (action != Advance::Aborted && !ready_)) { quit(); return; }
                if(action == Advance::Aborted) { resolved_ = true; MenuBase::quit(); return; }
                session_.completeMission(outcome_ == Outcome::Won);
                if(action == Advance::Complete) {
                    persist(session_, settings_);
                    resolved_ = true;
                    complete_ = true;
                    MenuBase::quit(1);
                    return;
                }
                try {
                    IMemoryStream stream(message.settingsBlob.data(), static_cast<int>(message.settingsBlob.size()));
                    GameInitSettings next(stream);
                    const auto context = readContext(next.getFiledata());
                    const auto expected = readContext(session_.prepareMap(readMissionTemplate(session_)));
                    if(next.getGameType() != GameType::CustomMultiplayer || !context
                        || !expected || !sameParticipants(*expected, *context)
                        || next.getModName() != settings_.getModName()) {
                        quit(); return;
                    }
                    // Verify the transmitted source marker as well as its context.
                    session_ = CoopCampaignSession::fromMapData(session_.prepareMap(next.getFiledata()));
                    next_ = std::move(next);
                    persist(session_, *next_);
                    resolved_ = true;
                    MenuBase::quit(1);
                    return;
                } catch(const std::exception& error) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Invalid co-op transition: %s", error.what());
                    quit(); return;
                }
            }
        }
    }

    const std::optional<GameInitSettings>& next() const { return next_; }
    bool complete() const { return complete_; }

private:
    void ready() {
        if(ready_) return;
        ready_ = true;
        continue_.setEnabled(false);
        continue_.setText(::settings.general.language == "fr"
            ? "En attente de ton partenaire…" : "Waiting for your partner…");
        if(!pNetworkManager->isServer())
            pNetworkManager->sendCoopReady(session_.context().sessionId,
                session_.context().stage, static_cast<Uint8>(outcome_));
    }

    void resolveHost(Advance action) {
        const auto previous = session_.context();
        if(action == Advance::Aborted) { quit(); return; }
        session_.completeMission(outcome_ == Outcome::Won);
        if(action == Advance::Complete) {
            persist(session_, settings_);
            pNetworkManager->sendCoopAdvance(previous.sessionId, previous.stage,
                static_cast<Uint8>(action));
            complete_ = true;
        } else {
            next_ = makeGameSettings(session_, settings_.getGameOptions(), settings_.getServername());
            session_ = CoopCampaignSession::fromMapData(next_->getFiledata());
            persist(session_, *next_);
            pNetworkManager->sendCoopAdvance(previous.sessionId, previous.stage,
                static_cast<Uint8>(action), serializeSettings(*next_));
        }
        resolved_ = true;
        MenuBase::quit(1);
    }

    CoopCampaignSession session_;
    GameInitSettings settings_;
    Outcome outcome_;
    std::string partner_;
    MissionBarrier barrier_;
    bool ready_ = false, resolved_ = false, complete_ = false;
    std::optional<GameInitSettings> next_;
    StaticContainer container_;
    Label title_, message_;
    TextButton continue_, cancel_;
};

class CompletionMenu final : public MenuBase {
public:
    CompletionMenu() {
        auto* background = pGFXManager->getUIGraphic(UI_MenuBackground);
        setBackground(background);
        resize(getTextureSize(background));
        setWindowWidget(&container_);
        const bool french = settings.general.language == "fr";
        message_.setAlignment(Alignment_HCenter);
        message_.setText(french ? "Campagne coop terminée !\n\nLes neuf missions de votre équipe sont sauvegardées."
            : "Co-op campaign complete!\n\nYour team's nine missions have been saved.");
        container_.addWidget(&message_, Point(36, 130), Point(getRendererWidth() - 72, 160));
        back_.setText(french ? "Retour au menu" : "Return to menu");
        back_.setOnClick([this] { quit(); });
        container_.addWidget(&back_, Point(getRendererWidth()/2 - 150, 330), Point(300, 32));
    }
private:
    StaticContainer container_;
    Label message_;
    TextButton back_;
};
} // namespace

std::string readMissionTemplate(const CoopCampaignSession& session) {
    const auto& context = session.context();
    const std::string suffix = "/campaign/coop/" + session.missionFilename();
    std::vector<std::string> candidates;
    if(context.modName != "vanilla") {
        if(ModManager::instance().isInitialized())
            candidates.push_back(ModManager::instance().getModPath(context.modName) + suffix);
        const auto root = getDuneLegacyDataDir();
        for(const std::string prefix : {"", "/..", "/../..", "/../../.."})
            candidates.push_back(root + prefix + "/mods/" + context.modName + suffix);
    } else {
        for(const auto& root : FileManager::getSearchPath())
            candidates.push_back(root + "/coop/" + session.missionFilename());
        candidates.push_back(getDuneLegacyDataDir() + "/coop/" + session.missionFilename());
    }
    for(const auto& path : candidates) {
        std::ifstream input(std::filesystem::u8path(path), std::ios::binary);
        if(input) return std::string(std::istreambuf_iterator<char>(input), {});
    }
    throw std::runtime_error("Missing co-op mission template: " + context.modName + "/" + session.missionFilename());
}

GameInitSettings makeGameSettings(const CoopCampaignSession& session,
    const SettingsClass::GameOptionsClass& requestedOptions, const std::string& serverName) {
    auto options = requestedOptions;
    options.easyMode = false;
    options.immortalHumanPlayer = false;
    const auto mapData = session.prepareMap(readMissionTemplate(session));
    const auto prepared = readContext(mapData);
    if(!prepared) throw std::runtime_error("Missing prepared co-op context");
    GameInitSettings settings(session.missionFilename(), mapData, serverName, false, options);
    settings.setRandomSeed(session.context().seed ^ (0x9e3779b9u * static_cast<Uint32>(session.context().stage)));
    std::array<bool, NUM_HOUSE_COLOR_SLOTS> usedColors{};
    for(const auto& slot : prepared->slots) {
        GameInitSettings::HouseInfo house(static_cast<HOUSETYPE>(slot.house), slot.team());
        int color = isValidHouseColorSlot(slot.color) ? slot.color
            : getDefaultHouseColorSlot(static_cast<HOUSETYPE>(slot.faction));
        if(!isValidHouseColorSlot(color) || usedColors[color]) {
            color = 0;
            while(color < NUM_HOUSE_COLOR_SLOTS && usedColors[color]) ++color;
        }
        house.colorOfHouse = color;
        usedColors[color] = true;
        house.addPlayerInfo(GameInitSettings::PlayerInfo(slot.playerName, slot.playerClass));
        settings.addHouseInfo(house);
    }
    return settings;
}

std::string progressPath(const Context& context) {
    // Context validation restricts sessionId to a filename-safe identifier.
    return getDirname(getConfigFilepath()) + "/coop/" + context.sessionId + ".ini";
}

std::string checkpointPath(const Context& context) {
    return getDirname(getConfigFilepath()) + "/coop/" + context.sessionId + ".dls";
}

void runMultiplayerSession(const GameInitSettings& initialSettings) {
    GameInitSettings settings = initialSettings;
    if(pNetworkManager) pNetworkManager->clearCoopMessages();
    Uint32 missionEpoch = 1;
    try {
        while(true) {
            const auto requestedParticipants = settings.getHouseInfoList();
            const bool multiplayerResume = settings.getGameType() == GameType::LoadMultiplayer;
            currentGame = new Game();
            currentGame->initGame(settings);
            settings = currentGame->getGameInitSettings();
            const auto context = readContext(settings.getFiledata());
            if(pNetworkManager) pNetworkManager->setGameEpoch(context ? missionEpoch : 0);
            std::optional<CoopCampaignSession> session;
            if(context) {
                session = CoopCampaignSession::fromMapData(settings.getFiledata());
                if(multiplayerResume) {
                    std::array<std::string, 2> names{context->slots[0].playerName, context->slots[1].playerName};
                    for(int i = 0; i < 2; ++i)
                        for(const auto& house : requestedParticipants)
                            if(house.houseID == context->slots[i].house)
                                for(const auto& player : house.playerInfoList)
                                    if(player.playerClass == HUMANPLAYERCLASS) names[i] = player.playerName;
                    // Reconfigure both names atomically: swapping Host/Guest
                    // must not trip the single-name duplicate check.
                    session->reconfigurePlayers({context->slots[0].faction, context->slots[1].faction}, names);
                    for(int i = 0; i < 2; ++i) session->setPlayerColor(i, context->slots[i].color);
                    auto participants = settings.getHouseInfoList();
                    for(auto& house : participants)
                        for(int i = 0; i < 2; ++i)
                            if(house.houseID == context->slots[i].house)
                                for(auto& player : house.playerInfoList)
                                    if(player.playerClass == HUMANPLAYERCLASS) player.playerName = names[i];
                    currentGame->updateCoopParticipants(session->prepareMap(settings.getFiledata()), participants);
                    settings = currentGame->getGameInitSettings();
                }
                if(checkpoint(*currentGame, *context))
                    persist(*session, settings, checkpointPath(*context));
                else persist(*session, settings);
            }
            currentGame->runMainLoop();
            const auto outcome = currentGame->hasFinished()
                ? (currentGame->hasWon() ? Outcome::Won : Outcome::Lost) : Outcome::Aborted;
            if(!context) {
                if(currentGame->whatNext() == GAME_CUSTOM_GAME_STATS) {
                    CustomGameStatsMenu stats;
                    stats.showMenu();
                }
                destroyGame();
                return;
            }
            if(outcome == Outcome::Aborted) {
                if(checkpoint(*currentGame, *context))
                    persist(*session, settings, checkpointPath(*context));
                if(pNetworkManager) {
                    if(pNetworkManager->isServer()) pNetworkManager->sendCoopAdvance(context->sessionId,
                        context->stage, static_cast<Uint8>(Advance::Aborted));
                    else pNetworkManager->sendCoopReady(context->sessionId, context->stage,
                        static_cast<Uint8>(Outcome::Aborted));
                }
                destroyGame();
                return;
            }
            {
                CustomGameStatsMenu stats;
                stats.showMenu();
            }
            destroyGame();
            if(!pNetworkManager) return;
            const auto peers = pNetworkManager->getConnectedPeers();
            if(peers.size() != 1) return;
            TransitionMenu transition(*session, settings, outcome, peers.front());
            transition.showMenu();
            if(transition.complete()) {
                CompletionMenu completion;
                completion.showMenu();
                return;
            }
            if(!transition.next()) return;
            settings = *transition.next();
            ++missionEpoch;
        }
    } catch(...) {
        destroyGame();
        throw;
    }
}
} // namespace coop
