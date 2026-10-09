/*
 *  This file is part of Dune Legacy Tornie.
 *
 *  Licensed under GPL-2.0-or-later.
 */

#include <Menu/HowToPlayMenu.h>
#include <Menu/MainMenuButtonColor.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

// RTS guide for the standalone Tornie edition.
static const char* kHowToPlayBody =
    "DUNE LEGACY TORNIE\n"
    "Build a base, harvest spice, and command your army on Arrakis.\n"
    "\n"
    "1) CHOOSE A GAME\n"
    "Play a campaign or select Single Player -> Custom Game for a\n"
    "skirmish. Multiplayer and the map editor are available too.\n"
    "\n"
    "2) BUILD YOUR ECONOMY\n"
    "Start with a Construction Yard, Wind Traps and a Refinery.\n"
    "Harvesters collect spice; Silos expand your storage.\n"
    "Place structures on rock and keep enough power for your base.\n"
    "\n"
    "3) DEFEND AND EXPAND\n"
    "Build production facilities, train units, and protect your\n"
    "harvesters. Upgrade buildings to unlock more technology.\n"
    "\n"
    "4) TORNIE MOD\n"
    "Use MODS to choose Vanilla, Tornie, Tornie Lite, Jericho,\n"
    "or Jericho Lite.\n"
    "Jericho Lite combines Tornie Lite technology with Jericho spice.\n"
    "The mods include custom units, structures and campaigns.\n"
    "\n5) CO-OP CAMPAIGN\n"
    "Choose Co-op Campaign in the main menu. Each ally owns a base.\n"
    "Connect a guest over LAN/Internet, or choose an AI ally.\n"
    "The host can choose enemy AI and optional allied control.\n"
    "Load a save in the host lobby: Manual or Automatic.\n"
    "After loading, the guest rejoins the new lobby.\n";

static const char* kHowToPlayBodyFrench =
    "DUNE LEGACY TORNIE\n"
    "Construisez une base, récoltez l'épice et dirigez votre armée.\n\n"
    "1) CHOISIR UNE PARTIE\n"
    "Jouez une campagne ou une partie personnalisée en solo.\n"
    "Le multijoueur et l'éditeur de cartes sont aussi disponibles.\n\n"
    "2) DÉVELOPPER VOTRE ÉCONOMIE\n"
    "Construisez un chantier, des éoliennes et une raffinerie.\n"
    "Les harvesters récoltent l'épice ; les silos la stockent.\n"
    "Construisez sur la roche et alimentez votre base en énergie.\n\n"
    "3) DÉFENDRE ET PROGRESSER\n"
    "Produisez des unités, protégez vos harvesters et améliorez\n"
    "vos bâtiments pour débloquer de nouvelles technologies.\n\n"
    "4) MODS\n"
    "Choisissez Vanilla, Tornie, Tornie Lite, Jericho ou Jericho Lite.\n"
    "Jericho Lite utilise les technologies de Tornie Lite et les\n"
    "épices de Jericho. Chaque mod possède ses campagnes.\n\n"
    "5) CAMPAGNE COOP\n"
    "Choisissez Campagne coop dans le menu principal. Chaque allié\n"
    "possède sa base. Connectez un invité en LAN/Internet ou\n"
    "choisissez une IA alliée. L'hôte règle les IA adverses et\n"
    "peut activer les contrôles alliés.\n"
    "Dans le lobby hôte, Charger une sauvegarde donne accès aux\n"
    "sauvegardes manuelles et automatiques. L'invité rejoint\n"
    "ensuite le nouveau lobby.\n";

HowToPlayMenu::HowToPlayMenu() : MenuBase()
{
    SDL_Texture* pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    SDL_Texture* pPlanetBackground = pGFXManager->getUIGraphic(UI_PlanetBackground);
    planetPicture.setTexture(pPlanetBackground);
    SDL_Rect dest1 = calcAlignedDrawingRect(pPlanetBackground);
    dest1.y = dest1.y - getHeight(pPlanetBackground) / 2 + 10;
    windowWidget.addWidget(&planetPicture, dest1);

    SDL_Texture* pDuneLegacy = pGFXManager->getUIGraphic(UI_DuneLegacy);
    duneLegacy.setTexture(pDuneLegacy);
    SDL_Rect dest2 = calcAlignedDrawingRect(pDuneLegacy);
    dest2.y = dest2.y + getHeight(pDuneLegacy) / 2 + 28;
    windowWidget.addWidget(&duneLegacy, dest2);

    title.setText(_("HOW TO PLAY"));
    title.setTextFontSize(20);
    title.setAlignment(Alignment_HCenter);
    windowWidget.addWidget(&title,
                           Point(getRendererWidth() / 2 - 280,
                                 getRendererHeight() / 2 - 200),
                           Point(560, 30));

    body.setTextFontSize(14);
    body.setText(settings.general.language == "fr" ? kHowToPlayBodyFrench : kHowToPlayBody);
    body.setAutohideScrollbar(false);
    windowWidget.addWidget(&body,
                           Point(getRendererWidth() / 2 - 320,
                                 getRendererHeight() / 2 - 160),
                           Point(640, 320));

    backButton.setText(_("Back"));
    MainMenuButtonColor::apply(backButton);
    backButton.setOnClick(std::bind(&HowToPlayMenu::onBack, this));
    windowWidget.addWidget(&backButton,
                           Point(getRendererWidth() / 2 - 60,
                                 getRendererHeight() / 2 + 175),
                           Point(120, 30));
}

HowToPlayMenu::~HowToPlayMenu() = default;

void HowToPlayMenu::onBack() {
    quit();
}
