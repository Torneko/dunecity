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

#include <structures/Worfinery.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <units/TrackedUnit.h>
#include <units/UnitBase.h>
#include <units/GroundUnit.h>
#include <units/Carryall.h>
#include <units/HarvesterHelpers.h>

namespace {
const FixPoint MaximumHarvesterExtractionSpeed = 0.625_fix;
constexpr int WorfineryIdleFrame = 2;
constexpr int WorfineryUnloadingFrame = 3;
}

Worfinery::Worfinery(House* newOwner)
    : BuilderBase(newOwner), extractingSpice(false), bookings(0) {
    Worfinery::init();

    setHealth(getMaxHealth());
}

Worfinery::Worfinery(InputStream& stream)
    : BuilderBase(stream), extractingSpice(false), bookings(0) {
    Worfinery::init();

    extractingSpice = stream.readBool();
    harvester.load(stream);
    bookings = stream.readUint32();

    if(extractingSpice) {
        showUnloadingFrame();
    } else {
        showIdleFrame();
    }
}

void Worfinery::init() {
    itemID = Structure_Worfinery;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 2;

    graphicID = ObjPic_Worfinery;
    graphic = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());

    numImagesX = 4;
    numImagesY = 1;
    showIdleFrame();
    lastVisibleFrame = WorfineryIdleFrame;
}

Worfinery::~Worfinery() {
    if(extractingSpice && harvester) {
        if(harvester.getUnitPointer() != nullptr) {
            harvester.getUnitPointer()->destroy();
        }
        harvester.pointTo(NONE_ID);
    }
}

bool Worfinery::receiveHarvester(TrackedUnit* unit) {
    if(unit == nullptr || !isHarvesterLikeUnit(unit->getItemID())) {
        return false;
    }

    extractingSpice = true;
    harvester.pointTo(unit);
    drawnAngle = 1;
    showUnloadingFrame();
    return true;
}

void Worfinery::save(OutputStream& stream) const {
    BuilderBase::save(stream);

    stream.writeBool(extractingSpice);
    harvester.save(stream);
    stream.writeUint32(bookings);
}

ObjectInterface* Worfinery::getInterfaceContainer() {
    // Reuse the Refinery interface — same layout (storage list + production
    // button) makes sense for a building that produces Troopers instead
    // of Harvesters.
    return BuilderBase::getInterfaceContainer();
}

void Worfinery::updateStructureSpecificStuff() {
    if(!extractingSpice) {
        return;
    }

    UnitBase* unit = harvester.getUnitPointer();
    if(unit == nullptr) {
        extractingSpice = false;
        showIdleFrame();
        return;
    }

    if(harvesterGetAmountOfSpice(unit) > 0) {
        int healthScale = floor(5 * getHealth() / getMaxHealth());
        if(healthScale == 0) {
            healthScale = 1;
        }

        const FixPoint extractionSpeed =
            (MaximumHarvesterExtractionSpeed * healthScale) / 5;
        owner->addCredits(harvesterExtractSpice(unit, extractionSpeed), true);
        return;
    }

    auto* groundUnit = static_cast<GroundUnit*>(unit);
    if(!groundUnit->isAwaitingPickup() && unit->getGuardPoint().isValid()) {
        Carryall* carryall = nullptr;
        if(getOwner()->hasCarryalls()) {
            for(UnitBase* candidate : unitList) {
                if(candidate->getOwner() == owner && candidate->getItemID() == Unit_Carryall) {
                    auto* candidateCarryall = static_cast<Carryall*>(candidate);
                    if(!candidateCarryall->isBooked()) {
                        carryall = candidateCarryall;
                        break;
                    }
                }
            }
        }

        if(carryall != nullptr) {
            carryall->setTarget(this);
            carryall->clearPath();
            groundUnit->bookCarrier(carryall);
            unit->setTarget(nullptr);
            unit->setDestination(unit->getGuardPoint());
        } else {
            deployContainedHarvester();
        }
    } else if(!groundUnit->hasBookedCarrier()) {
        deployContainedHarvester();
    }
}

void Worfinery::unbookHarvesterDropoff() {
    if(bookings > 0) {
        --bookings;
    }
}

void Worfinery::startHarvesterDropoffAnimation() {
    showUnloadingFrame();
}

void Worfinery::deployContainedHarvester(Carryall* carryall) {
    unbookHarvesterDropoff();
    drawnAngle = 0;
    extractingSpice = false;

    UnitBase* unit = harvester.getUnitPointer();
    if(unit == nullptr) {
        showIdleFrame();
        return;
    }

    if(carryall != nullptr && unit->getGuardPoint().isValid()) {
        carryall->giveCargo(unit);
        carryall->setTarget(nullptr);
        carryall->setDestination(unit->getGuardPoint());
    } else {
        const Coord deployPos = currentGameMap->findDeploySpot(
            unit, location, currentGame->randomGen, destination, structureSize);
        unit->deploy(deployPos);
    }

    harvester.pointTo(NONE_ID);
    showIdleFrame();
}

void Worfinery::showIdleFrame() {
    firstAnimFrame = WorfineryIdleFrame;
    lastAnimFrame = WorfineryIdleFrame;
    curAnimFrame = WorfineryIdleFrame;
}

void Worfinery::showUnloadingFrame() {
    firstAnimFrame = WorfineryUnloadingFrame;
    lastAnimFrame = WorfineryUnloadingFrame;
    curAnimFrame = WorfineryUnloadingFrame;
}
