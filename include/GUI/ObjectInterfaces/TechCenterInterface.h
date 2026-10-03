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

#ifndef TECHCENTERINTERFACE_H
#define TECHCENTERINTERFACE_H

#include "DefaultStructureInterface.h"

#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <GUI/Label.h>
#include <GUI/ProgressBar.h>
#include <GUI/VBox.h>

#include <House.h>
#include <structures/TechCenter.h>

class TechCenterInterface : public DefaultStructureInterface {
public:
    static TechCenterInterface* create(int objectID) {
        const auto tmp = new TechCenterInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit TechCenterInterface(int objectID) : DefaultStructureInterface(objectID) {
        mainHBox.addWidget(&techCenterVBox);
        techCenterVBox.addWidget(&spawnBox);

        SDL_Texture* pTexture = pGFXManager->getSmallDetailPic(Picture_TechCenter);
        spawnBox.addWidget(&spawnProgressBar, Point((SIDEBARWIDTH - 25 - getWidth(pTexture))/2, 5), getTextureSize(pTexture));
        spawnBox.addWidget(&spawnSelectButton, Point((SIDEBARWIDTH - 25 - getWidth(pTexture))/2, 5), getTextureSize(pTexture));

        sdl2::surface_ptr pText{ pFontManager->createSurfaceWithText(_("READY"), COLOR_WHITE, 12) };
        sdl2::surface_ptr pReady{ SDL_CreateRGBSurface(0, getWidth(pTexture), getHeight(pTexture), SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
        SDL_FillRect(pReady.get(), nullptr, COLOR_TRANSPARENT);

        SDL_Rect dest = calcAlignedDrawingRect(pText.get(), pReady.get());
        SDL_BlitSurface(pText.get(), nullptr, pReady.get(), &dest);

        spawnSelectButton.setTextures(convertSurfaceToTexture(pReady.get()));
        spawnSelectButton.setVisible(false);
        spawnSelectButton.setTooltipText(_("Deploy IX vehicles"));
        spawnSelectButton.setOnClick(std::bind(&TechCenterInterface::onSpawn, this));

        Uint32 color = getHouseColorRGB(getHouseVisualHouse(pLocalHouse->getHouseID()), 3);


        poweredLabel.setTextFontSize(12);
        poweredLabel.setTextColor(color);
        techCenterVBox.addWidget(&poweredLabel, (Sint32)18);

        techCenterVBox.addWidget(Spacer::create(), 0.99);
    }

    bool update() override {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject == nullptr) {
            return false;
        }

        TechCenter* pTechCenter = dynamic_cast<TechCenter*>(pObject);
        if(pTechCenter != nullptr) {
            SDL_Texture* pTexture = pGFXManager->getSmallDetailPic(Picture_TechCenter);
            spawnProgressBar.setTexture(pTexture);
            spawnProgressBar.setProgress(pTechCenter->getPercentComplete());
            spawnSelectButton.setVisible(pTechCenter->canSpawnVehicles());


            const bool powered = pTechCenter->getOwner()->hasPower();
            poweredLabel.setText(std::string(" ") + (powered ? _("Powered") : _("UNPOWERED")));

        }

        return DefaultStructureInterface::update();
    }

private:
    void onSpawn() {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        TechCenter* pTechCenter = dynamic_cast<TechCenter*>(pObject);
        if(pTechCenter != nullptr) {
            pTechCenter->handleSpawnClick();
        }
    }

    VBox                techCenterVBox;
    StaticContainer     spawnBox;
    PictureProgressBar  spawnProgressBar;
    PictureButton       spawnSelectButton;
    Label               poweredLabel;
};

#endif // TECHCENTERINTERFACE_H
