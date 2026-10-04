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

#include <structures/Palace.h>
#include <Achievements/AchievementEvents.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Bullet.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>
#include <mod/ModManager.h>

#include <units/InfantryBase.h>
#include <units/UnitBase.h>
#include <units/Trooper.h>
#include <units/Saboteur.h>

#include <GUI/ObjectInterfaces/PalaceInterface.h>

#define PALACE_DEATHHAND_WEAPONDAMAGE       100

Palace::Palace(House* newOwner) : StructureBase(newOwner) {
    Palace::init();

    setHealth(getMaxHealth());
    specialWeaponTimer = getMaxSpecialWeaponTimer();

    // TODO: Special weapon is available immediately but AI uses it only after first visual contact
    //specialTimer = 1; // we want the special weapon to be immediately ready
}

Palace::Palace(InputStream& stream) : StructureBase(stream) {
    Palace::init();

    specialWeaponTimer = stream.readSint32();


}

void Palace::init() {
    itemID = Structure_Palace;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 3;

    graphicID = ObjPic_Palace;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 4;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;

    canAttackStuff = true;
}

Palace::~Palace() = default;

void Palace::setOriginalHouseID(int houseID) {
    if(houseID != originalHouseID) {
        StructureBase::setOriginalHouseID(houseID);
        specialWeaponTimer = getMaxSpecialWeaponTimer();
    }
}

void Palace::save(OutputStream& stream) const {
    StructureBase::save(stream);
    stream.writeSint32(specialWeaponTimer);
}

ObjectInterface* Palace::getInterfaceContainer() {
    if((pLocalHouse == owner) || (debug == true)) {
        return PalaceInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

void Palace::handleSpecialClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PALACE_SPECIALWEAPON,objectID));
}

void Palace::handleDeathhandClick(int xPos, int yPos) {
    if (currentGameMap->tileExists(xPos, yPos)) {
        currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PALACE_DEATHHAND,objectID, (Uint32) xPos, (Uint32) yPos));
    }
}

bool Palace::usesTornieMainRebelsCooldown() const {
    if(!ModManager::instance().isInitialized()) return false;
    const std::string activeMod = ModManager::instance().getActiveModName();

    const HOUSETYPE faction =
        getHouseFactionIdentity(static_cast<HOUSETYPE>(originalHouseID));
    if(activeMod == "vanilla") {
        return faction == HOUSE_NEUTRAL || faction == HOUSE_REBELS;
    }

    return (activeMod == "Tornie" || activeMod == "Jericho")
        && faction == HOUSE_REBELS;
}

bool Palace::usesTornieMainRebelsRandomSpecial() const {
    return usesTornieMainRebelsCooldown();
}

Palace::TornieRebelsSpecialWeapon Palace::getTornieMainRebelsSpecialWeapon() const {
    if(!usesTornieMainRebelsRandomSpecial() || specialWeaponTimer >= 0) {
        return TornieRebelsSpecialWeapon::None;
    }

    const Sint32 encodedWeapon = -specialWeaponTimer;
    if(encodedWeapon < static_cast<Sint32>(TornieRebelsSpecialWeapon::Missile)
       || encodedWeapon > static_cast<Sint32>(TornieRebelsSpecialWeapon::Ornithopters)) {
        return TornieRebelsSpecialWeapon::None;
    }

    return static_cast<TornieRebelsSpecialWeapon>(encodedWeapon);
}

bool Palace::usesTargetedSpecialWeapon() const {
    if(usesTornieMainRebelsRandomSpecial()) {
        return getTornieMainRebelsSpecialWeapon() == TornieRebelsSpecialWeapon::Missile;
    }

    const HOUSETYPE palaceHouse = getHouseFallbackHouse(static_cast<HOUSETYPE>(originalHouseID));
    return palaceHouse == HOUSE_HARKONNEN || palaceHouse == HOUSE_SARDAUKAR;
}

void Palace::selectTornieMainRebelsSpecialWeapon() {
    const bool vanillaRandom = ModManager::instance().isInitialized()
        && ModManager::instance().getActiveModName() == "vanilla";
    const TornieRebelsSpecialWeapon lastWeapon = vanillaRandom
        ? TornieRebelsSpecialWeapon::Saboteur
        : TornieRebelsSpecialWeapon::Ornithopters;
    const Sint32 selectedWeapon = currentGame->randomGen.rand(
        static_cast<Sint32>(TornieRebelsSpecialWeapon::Missile),
        static_cast<Sint32>(lastWeapon));
    specialWeaponTimer = -selectedWeapon;
}

