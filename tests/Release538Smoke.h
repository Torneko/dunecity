#pragma once
#include <Campaign/CoopCampaignRuntime.h>
#include <Achievements/AchievementEvents.h>
#include <Achievements/AchievementManager.h>
#include <GUI/dune/BuilderList.h>
#include <GUI/StaticContainer.h>
#include <FileClasses/LoadSavePNG.h>
#include <Bullet.h>
#include <Trigger/ReinforcementTrigger.h>
#include <structures/StructureBase.h>
#include <filesystem>
#include <map>
#include "WorfinerySquadSmoke.h"
#include "Followup538Smoke.h"

inline void runRelease538Smoke() {
    const auto require = [](bool value, const std::string& message) {
        if(!value) throw std::runtime_error("1.0.538 integration: " + message);
    };
    const auto previous = ModManager::instance().getActiveModName();
    const std::filesystem::path output(std::getenv("DUNELEGACY_SMOKE_DIR"));
    auto& manager = achievements::AchievementManager::instance();
    for(const std::string mod : {"vanilla", "Tornie", "Jericho", "TornieLite", "JerichoLite"}) {
        require(ModManager::instance().setActiveMod(mod), "mod activation");
        verifyFollowup538(output.string(), mod);
        std::vector<int> roster;
        for(int h = 0; h < NUM_HOUSES; ++h) if(isCampaignHouseAvailable(static_cast<HOUSETYPE>(h))) roster.push_back(h);
        auto session = coop::CoopCampaignSession::create("release538-" + mod, mod, roster,
            {HOUSE_ATREIDES, HOUSE_ATREIDES}, {settings.general.playerName, "538 AI Ally"}, 538u, false);
        session.setPlayerClass(1,"qBotMedium");
        auto options = effectiveGameOptions;
        options.chaosMode = false; options.easyMode = false; options.immortalHumanPlayer = false;
        for(int stage = 1; stage <= coop::StageCount; ++stage) {
            if(stage != 1 && stage != 2 && stage != 9) {session.completeMission(true); continue;}
            const auto baseline = coop::makeGameSettings(session, options, "538 baseline");
            const auto baselineContext = coop::readContext(baseline.getFiledata());
            std::map<int,int> ordinary;
            std::map<int,FixPoint> originalCredits;
            {
                auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(baseline);
                for(const auto* unit : unitList) ++ordinary[unit->getOwner()->getHouseID()];
                for(const auto& slot:baselineContext->slots)
                    originalCredits[slot.house]=game->getHouse(slot.house)->getCredits();
            }
            currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
            session.setEasyMode(true); session.setExtraEnemyForces(true);
            const auto enhanced = coop::makeGameSettings(session, options, "538 options");
            const auto context = coop::readContext(enhanced.getFiledata());
            require(context && context->easyMode && context->extraEnemyForces, "option metadata");
            const auto save = (output/(mod+"-538-options-"+std::to_string(stage)+".sav")).string();
            std::size_t triggerCount = 0;
            {
                auto game = std::make_unique<Game>(); currentGame = game.get(); game->initGame(enhanced);
                std::map<int,int> actual;
                for(const auto* unit : unitList) ++actual[unit->getOwner()->getHouseID()];
                for(std::size_t slot=0;slot<context->slots.size();++slot) {
                    const int owner=context->slots[slot].house;
                    const int extra=slot>=2 && context->enemyPresent[slot-2] ? 5 : 0;
                    require(actual[owner]==ordinary[owner]+extra,"exactly five added units per active enemy");
                    require(enhanced.campaignPurchasePrice(100,owner)==(slot<2?75:100),"allied-only discount");
                    require(enhanced.campaignPurchasePrice(10,owner)==(slot<2?1:10),"minimum purchase price");
                    require(enhanced.campaignStartingCredits(1000,owner)==(slot<2 && stage==2?1500:1000),"only mission-two allied funds");
                    require(game->getHouse(owner)->getCredits()==originalCredits[owner]+FixPoint(slot<2 && stage==2?500:0),"actual allied starting funds");
                }
                triggerCount=game->getTriggerManager().getTriggers().size();
                require(game->saveGame(save),"options checkpoint");
            }
            currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
            {
                auto game = std::make_unique<Game>(); currentGame = game.get();
                GameInitSettings load(getBasename(save,true),readCompleteFile(save),"538-checkpoint");
                for(const auto& house:enhanced.getHouseInfoList()) load.addHouseInfo(house);
                game->initGame(load);
                const auto saved=coop::readContext(game->getGameInitSettings().getFiledata());
                require(saved && saved->easyMode && saved->extraEnemyForces,"saved options");
                require(game->getTriggerManager().getTriggers().size()==triggerCount,"reinforcements duplicated on reload");
                std::map<int,int> actual;
                for(const auto* unit:unitList)++actual[unit->getOwner()->getHouseID()];
                for(std::size_t slot=0;slot<context->slots.size();++slot) {
                    const int owner=context->slots[slot].house;
                    const int extra=slot>=2 && context->enemyPresent[slot-2]?5:0;
                    require(actual[owner]==ordinary[owner]+extra,"extra forces duplicated on reload");
                    require(game->getHouse(owner)->getCredits()==originalCredits[owner]+FixPoint(slot<2 && stage==2?500:0),"starting credit bonus repeated on reload");
                }
                if(stage==2) {
                    StaticContainer container;
                    int column=0;
                    for(const int building:{Structure_WOR,Structure_Worfinery,Structure_Barracks}) {
                        if(mod=="vanilla" && building==Structure_Worfinery)continue;
                        auto* structure=pLocalHouse->placeStructure(NONE_ID,building,2+4*(building%3),2,true,true);
                        require(structure,"production portrait building");
                        auto* list=BuilderList::create(structure->getObjectID());
                        container.addWidget(list,Point(10+column++*110,10),list->getMinimumSize());
                        for(const int item:{Unit_Soldier,Unit_Infantry,Unit_Infantry5,Unit_Trooper,Unit_Troopers,Unit_Troopers5}) {
                            const bool expected=mod!="vanilla";
                            require((list->getProductionPortrait(item)!=nullptr)==expected,"Barracks/WOR/Worfinery production portraits, Vanilla excluded");
                            require(list->getProductionPortrait(item)!=resolveItemPicture(item,HOUSE_ATREIDES),"selected-unit portrait replaced");
                        }
                    }
                    SDL_RenderClear(renderer);container.draw(Point(0,0));
                    auto pixels=renderReadSurface(renderer);
                    require(pixels && SavePNG(pixels.get(),(output/(mod+"-538-production-portraits.png")).string().c_str())==0,"production portrait capture");
                }
            }
            currentGame = nullptr; pLocalHouse = nullptr; pLocalPlayer = nullptr;
            session.setEasyMode(false); session.setExtraEnemyForces(false);session.completeMission(true);
        }
        if(mod=="Tornie") {
            const auto color=getHouseColorSDL(HOUSECOLOR_GUEST_1);
            require(color.b>color.r && color.r>color.g,"Tornie Wildspade dark purple");
        }
        if(mod=="Jericho") {
            const auto color=getHouseColorSDL(HOUSE_REBELS);
            require(color.r==color.g && color.g==color.b && color.r<=82,"Jericho Rebels dark grey");
        }
        if(mod!="vanilla") {
            verifyWorfinerySquad(output.string(),mod);
            verifyBarracksSquad(output.string(),mod);
        }
        SDL_Log("538 OPTIONS PASS: %s missions 1/2/9, +5 per enemy, allied Easy Mode, AI guest, same faction, save/load and production portraits",mod.c_str());
    }
    // Run real palace-sized blast footprints through Map::damage and ObjectBase.
    require(ModManager::instance().setActiveMod("Tornie"),"missile fixture mod");
    auto options=effectiveGameOptions; options.immortalHumanPlayer=false;
    std::string terrain="[BASIC]\nVersion=2\nTechLevel=9\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y=0;y<32;++y)terrain+=fmt::sprintf("%03d=",y)+std::string(32,'%')+"\n";
    terrain+="[Player1]\nCredits=10000\n[Player2]\nCredits=10000\n";
    GameInitSettings init("538-missile",terrain,false,options);
    for(const auto house:{HOUSE_ATREIDES,HOUSE_HARKONNEN}) {
        GameInitSettings::HouseInfo info(house,house==HOUSE_ATREIDES?1:2);
        info.addPlayerInfo(GameInitSettings::PlayerInfo(house==HOUSE_ATREIDES?settings.general.playerName:"538 Enemy",house==HOUSE_ATREIDES?HUMANPLAYERCLASS:DEFAULTAIPLAYERCLASS));
        init.addHouseInfo(info);
    }
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* local=game->getHouse(HOUSE_ATREIDES);auto* enemy=game->getHouse(HOUSE_HARKONNEN);
        auto* launcher=local->placeStructure(NONE_ID,Structure_Palace,2,2,true,true);
        require(launcher,"missile launcher");
        AchievementEvents::begin(*game,"",false,true);
        auto info=manager.match().info;info.enabled=true;
        // This suite destroys buildings; the older capture suite requires a
        // fresh cumulative profile. Keep their real-event fixtures independent.
        require(manager.configure(getDuneLegacyDataDir()+"/config/Achievements.ini",
            (output/"achievements-538-missiles.ini").string()),"isolated missile profile");
        manager.begin(info);
        const auto fire=[&](House* owner,Coord destination) {
            const auto shooter=owner==local?launcher->getObjectID():enemy->placeStructure(NONE_ID,Structure_Palace,26,2,true,true)->getObjectID();
            auto* bullet=new Bullet(shooter,&destination,&destination,Bullet_LargeRocket,10000,false,nullptr);
            bulletList.push_back(bullet);bullet->destroy();
        };
        fire(enemy,Coord(25*TILESIZE,25*TILESIZE));
        require(!manager.unlocked("FUEL_WASTE"),"enemy miss granted local award");
        auto* friendly=local->placeStructure(NONE_ID,Structure_WindTrap,20,20,true,true);
        require(friendly,"friendly blast target");
        fire(local,Coord(21*TILESIZE,21*TILESIZE));
        require(!manager.unlocked("FUEL_WASTE"),"friendly fire counted as a miss");
        fire(local,Coord(27*TILESIZE,27*TILESIZE));
        require(manager.unlocked("FUEL_WASTE"),"real empty palace blast");
        for(const auto position:{Coord(10,10),Coord(11,10),Coord(12,10)}) {
            auto* wall=enemy->placeStructure(NONE_ID,Structure_Wall,position.x,position.y,true,true);
            require(wall,"wall blast fixture");wall->setHealth(1);
        }
        fire(local,Coord(11*TILESIZE,10*TILESIZE));
        require(!manager.unlocked("DEMOLITION_STRIKE"),"walls counted as buildings");
        for(const auto position:{Coord(10,10),Coord(12,10),Coord(10,12)}) {
            auto* target=enemy->placeStructure(NONE_ID,Structure_WindTrap,position.x,position.y,true,true);
            require(target,"demolition target");target->setHealth(1);
        }
        fire(local,Coord(12*TILESIZE,12*TILESIZE));
        require(manager.unlocked("DEMOLITION_STRIKE") && manager.statistic("BestMissileStructures")>=3,"three distinct buildings from one real missile");
        SDL_Log("538 MISSILE PASS: enemy excluded, friendly hit excludes waste, empty blast and three-building single strike");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    manager.suspend();require(ModManager::instance().setActiveMod(previous),"restore mod");
}
