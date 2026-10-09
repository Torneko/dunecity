#include <Menu/CoopCampaignMenu.h>
#include <Menu/CustomGamePlayers.h>
#include <Menu/MainMenuButtonColor.h>
#include <Campaign/CoopCampaignRuntime.h>
#include <Campaign/CoopCampaignSession.h>
#include <FileClasses/GFXManager.h>
#include <GUI/Spacer.h>
#include <GUI/MsgBox.h>
#include <GUI/dune/LoadSaveWindow.h>
#include <misc/IMemoryStream.h>
#include <misc/SaveGameLobbySetup.h>
#include <misc/FileSystem.h>
#include <misc/string_util.h>
#include <misc/fnkdat.h>
#include <mod/ModManager.h>
#include <globals.h>
#include <sand.h>
#include <mmath.h>
#include <filesystem>

CoopCampaignMenu::CoopCampaignMenu(bool LANServer) : LANServer(LANServer) {
    auto* background = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(background);
    resize(getTextureSize(background));
    setWindowWidget(&windowWidget);
    windowWidget.addWidget(&mainVBox, Point(36, 32), Point(getRendererWidth() - 72, getRendererHeight() - 64));
    const bool french = settings.general.language == "fr";
    caption.setText(french ? "Campagne coop commune" : "Common co-op campaign");
    caption.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&caption, 28);
    mainVBox.addWidget(VSpacer::create(20));
    description.setTextFontSize(14);
    description.setText(french
        ? "Deux joueurs, deux bases et deux armées alliées. Chacun choisit sa faction, y compris la même faction.\n\nNeuf étapes suivent la campagne de la faction de l’hôte. Le partenaire reçoit les mêmes unités et un MCV près du chantier. La progression reste séparée du solo.\n\nMod actif : " + ModManager::instance().getActiveModName()
        : "Two players with separate allied bases and armies. Each chooses a faction, including the same faction.\n\nNine stages follow the host faction’s campaign. The partner receives matching units and an MCV near the Construction Yard. Progress remains separate from solo.\n\nActive mod: " + ModManager::instance().getActiveModName());
    mainVBox.addWidget(&description, 0.65);
    chaosMode.setText("Chaos Mode");
    chaosMode.setChecked(false);
    chaosMode.setEnabled(ModManager::instance().getActiveModName() != "vanilla");
    mainVBox.addWidget(&chaosMode, 26);
    mainVBox.addWidget(VSpacer::create(16));
    newCampaignButton.setText(french ? "Nouvelle campagne" : "New campaign");
    MainMenuButtonColor::apply(newCampaignButton);
    newCampaignButton.setOnClick(std::bind(&CoopCampaignMenu::onNewCampaign, this));
    mainVBox.addWidget(&newCampaignButton, 28);
    mainVBox.addWidget(VSpacer::create(10));
    resumeButton.setText(french ? "Reprendre une campagne commune" : "Resume a common campaign");
    MainMenuButtonColor::apply(resumeButton);
    resumeButton.setOnClick(std::bind(&CoopCampaignMenu::onResumeCampaign, this));
    mainVBox.addWidget(&resumeButton, 28);
    mainVBox.addWidget(Spacer::create(), 0.1);
    backButton.setText(french ? "Retour" : "Back");
    MainMenuButtonColor::apply(backButton);
    backButton.setOnClick(std::bind(&CoopCampaignMenu::onBack, this));
    mainVBox.addWidget(&backButton, 26);
}

