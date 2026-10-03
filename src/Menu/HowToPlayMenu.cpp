/*
 *  This file is part of Dune Legacy Tornie.
 *
 *  Licensed under GPL-2.0-or-later.
 */

#include <Menu/HowToPlayMenu.h>

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
    "Use MODS to choose the bundled Tornie mod or the vanilla RTS.\n"
    "Tornie includes custom units and the Neutral and Rebels houses.\n";

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
    body.setText(kHowToPlayBody);
    body.setAutohideScrollbar(false);
    windowWidget.addWidget(&body,
                           Point(getRendererWidth() / 2 - 320,
                                 getRendererHeight() / 2 - 160),
                           Point(640, 320));

    backButton.setText(_("Back"));
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
