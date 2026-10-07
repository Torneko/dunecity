// SPDX-License-Identifier: GPL-2.0-or-later
#include <Achievements/AchievementEvents.h>
#include <Achievements/AchievementManager.h>
#include <Game.h>
#include <House.h>
#include <ObjectBase.h>
#include <Tile.h>
#include <units/UnitBase.h>
#include <structures/StructureBase.h>
#include <globals.h>
#include <main.h>
#include <misc/FileSystem.h>
#include <misc/md5.h>
#include <mod/ModManager.h>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace AchievementEvents {
namespace {
auto& manager(){return achievements::AchievementManager::instance();}
bool local(House* house){return house&&pLocalHouse&&house==pLocalHouse;}
bool enemy(House* house){return house&&pLocalHouse&&house->getTeamID()!=pLocalHouse->getTeamID();}
bool enemiesRemain(bool initial = false){
    const auto& destroyed=manager().match().destroyed;
    for(auto* u:unitList)if(enemy(u->getOwner())&&u->getHealth()>0&&(initial||!destroyed.count(u->getObjectID())))return true;
    for(auto* s:structureList)if(enemy(s->getOwner())&&s->getHealth()>0&&(initial||!destroyed.count(s->getObjectID())))return true;
    return false;
}
std::string houseName(HOUSETYPE house){
    const char* names[]={"Harkonnen","Atreides","Ordos","Fremen","Sardaukar","Mercenary","Neutral","Rebels","Corruptique","Wildspade","Kleshmersh","Tharpique"};
    const int identity=getHouseFactionIdentity(house);return identity>=0&&identity<NUM_HOUSES?names[identity]:"Unknown";
}
bool french(){return settings.general.language=="fr";}
}
void initializeProfile(){
    const auto catalog=getDuneLegacyDataDir()+"/config/Achievements.ini";
    if(!manager().configure(catalog,getDirname(getConfigFilepath())+"/achievements.ini"))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Achievements: %s",manager().error().c_str());
}
std::string dataKey(const std::string& data){
    unsigned char digest[16];md5(reinterpret_cast<const unsigned char*>(data.data()),data.size(),digest);
    std::ostringstream out;out<<std::hex<<std::setfill('0');for(auto c:digest)out<<std::setw(2)<<static_cast<unsigned>(c);return out.str();
}
std::string fileKey(const std::string& name){if(!existsFile(name))return {};return dataKey(readCompleteFile(name));}
void begin(Game& game,const std::string& key,bool resumed,bool eligible){
    initializeProfile();achievements::MatchInfo info;
    const auto& setup=game.getGameInitSettings();
    switch(setup.getGameType()){
        case GameType::Campaign:info.mode=achievements::Mode::Campaign;break;
        case GameType::Skirmish:info.mode=achievements::Mode::Skirmish;break;
        case GameType::CustomMultiplayer:info.mode=achievements::Mode::Multiplayer;break;
        default:info.mode=achievements::Mode::Custom;break;
    }
    info.mod=ModManager::instance().getActiveModName();
    info.house=pLocalHouse?houseName(static_cast<HOUSETYPE>(pLocalHouse->getFactionID())):"Unknown";
    info.chaosCampaignEligible=setup.isChaosCampaignEligible();
    info.mission=setup.getMission();info.enabled=eligible&&pLocalHouse&&!std::getenv("DUNELEGACY_SMOKE_DIR");
    if(pLocalHouse){const auto house=static_cast<HOUSETYPE>(pLocalHouse->getHouseID());info.color=getHouseVisualHouse(house);info.defaultColor=getDefaultHouseColorSlot(static_cast<HOUSETYPE>(pLocalHouse->getFactionID()));info.initialRefinedSpice=static_cast<std::uint64_t>(std::max(0,pLocalHouse->getHarvestedSpice().floor()));}
    info.hadEnemies=enemiesRemain(true);info.roadkillWindow=MILLI2CYCLES(2000);
    for(const auto& house:setup.getHouseInfoList()){
        if(!pLocalHouse||house.team==pLocalHouse->getTeamID())continue;
        for(const auto& player:house.playerInfoList)if(player.playerClass.find("Hard")!=std::string::npos||player.playerClass.find("Brutal")!=std::string::npos)info.highDifficulty=true;
    }
    manager().begin(info,key,resumed);
    for(auto* unit:unitList)if(unit->getItemID()==Unit_FlameTank)manager().flameSource(unit->getObjectID());
}
void finish(bool won){manager().finish(won,enemiesRemain());}
void pump(Game& game){
    static std::uint32_t lastNotification=0,lastSave=0;const auto now=SDL_GetTicks();
    if(now-lastSave>=5000){manager().save();lastSave=now;}
    if(now-lastNotification<4000)return;
    const auto id=manager().takeNotification();if(id.empty())return;
    for(const auto& a:manager().definitions())if(a.id==id){game.addToNewsTicker(std::string(french()?"Haut fait débloqué : ":"Achievement unlocked: ")+manager().displayName(a,french()));break;}
    lastNotification=now;
}
void damage(ObjectBase* victim,std::uint32_t attackerID,House* attacker,bool lethal){
    if(victim&&lethal&&victim->isAUnit()&&victim->getItemID()!=Unit_Sandworm&&pLocalHouse
       &&(local(victim->getOwner())||victim->getOriginalHouseID()==pLocalHouse->getHouseID()))manager().unitLost(victim->getObjectID());
    const bool flame=manager().match().flameSources.count(attackerID)>0;
    if(victim&&victim->getItemID()==Unit_Sandworm)manager().wormSource(victim->getObjectID(),local(attacker)?(flame?2:1):0);
    if(!currentGame||!victim||!local(attacker)||!enemy(victim->getOwner()))return;
    const auto* source=currentGame->getObjectManager().getObject(attackerID);
    if(source&&source->getItemID()==Unit_Sandworm)return;
    manager().enemyDamage();
    // Worm defeats are resolved at the game's half-health retreat threshold.
    if(lethal&&victim->getItemID()!=Unit_Sandworm)manager().enemyDestroyed(victim->getObjectID(),victim->isAStructure(),false,flame);
}
void wormDefeated(UnitBase* unit){
    if(unit&&unit->getItemID()==Unit_Sandworm&&unit->getHealth()<=unit->getMaxHealth()/2){
        const auto source=manager().match().wormSources.find(unit->getObjectID());
        if(source!=manager().match().wormSources.end()&&source->second>0)manager().enemyDestroyed(unit->getObjectID(),false,true,source->second==2);
    }
}
void lost(UnitBase* unit){
    wormDefeated(unit);
    if(!unit||!pLocalHouse||(!local(unit->getOwner())&&unit->getOriginalHouseID()!=pLocalHouse->getHouseID()))return;
    // Deploying a healthy MCV is a conversion, not a loss.
    if(unit->getItemID()==Unit_MCV&&unit->getHealth()>0&&!unit->isVisible())return;
    if(unit->getHealth()>0&&!unit->isVisible()&&(unit->getItemID()==Unit_Frigate||isCarryallUnit(unit->getItemID())))return;
    manager().unitLost(unit->getObjectID());
}
void built(House* house,ObjectBase* object){
    if(object&&object->getItemID()==Unit_FlameTank)manager().flameSource(object->getObjectID());
    if(!local(house)||!object)return;
    std::string special;
    switch(object->getItemID()){
        case Unit_RocketTrike:special="RocketTrike";break;case Unit_SonicTrike:special="SonicTrike";break;
        case Unit_FlameTank:special="FlameTank";break;case Unit_EliteLauncher:special="EliteLauncher";break;
        case Unit_EliteSiegeTank:special="EliteSiegeTank";break;case Unit_ChemicalSiegeTank:special="ChemicalSiegeTank";break;
        case Unit_ChemicalCarryall:special="ChemicalCarryall";break;case Unit_RebelHarvester:special="Harvestank";break;default:break;
    }
    manager().built(object->isAStructure(),special);
}
void captured(House* captor,House* previous){if(local(captor)&&enemy(previous))manager().captured();}
void crushed(ObjectBase* victim,ObjectBase* vehicle){
    if(victim&&vehicle&&local(vehicle->getOwner())&&enemy(victim->getOwner()))
        manager().crushed(victim->getObjectID(),vehicle->getObjectID(),currentGame->getGameCycleCount());
}
void refined(House* house,std::uint64_t total){if(local(house))manager().refinedSpice(total);}
void spice(House* house,const Tile* tile){
    if(!local(house)||!tile)return;
    manager().harvestedType(tile->isRedSpice()?2:tile->isGreenSpice()?4:tile->isPaleLilacSpice()?8:tile->isWhiteSpice()?16:1);
}
void bloom(House* house){if(local(house))manager().bloom();}
void missile(House* house){if(local(house))manager().missile();}
void palace(House* house){if(local(house))manager().palace();}
}