void CoopCampaignMenu::onNewCampaign() {
    try {
        std::vector<int> roster;
        for(int h = 0; h < NUM_HOUSES; ++h)
            if(isCampaignHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
        if(roster.size() < 5) throw std::runtime_error("The active mod needs at least five available factions.");
        const auto seed = static_cast<Uint32>(getRandomInt());
        const auto sessionId = std::to_string(SDL_GetTicks()) + "-" + std::to_string(seed);
        auto session = coop::CoopCampaignSession::create(sessionId,
            ModManager::instance().getActiveModName(), roster, {roster[0], roster[1]},
            {settings.general.playerName, settings.general.playerName == "Guest" ? "Guest 2" : "Guest"}, seed, chaosMode.isChecked());
        auto options = effectiveGameOptions;
        options.easyMode = false;
        options.immortalHumanPlayer = false;
        options.chaosMode = chaosMode.isChecked();
        auto init = coop::makeGameSettings(session, options, settings.general.playerName + " — Co-op");
        // This temporary lobby field is removed by prepareMap at first commit.
        // Rejoining a saved stage-1 campaign never changes its source faction.
        init.setMapData(init.getFiledata() + "SourcePending=1\n");
        const int result = CustomGamePlayers::hostLobby(init, LANServer);
        if(result != MENU_QUIT_DEFAULT) quit(result);
    } catch(const std::exception& error) {
        openWindow(MsgBox::create(error.what()));
    }
}

void CoopCampaignMenu::onResumeCampaign() {
    char directory[FILENAME_MAX];
    fnkdat("coop/", directory, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
    openWindow(LoadSaveWindow::create(false, settings.general.language == "fr"
        ? "Reprendre la progression coop" : "Resume co-op progress", directory, "ini"));
}

void CoopCampaignMenu::onChildWindowClose(Window* child) {
    auto* load = dynamic_cast<LoadSaveWindow*>(child);
    if(!load || load->getFilename().empty()) return;
    try {
        const auto session = coop::CoopCampaignSession::loadProgress(load->getFilename());
        if(session.isComplete()) throw std::runtime_error(settings.general.language == "fr"
            ? "Cette campagne commune est terminée. Crée une nouvelle campagne."
            : "This common campaign is complete. Create a new campaign.");
        if(session.context().modName != ModManager::instance().getActiveModName())
            throw std::runtime_error(settings.general.language == "fr"
                ? "Active d’abord le mod de cette campagne dans le menu Mods."
                : "First activate this campaign's mod in the Mods menu.");
        if(session.settingsBlob().empty()) throw std::runtime_error("Missing co-op mission settings.");
        IMemoryStream stream(session.settingsBlob().data(), session.settingsBlob().size());
        GameInitSettings saved(stream);
        // A new lobby reassigns human names to the two preserved bases. The
        // stage and the common progress remain those of the saved session.
        auto init = coop::makeGameSettings(session, saved.getGameOptions(), settings.general.playerName + " — Co-op");
        if(!session.checkpointPath().empty()) {
            auto checkpoint = session.checkpointPath();
            if(!std::filesystem::is_regular_file(checkpoint)
               && std::filesystem::is_regular_file(checkpoint + ".bak")) checkpoint += ".bak";
            if(std::filesystem::is_regular_file(checkpoint)) {
                const auto bytes = readCompleteFile(checkpoint);
                const auto setup = readSaveGameLobbySetup(bytes);
                const auto context = coop::readContext(setup.settings.getFiledata());
                const auto& expected = session.context();
                if(!context || context->sessionId != expected.sessionId || context->modName != expected.modName
                   || context->stage != expected.stage || context->completedMask != expected.completedMask
                   || context->seed != expected.seed || context->roster != expected.roster
                   || context->mapLayout != expected.mapLayout || context->sourceFaction != expected.sourceFaction
                   || context->enemyPresent != expected.enemyPresent || context->slots.size() != expected.slots.size())
                    throw std::runtime_error("The co-op checkpoint does not match this campaign's progress.");
                for(std::size_t i = 0; i < context->slots.size(); ++i)
                    if(context->slots[i].house != expected.slots[i].house
                       || context->slots[i].faction != expected.slots[i].faction)
                        throw std::runtime_error("The co-op checkpoint has different faction ownership slots.");
                init = GameInitSettings(getBasename(checkpoint, true), bytes,
                    settings.general.playerName + " — Co-op");
            }
        }
        const int result = CustomGamePlayers::hostLobby(init, LANServer);
        if(result != MENU_QUIT_DEFAULT) quit(result);
    } catch(const std::exception& error) {
        openWindow(MsgBox::create(error.what()));
    }
}

void CoopCampaignMenu::onBack() { quit(); }
