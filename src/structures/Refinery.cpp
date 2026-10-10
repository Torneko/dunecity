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

#include <structures/Refinery.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <SoundPlayer.h>
#include <Map.h>

#include <units/UnitBase.h>
#include <units/GroundUnit.h>
#include <units/Harvester.h>
#include <units/HarvesterHelpers.h>
#include <units/Carryall.h>

#include <GUI/ObjectInterfaces/RefineryAndSiloInterface.h>

/* how fast is spice extracted */
#define MAXIMUMHARVESTEREXTRACTSPEED (0.625_fix)

Refinery::Refinery(House* newOwner, int newItemID) : StructureBase(newOwner) {
    Refinery::init(newItemID);

    setHealth(getMaxHealth());

    bookings = 0;

    firstRun = true;

    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

Refinery::Refinery(InputStream& stream, int newItemID) : StructureBase(stream) {
    Refinery::init(newItemID);

    for(int index = 0; index < bayCount(); ++index) {
        const bool occupied = stream.readBool();
        harvesters[index].load(stream);
        // Old classic refineries retained their last pointer after deployment.
        if(!occupied) harvesters[index].pointTo(NONE_ID);
    }
    bookings = stream.readUint32();

    refreshAnimation();

    firstRun = false;
}

void Refinery::init(int newItemID) {
    itemID = newItemID;
    owner->incrementStructures(itemID);

    structureSize.x = itemID == Structure_Doublefinery ? 5 : 3;
    structureSize.y = 2;

    graphicID = itemID == Structure_Doublefinery ? ObjPic_Doublefinery : ObjPic_Refinery;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
}

bool Refinery::restoreLegacyDoublefineryFootprint() {
    if(itemID != Structure_Doublefinery) return true;
    // Save 9830 stored the prototype's four-column tile occupancy. Validate the
    // entire added column before assigning it, without overwriting other objects.
    for(int y=0;y<2;++y) {
        const Coord pos=location+Coord(4,y);
        if(!currentGameMap->tileExists(pos)) return false;
        auto* tile=currentGameMap->getTile(pos);
        if(tile->isMountain() || tile->hasInfantry()) return false;
        if(tile->hasANonInfantryGroundObject() && tile->getNonInfantryGroundObject()!=this) return false;
    }
    for(int y=0;y<2;++y) {
        auto* tile=currentGameMap->getTile(location+Coord(4,y));
        if(!tile->hasANonInfantryGroundObject()) tile->assignNonInfantryGroundObject(getObjectID());
        tile->setType(Terrain_Rock);
        tile->setOwner(getOwner()->getHouseID());
    }
    currentGameMap->incrementPathingRevision();
    return true;
}

Refinery::~Refinery() {
    for(auto& harvester : harvesters) {
        auto* unit = harvester.getUnitPointer();
        harvester.pointTo(NONE_ID);
        if(unit) unit->destroy();
    }
}

void Refinery::save(OutputStream& stream) const {
    StructureBase::save(stream);

    // Keep the classic refinery's exact bool/pointer/bookings byte layout.
    for(int index = 0; index < bayCount(); ++index) {
        stream.writeBool(harvesters[index].getUnitPointer() != nullptr);
        harvesters[index].save(stream);
    }
    stream.writeUint32(bookings);
}

ObjectInterface* Refinery::getInterfaceContainer() {
    if(currentGame->canControlHouse(owner) || (debug == true)) {
        return RefineryAndSiloInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

bool Refinery::isFree() const {
    for(int index = 0; index < bayCount(); ++index)
        if(!harvesters[index].getUnitPointer()) return true;
    return false;
}

UnitBase* Refinery::getContainedHarvester() {
    return const_cast<UnitBase*>(static_cast<const Refinery*>(this)->getContainedHarvester());
}

const UnitBase* Refinery::getContainedHarvester() const {
    for(int index = 0; index < bayCount(); ++index)
        if(auto* unit = harvesters[index].getUnitPointer()) return unit;
    return nullptr;
}

std::vector<UnitBase*> Refinery::getContainedHarvesters() {
    std::vector<UnitBase*> result;
    for(int index = 0; index < bayCount(); ++index)
        if(auto* unit = harvesters[index].getUnitPointer()) result.push_back(unit);
    return result;
}

bool Refinery::assignHarvester(TrackedUnit* newHarvester) {
    if(!newHarvester) return false;
    for(int index = 0; index < bayCount(); ++index) {
        if(harvesters[index].getUnitPointer() == newHarvester) return false;
    }
    for(int index = 0; index < bayCount(); ++index) {
        if(!harvesters[index].getUnitPointer()) {
            harvesters[index].pointTo(newHarvester);
            refreshAnimation();
            return true;
        }
    }
    return false;
}

void Refinery::deployHarvester(Carryall* carryall) {
    for(int index = 0; index < bayCount(); ++index) {
        auto* unit = static_cast<GroundUnit*>(harvesters[index].getUnitPointer());
        // Each carryall must collect the harvester that booked it, never the
        // other bay's still-loaded vehicle.
        if(unit && (!carryall || unit->getCarrier() == carryall)) {
            deployBay(index, carryall);
            return;
        }
    }
    if(carryall) carryall->setTarget(nullptr);
}

void Refinery::deployBay(int index, Carryall* pCarryall) {
    UnitBase* pHarvester = harvesters[index].getUnitPointer();
    if(!pHarvester) return;
    harvesters[index].pointTo(NONE_ID);
    unBook();

    if(firstRun) {
        if(getOwner() == pLocalHouse) {
            soundPlayer->playVoice(HarvesterDeployed,getOwner()->getHouseID());
        }
    }

    firstRun = false;

    if((pCarryall != nullptr) && pHarvester->getGuardPoint().isValid()) {
        pCarryall->giveCargo(pHarvester);
        pCarryall->setTarget(nullptr);
        pCarryall->setDestination(pHarvester->getGuardPoint());
    } else {
        Coord deployPos = currentGameMap->findDeploySpot(pHarvester, location, currentGame->randomGen, destination, structureSize);
        pHarvester->deploy(deployPos);
    }

    refreshAnimation();
}

void Refinery::refreshAnimation() {
    // During save loading the referenced units may not be registered yet.
    // Checking IDs here must not resolve (and invalidate) their ObjectPointers.
    const bool occupied = static_cast<bool>(harvesters[0]) || static_cast<bool>(harvesters[1]);
    drawnAngle = occupied ? 1 : 0;
    const int first = occupied ? 8 : 2;
    const int last = occupied ? 9 : (bookings ? 7 : 3);
    firstAnimFrame = first;
    lastAnimFrame = last;
    if(curAnimFrame < first || curAnimFrame > last) curAnimFrame = first;
}

void Refinery::startAnimate() {
    refreshAnimation();
    justPlacedTimer = 0;
    animationCounter = 0;
}

void Refinery::stopAnimate() {
    refreshAnimation();
}

void Refinery::updateStructureSpecificStuff() {
    refreshAnimation();
    for(int index = 0; index < bayCount(); ++index) {
        UnitBase* pHarvester = harvesters[index].getUnitPointer();
        if(pHarvester == nullptr) {
            continue;
        }

        if(harvesterGetAmountOfSpice(pHarvester) > 0) {
            FixPoint extractionSpeed = MAXIMUMHARVESTEREXTRACTSPEED;

            int scale = floor(5*getHealth()/getMaxHealth());
            if(scale == 0) {
                scale = 1;
            }

            extractionSpeed = (extractionSpeed * scale) / 5;
            owner->addCredits(harvesterExtractSpice(pHarvester, extractionSpeed), true);
        } else {
            GroundUnit* pGroundHarvester = static_cast<GroundUnit*>(pHarvester);
            if((pGroundHarvester->isAwaitingPickup() == false) && (pHarvester->getGuardPoint().isValid())) {
            // find carryall
            Carryall* pCarryall = nullptr;
            if((pHarvester->getGuardPoint().isValid()) && getOwner()->hasCarryalls())   {
                for(UnitBase* pUnit : unitList) {
                    if ((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                        Carryall* pTmpCarryall = static_cast<Carryall*>(pUnit);
                        if (!pTmpCarryall->isBooked()) {
                            pCarryall = pTmpCarryall;
                            break;
                        }
                    }
                }
            }

            if(pCarryall != nullptr) {
                pCarryall->setTarget(this);
                pCarryall->clearPath();
                pGroundHarvester->bookCarrier(pCarryall);
                pHarvester->setTarget(nullptr);
                pHarvester->setDestination(pHarvester->getGuardPoint());
            } else {
                deployBay(index, nullptr);
            }
            } else if(!pGroundHarvester->hasBookedCarrier()) {
                deployBay(index, nullptr);
            }
        }
    }
}
