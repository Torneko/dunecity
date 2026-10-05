#pragma once
#include <units/RebelHarvester.h>
// Test-build-only integration through real game objects, UI and save streams.
// SPDX-License-Identifier: GPL-2.0-or-later
#include <Achievements/AchievementEvents.h>
#include <Achievements/AchievementManager.h>
#include <GUI/dune/AchievementsWindow.h>
#include <units/Soldier.h>
#include <units/Harvester.h>
#include <units/SandWorm.h>
#include <units/MCV.h>
#include <filesystem>

class AchievementCaptureInfantry final : public InfantryBase {
public:
    explicit AchievementCaptureInfantry(House* house):InfantryBase(house){
        // A test fixture exposes arrival to exercise the complete base capture path.
        itemID=Unit_Soldier;owner->incrementUnits(itemID);setHealth(getMaxHealth());
        setObjectID(currentGame->getObjectManager().addObject(this));
        graphicID=ObjPic_Soldier;graphic=pGFXManager->getObjPic(graphicID,house->getHouseID());
        numImagesX=4;numImagesY=3;
    }
    void arrive(){justStoppedMoving=true;checkPos();}
};

inline void runAchievementRuntimeSmoke(){
    auto require=[](bool value,const std::string& text){if(!value)throw std::runtime_error("Achievement runtime check: "+text);};
    const char* output=std::getenv("DUNELEGACY_SMOKE_DIR");
    require(output&&std::filesystem::is_directory(output),"isolated profile/output required");
    auto& mods=ModManager::instance();const auto previous=mods.getActiveModName();require(mods.setActiveMod("Tornie"),"Tornie activation");
    auto options=settings.gameOptions;options.immortalHumanPlayer=false;options.sandwormsRespawn=true;
    std::string terrain="[BASIC]\nVersion=2\nTechLevel=9\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y=0;y<32;++y)terrain+=fmt::sprintf("%03d=",y)+std::string(32,'%')+"\n";
    terrain+="[Player1]\nCredits=100000\n[Player2]\nCredits=100000\n[Player3]\nCredits=100000\n";
    GameInitSettings init("achievement-runtime",terrain,false,options);
    for(auto house:{HOUSE_ATREIDES,HOUSE_HARKONNEN,HOUSE_ORDOS}){
        GameInitSettings::HouseInfo info(house,house==HOUSE_HARKONNEN?2:1);
        info.colorOfHouse=getDefaultHouseColorSlot(house==HOUSE_ATREIDES?HOUSE_HARKONNEN:house);
        info.addPlayerInfo(GameInitSettings::PlayerInfo(house==HOUSE_ATREIDES?settings.general.playerName:"Achievement AI",house==HOUSE_ATREIDES?HUMANPLAYERCLASS:DEFAULTAIPLAYERCLASS));init.addHouseInfo(info);
    }
    const auto file=(std::filesystem::path(output)/"achievement-runtime.sav").string();
    auto& manager=achievements::AchievementManager::instance();
    std::string runID;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        require(pLocalHouse&&pLocalHouse->getHouseID()==HOUSE_ATREIDES,"local house assignment");
        auto* local=game->getHouse(HOUSE_ATREIDES);auto* enemy=game->getHouse(HOUSE_HARKONNEN);auto* ally=game->getHouse(HOUSE_ORDOS);
        require(local&&enemy&&ally,"fixture houses");
        auto* vehicle=local->createUnit(Unit_Tank);vehicle->deploy(Coord(2,22));
        auto* target=enemy->createUnit(Unit_Tank);target->deploy(Coord(8,22));
        auto* allied=ally->createUnit(Unit_Tank);allied->deploy(Coord(12,22));
        AchievementEvents::begin(*game,"",false,true);
        auto matchInfo=manager.match().info;matchInfo.enabled=true;manager.begin(matchInfo);
        require(matchInfo.color!=matchInfo.defaultColor,"resolved nonstandard initial color");
        local->informWasBuilt(vehicle);local->informWasBuilt(vehicle,false);
        require(manager.statistic("UnitsBuilt")==1,"factory production, excluding Starport purchases");
        allied->handleDamage(1,vehicle->getObjectID(),local);
        require(!manager.match().damagedEnemy,"friendly fire excluded from adversary damage");
        target->handleDamage(1,vehicle->getObjectID(),local);
        require(manager.match().damagedEnemy&&!manager.match().destroyedEnemy,"nonlethal adversary damage");
        target->handleDamage(target->getMaxHealth()+1,vehicle->getObjectID(),ally);
        require(!manager.match().destroyedEnemy,"another faction's destruction is not attributed locally");

        manager.begin(matchInfo);
        auto* building=enemy->placeStructure(NONE_ID,Structure_WindTrap,18,3,true,true);
        require(building!=nullptr,"capture target");building->setHealth(1);
        auto* infantry=new AchievementCaptureInfantry(local);infantry->deploy(Coord(18,3));infantry->doCaptureStructure(building);infantry->arrive();
        require(manager.statistic("StructuresCaptured")==1,"real infantry capture event");
        require(manager.statistic("UnitsLost")==1,"consumed infantry is counted before a possible victory");
        require(!manager.match().destroyedEnemy&&manager.statistic("EnemyStructuresDestroyed")==0,"capture is not a destruction");
        require(manager.unlocked("PRISE_DE_CONTROLE"),"capture achievement");

        for(int i=0;i<5;++i){auto* soldier=enemy->createUnit(Unit_Soldier);soldier->deploy(Coord(20,20));}
        currentGameMap->getTile(20,20)->squash(vehicle);
        require(manager.unlocked("ROADKILL")&&manager.statistic("InfantryCrushed")==5,"actual same-vehicle infantry crushing");

        const auto deaths=manager.statistic("EnemyUnitsDestroyed");
        auto* flame=local->createUnit(Unit_FlameTank);flame->deploy(Coord(2,24));local->informWasBuilt(flame);
        auto* flameTarget=enemy->createUnit(Unit_Tank);flameTarget->deploy(Coord(8,24));
        flameTarget->handleDamage(flameTarget->getMaxHealth()+1,flame->getObjectID(),local);
        require(manager.statistic("FlameTankKills")==1&&manager.statistic("EnemyUnitsDestroyed")==deaths+1,"flame attribution");

        auto* worm=enemy->createUnit(Unit_Sandworm);worm->deploy(Coord(26,26));
        worm->handleDamage(worm->getMaxHealth()/2+1,vehicle->getObjectID(),local);worm->update();
        require(manager.unlocked("WORM_HUNTER")&&manager.statistic("SandwormsKilled")==1,"worm defeated at half health with respawn enabled");
        require(!worm->isActive(),"defeated worm sleeps");
        worm->update();require(manager.statistic("SandwormsKilled")==1,"worm award deduplicated");
        auto* ownedWorm=local->createUnit(Unit_Sandworm);ownedWorm->deploy(Coord(27,26));
        ownedWorm->handleDamage(ownedWorm->getMaxHealth()/2+1,vehicle->getObjectID(),local);ownedWorm->update();
        require(manager.statistic("SandwormsKilled")==2,"same-house neutral worm still credits local attacker");
        auto* otherWorm=enemy->createUnit(Unit_Sandworm);otherWorm->deploy(Coord(28,26));
        otherWorm->handleDamage(otherWorm->getMaxHealth()/2+1,vehicle->getObjectID(),ally);otherWorm->update();
        require(manager.statistic("SandwormsKilled")==2,"AI-defeated worm not credited locally");
        auto* mcv=static_cast<MCV*>(local->createUnit(Unit_MCV));mcv->deploy(Coord(26,3));
        const auto losses=manager.statistic("UnitsLost");require(mcv->doDeploy(),"MCV deployment");
        require(manager.statistic("UnitsLost")==losses,"MCV conversion is not a loss");

        auto* red=currentGameMap->getTile(25,22);red->setType(Terrain_RedSpice);red->setSpice(500_fix);
        auto* green=currentGameMap->getTile(25,24);green->setType(Terrain_GreenSpice);green->setSpice(500_fix);
        auto* harvester=static_cast<Harvester*>(local->createUnit(Unit_Harvester));harvester->deploy(Coord(25,22));harvester->setDestination(Coord(25,22));harvester->move();
        require(manager.unlocked("RED_HARVEST"),"real red-spice collection");
        currentGameMap->removeObjectFromMap(harvester->getObjectID());harvester->deploy(Coord(25,24));harvester->setDestination(Coord(25,24));harvester->move();
        require(manager.unlocked("GREEN_HARVEST")&&manager.unlocked("SPICE_COLLECTOR"),"real green-spice collection");
        // Exercise both real harvester classes at an eligible healing cycle.
        for(int item:{Unit_Harvester,Unit_RebelHarvester}) {
            UnitBase* unit=nullptr;
            constexpr unsigned interval=(1000+GAMESPEED_DEFAULT-1)/GAMESPEED_DEFAULT;
            for(unsigned attempt=0;attempt<interval;++attempt) {
                unit=local->createUnit(item);
                if(unit->getObjectID()%interval==game->getGameCycleCount()%interval)break;
            }
            auto* lilac=currentGameMap->getTile(24,28);lilac->setType(Terrain_PaleLilacSpice);lilac->setSpice(500_fix);
            unit->deploy(Coord(24,28));unit->setDestination(Coord(24,28));unit->setHealth(unit->getMaxHealth()/2);
            auto harvestMove=[&] { if(item==Unit_Harvester)static_cast<Harvester*>(unit)->move();else static_cast<RebelHarvester*>(unit)->move(); };
            const auto health=unit->getHealth();harvestMove();
            require(unit->getHealth()==health+1,"real harvester heals only one point per eligible tick");
            const auto healed=unit->getHealth();lilac->setSpice(0_fix);lilac->setType(Terrain_Sand);harvestMove();
            require(unit->getHealth()==healed,"healing requires actual spice collection");
            currentGameMap->removeObjectFromMap(unit->getObjectID());
        }
        local->addCredits(10000_fix,true);require(manager.unlocked("THE_SPICE_MUST_FLOW"),"refinery credit event");
        require(game->saveGame(file),"save stream with profile checkpoint");runID=manager.match().runID;
        manager.enemyDestroyed(999999,false,false,false);
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(file),"unchanged save format reload");
        AchievementEvents::begin(*game,AchievementEvents::fileKey(file),true,true);
        auto info=manager.match().info;info.enabled=true;manager.begin(info,AchievementEvents::fileKey(file),true);
        require(manager.match().runID==runID&&manager.match().historyKnown,"checkpoint survives save/load");
        game->setGameWon();require(manager.unlocked("TRUE_COLORS"),"real completed-game victory with initial color");
        require(manager.statistic("GamesWon")==1,"victory event counted once");
        require(!manager.unlocked("NO_CASUALTIES"),"capture infantry loss prevents a no-loss award");
        AchievementsWindow window;require(window.getSize().x<=getRendererWidth()&&window.getSize().y<=getRendererHeight(),"achievement window fits display");
        SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);window.draw();
        const int width=getRendererWidth(),height=getRendererHeight();auto surface=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_ARGB8888));
        require(surface!=nullptr&&SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,surface->pixels,surface->pitch)==0,"window rendering");
        require(SDL_SaveBMP(surface.get(),(std::filesystem::path(output)/"achievements-window.bmp").string().c_str())==0,"window preview");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* local=game->getHouse(HOUSE_ATREIDES);auto* enemy=game->getHouse(HOUSE_HARKONNEN);
        auto* vehicle=local->createUnit(Unit_Tank);vehicle->deploy(Coord(2,22));
        auto* building=enemy->placeStructure(NONE_ID,Structure_WindTrap,18,3,true,true);building->setHealth(1);
        AchievementEvents::begin(*game,"",false,true);auto info=manager.match().info;info.enabled=true;manager.begin(info);
        game->winFlags=WINLOSEFLAGS_AI_NO_BUILDINGS;game->loseFlags=WINLOSEFLAGS_AI_NO_BUILDINGS;
        auto* infantry=new AchievementCaptureInfantry(local);infantry->deploy(Coord(18,3));infantry->doCaptureStructure(building);infantry->arrive();
        require(manager.match().completed&&manager.statistic("GamesWon")==2,"final-building capture automatically wins");
        require(manager.unlocked("PACIFISM")&&manager.unlocked("SANS_TIRER"),"winning capture counted before victory checks");
        require(!manager.unlocked("NO_CASUALTIES"),"winning capture consumes its infantry before victory checks");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    if(const char* legacy=std::getenv("DUNELEGACY_OLD_SAVE")){
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(legacy),"previous-version save compatibility");
        AchievementEvents::begin(*game,AchievementEvents::fileKey(legacy),true,true);auto info=manager.match().info;info.enabled=true;manager.begin(info,AchievementEvents::fileKey(legacy),true);
        require(!manager.match().historyKnown,"legacy saves keep unknown history conservative");
        SDL_Log("ACHIEVEMENT LEGACY SAVE: previous-version save loaded without changing its bytes");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;require(mods.setActiveMod(previous),"mod restoration");
    SDL_Log("ACHIEVEMENT SMOKE COMPLETE: attribution, captures, roadkill, worms, spice, colors, save/load and window rendering");
}