bool Palace::usesJerichoOrnithopterStrike() const {
    return ModManager::instance().isInitialized()
        && isHouseFaction(static_cast<HOUSETYPE>(originalHouseID), HOUSE_WILDSPADE);
}

bool Palace::usesJerichoKleshmershFremenCall() const {
    return ModManager::instance().isInitialized()
        && isHouseFaction(static_cast<HOUSETYPE>(originalHouseID), HOUSE_KLESHMERSH);
}

bool Palace::usesLightVehicleCall() const {
    const HOUSETYPE originalHouse = static_cast<HOUSETYPE>(originalHouseID);

    if(isHouseFaction(originalHouse, HOUSE_THARPIQUE)) {
        return true;
    }
    if(usesJerichoKleshmershFremenCall()) {
        return false;
    }
    if(usesJerichoOrnithopterStrike()) {
        return false;
    }

    const HOUSETYPE fallbackHouse = getHouseFallbackHouse(originalHouse);
    return fallbackHouse == HOUSE_NEUTRAL || fallbackHouse == HOUSE_REBELS;
}

void Palace::doSpecialWeapon() {
    if(!isSpecialWeaponReady()) {
        return;
    }

    const HOUSETYPE originalHouse = static_cast<HOUSETYPE>(originalHouseID);
    if(usesTornieMainRebelsRandomSpecial()) {
        bool activated = false;
        switch(getTornieMainRebelsSpecialWeapon()) {
            case TornieRebelsSpecialWeapon::Missile:
                // The missile is launched through doLaunchDeathhand after selecting a target.
                return;

            case TornieRebelsSpecialWeapon::Fremen:
                activated = callFremen();
                break;

            case TornieRebelsSpecialWeapon::Saboteur:
                activated = spawnSaboteur();
                break;

            case TornieRebelsSpecialWeapon::LightVehicles:
                activated = callLightVehicles();
                break;

            case TornieRebelsSpecialWeapon::Ornithopters:
                activated = callOrnithopterStrike();
                break;

            case TornieRebelsSpecialWeapon::None:
            default:
                return;
        }

        if(activated) {
            specialWeaponTimer = getMaxSpecialWeaponTimer();
            AchievementEvents::palace(getOwner());
        }
        return;
    }

    if(usesJerichoOrnithopterStrike()) {
        if(callOrnithopterStrike()) {
            specialWeaponTimer = getMaxSpecialWeaponTimer();
            AchievementEvents::palace(getOwner());
        }
        return;
    }

    if(usesJerichoKleshmershFremenCall()) {
        if(callFremen()) {
            specialWeaponTimer = getMaxSpecialWeaponTimer();
            AchievementEvents::palace(getOwner());
        }
        return;
    }

    if(usesLightVehicleCall()) {
        if(callLightVehicles()) {
            specialWeaponTimer = getMaxSpecialWeaponTimer();
            AchievementEvents::palace(getOwner());
        }
        return;
    }

    switch (getHouseFallbackHouse(originalHouse)) {
        case HOUSE_HARKONNEN:
        case HOUSE_SARDAUKAR: {
            // wrong house (see DoLaunchDeathhand)
            return;
        } break;

        case HOUSE_ATREIDES:
        case HOUSE_FREMEN: {
            if(callFremen()) {
                specialWeaponTimer = getMaxSpecialWeaponTimer();
                AchievementEvents::palace(getOwner());
            }
        } break;

        case HOUSE_ORDOS:
        case HOUSE_MERCENARY: {
            if(spawnSaboteur()) {
                specialWeaponTimer = getMaxSpecialWeaponTimer();
                AchievementEvents::palace(getOwner());
            }
        } break;

        case HOUSE_NEUTRAL:
        case HOUSE_REBELS: {
            if(callLightVehicles()) {
                specialWeaponTimer = getMaxSpecialWeaponTimer();
                AchievementEvents::palace(getOwner());
            }
        } break;

        default: {
            SDL_Log("PALACE: Ignoring special weapon for unsupported house %d", originalHouseID);
            return;
        } break;
    }
}

void Palace::doLaunchDeathhand(int x, int y) {
    if(!isSpecialWeaponReady()) {
        return;
    }

    if(!usesTargetedSpecialWeapon()) {
        // This command is valid for Harkonnen/Sardaukar or Tornie's revealed missile.
        return;
    }

    // Dynasty scatter algorithm: biased towards smaller scatter values
    // Get random 0-255, then repeatedly halve until <= 160
    int scatterDistance = currentGame->randomGen.rand(0, 255);
    while (scatterDistance > 160) {
        scatterDistance /= 2;
    }
    // Convert to pixels (160 Dynasty units = 10 tiles = 320 pixels)
    int radius = scatterDistance * 2;

    FixPoint randAngle = 2 * FixPt_PI * currentGame->randomGen.randFixPoint();
    int deathOffX = lround(FixPoint::sin(randAngle) * radius);
    int deathOffY = lround(FixPoint::cos(randAngle) * radius);

    Coord centerPoint = getCenterPoint();
    Coord dest( x * TILESIZE + TILESIZE/2 + deathOffX,
                y * TILESIZE + TILESIZE/2 + deathOffY);

    bulletList.push_back(new Bullet(objectID, &centerPoint, &dest, Bullet_LargeRocket, PALACE_DEATHHAND_WEAPONDAMAGE, false, nullptr));
    soundPlayer->playSoundAt(Sound_Rocket, getLocation());

    if(getOwner() != pLocalHouse) {
        currentGame->addToNewsTicker(_("@DUNE.ENG|81#Missile is approaching"));
        soundPlayer->playVoice(MissileApproaching, pLocalHouse->getHouseID());
    }

    specialWeaponTimer = getMaxSpecialWeaponTimer();
    AchievementEvents::palace(getOwner());

}

void Palace::updateStructureSpecificStuff() {
    bool becameReady = false;

    if(specialWeaponTimer > 0) {
        --specialWeaponTimer;
        becameReady = specialWeaponTimer <= 0;
    } else if(specialWeaponTimer == 0 && usesTornieMainRebelsRandomSpecial()) {
        // Upgrade an already-ready Palace from an older save to the random Tornie ability.
        becameReady = true;
    }

    if(!becameReady) {
        return;
    }

    if(usesTornieMainRebelsRandomSpecial()) {
        selectTornieMainRebelsSpecialWeapon();
    } else {
        specialWeaponTimer = 0;
    }

    if(getOwner() == pLocalHouse) {
        currentGame->addToNewsTicker(_("Palace is ready"));
    } else if(getOwner()->isAI() && !usesTargetedSpecialWeapon()) {
        doSpecialWeapon();
    }
}

bool Palace::callFremen() {
    int count = 0;
    int x;
    int y;
    do {
        x = currentGame->randomGen.rand(1, currentGameMap->getSizeX()-2);
        y = currentGame->randomGen.rand(1, currentGameMap->getSizeY()-2);
    } while((currentGameMap->getTile(x-1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y)->hasAGroundObject()
            || currentGameMap->getTile(x, y)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y+1)->hasAGroundObject())
            && (count++ <= 1000));

    if(count < 1000) {

        for(int numFremen = 0; numFremen < 15; numFremen++) {
            if(currentGame->randomGen.rand(0, 5) == 0) {
                continue;
            }

            Trooper *pFremen = static_cast<Trooper*>(getOwner()->createUnit(Unit_Trooper, false, getProductionHouseID()));

            int i;
            int j;
            do {
                i = currentGame->randomGen.rand(-1, 1);
                j = currentGame->randomGen.rand(-1, 1);
            } while (!currentGameMap->getTile(x + i, y + j)->infantryNotFull());

            pFremen->deploy(Coord(x + i,y + j));

            pFremen->doSetAttackMode(HUNT);
            pFremen->setRespondable(false);

            const StructureBase* closestStructure = pFremen->findClosestTargetStructure();
            if(closestStructure) {
                Coord closestPoint = closestStructure->getClosestPoint(pFremen->getLocation());
                pFremen->setGuardPoint(closestPoint);
                pFremen->setDestination(closestPoint);
            } else {
                const UnitBase* closestUnit = pFremen->findClosestTargetUnit();
                if(closestUnit) {
                    pFremen->setGuardPoint(closestUnit->getLocation());
                    pFremen->setDestination(closestUnit->getLocation());
                }
            }
        }

        return true;
    } else {
        if(getOwner() == pLocalHouse) {
            currentGame->addToNewsTicker(_("Unable to spawn Fremen"));
        }

        return false;
    }
}

bool Palace::spawnSaboteur() {
    Saboteur* saboteur = static_cast<Saboteur*>(getOwner()->createUnit(Unit_Saboteur, false, getProductionHouseID()));
    Coord spot = currentGameMap->findDeploySpot(saboteur, getLocation(), currentGame->randomGen, getDestination(), getStructureSize());

    saboteur->deploy(spot);

    if(getOwner()->isAI()) {
        SDL_Log("PALACE: Spawning AI saboteur at (%d,%d), setting to HUNT mode", spot.x, spot.y);
        saboteur->doSetAttackMode(HUNT);
        SDL_Log("PALACE: Saboteur attack mode after setting: %d", saboteur->getAttackMode());
        currentGame->addToNewsTicker(_("@DUNE.ENG|79#Saboteur is approaching"));
        soundPlayer->playVoice(SaboteurApproaching, pLocalHouse->getHouseID());
    }

    return true;
}

bool Palace::callLightVehicles() {
    int count = 0;
    int x;
    int y;
    do {
        x = currentGame->randomGen.rand(1, currentGameMap->getSizeX()-2);
        y = currentGame->randomGen.rand(1, currentGameMap->getSizeY()-2);
    } while((currentGameMap->getTile(x-1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y)->hasAGroundObject()
            || currentGameMap->getTile(x, y)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y+1)->hasAGroundObject())
            && (count++ <= 1000));

    if(count >= 1000) {
        if(getOwner() == pLocalHouse) {
            currentGame->addToNewsTicker(_("Unable to spawn vehicles"));
        }
        return false;
    }

    const int trikeCount = currentGame->randomGen.rand(1, 3);
    const int quadCount = currentGame->randomGen.rand(0, 2);
    int spawned = 0;

    const auto spawnVehicle = [&](int itemID) {
        UnitBase* newUnit = getOwner()->createUnit(itemID, false, getProductionHouseID());
        if(newUnit == nullptr) {
            return;
        }

        for(int tries = 0; tries < 30; ++tries) {
            const int i = currentGame->randomGen.rand(-1, 1);
            const int j = currentGame->randomGen.rand(-1, 1);
            const Coord deployPos(x + i, y + j);

            if(currentGameMap->getTile(deployPos)->hasAGroundObject()) {
                continue;
            }
            if(!newUnit->canPass(deployPos.x, deployPos.y)) {
                continue;
            }

            newUnit->deploy(deployPos);
            newUnit->setGuardPoint(deployPos);
            newUnit->doSetAttackMode(HUNT);
            spawned++;
            return;
        }

        delete newUnit;
    };

    for(int i = 0; i < trikeCount; ++i) {
        spawnVehicle(Unit_Trike);
    }
    for(int i = 0; i < quadCount; ++i) {
        spawnVehicle(Unit_Quad);
    }

    if(spawned <= 0 && getOwner() == pLocalHouse) {
        currentGame->addToNewsTicker(_("Unable to spawn vehicles"));
    }

    return spawned > 0;
}
bool Palace::callOrnithopterStrike() {
    int spawned = 0;

    for(int i = 0; i < 3; ++i) {
        UnitBase* ornithopter = getOwner()->createUnit(Unit_Ornithopter, false, getProductionHouseID());
        if(ornithopter == nullptr) {
            continue;
        }

        const Coord deployPos = currentGameMap->findDeploySpot(
            ornithopter, getLocation(), currentGame->randomGen, getDestination(), getStructureSize());
        if(!deployPos.isValid()) {
            delete ornithopter;
            continue;
        }

        ornithopter->deploy(deployPos);
        ornithopter->setGuardPoint(deployPos);
        ornithopter->doSetAttackMode(HUNT);
        ++spawned;
    }

    if(getOwner() == pLocalHouse) {
        if(spawned == 3) {
            currentGame->addToNewsTicker(_("Three Ornithopters deployed in hunt mode"));
        } else {
            currentGame->addToNewsTicker(_("Unable to deploy all three Ornithopters"));
        }
    }

    return spawned > 0;
}
