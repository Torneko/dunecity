#pragma once
// SPDX-License-Identifier: GPL-2.0-or-later
#include <GUI/ObjectInterfaces/UnitInterface.h>
#include <GUI/ObjectInterfaces/MultiUnitInterface.h>
#include <units/InfantryBase.h>
#include <filesystem>
#include <fstream>

struct CaptureHuntGameFixture {
    static void start(Game& game) { game.initializeGameLoop(); }
    static void chaos(Game& game, ObjectData& data, Uint32 seed) {
        game.chaosMode.generate(data, true, seed);
    }
    static void resetChaos(Game& game) { game.chaosMode.reset(); }
    static void tick(Game& game) {
        game.processObjects();
        ++game.gameCycleCount;
    }
};

class CaptureHuntUnitUI final : public UnitInterface {
public:
    using UnitInterface::update;
    explicit CaptureHuntUnitUI(Uint32 id) : UnitInterface(id) {}
    bool hasButton() const { return sabotageButton.isVisible(); }
    bool belowRetreat() {
        return buttonVBox.getWidgetPosition(&sabotageButton).y
            >= buttonVBox.getWidgetPosition(&retreatButton).y + retreatButton.getSize().y;
    }
    bool selected() const { return sabotageButton.getToggleState(); }
    void click() {
        sabotageButton.handleMouseLeft(5,5,true);
        sabotageButton.handleMouseLeft(5,5,false);
    }
};

class CaptureHuntGroupUI final : public MultiUnitInterface {
public:
    using MultiUnitInterface::update;
    bool hasButton() const { return sabotageButton.isVisible(); }
    bool belowRetreat() {
        return buttonVBox.getWidgetPosition(&sabotageButton).y
            >= buttonVBox.getWidgetPosition(&retreatButton).y + retreatButton.getSize().y;
    }
    bool selected() const { return sabotageButton.getToggleState(); }
    void click() {
        sabotageButton.handleMouseLeft(5,5,true);
        sabotageButton.handleMouseLeft(5,5,false);
    }
};

inline void verifyCaptureHunt(const std::string& output, const std::string& mod) {
    auto require=[](bool value,const char* text) {
        if(!value) throw std::runtime_error(std::string("Capture hunt: ")+text);
    };
    const auto mapPath=(std::filesystem::path(output)/(mod+"-capture-hunt.ini")).string();
    {
        MapEditor editor;
        editor.setMap(MapData(32,32,Terrain_Rock),MapInfo());
        for(auto& player:editor.getPlayers()) {
            player.bActive=player.house==HOUSE_ATREIDES || player.house==HOUSE_HARKONNEN || player.house==HOUSE_ORDOS;
            player.bAnyHouse=false;
            player.credits=0; // Keep red capture targets from automatically repairing during the walk.
        }
        for(const auto& entry: {std::pair<unsigned,Coord>{Unit_Soldier,{3,3}},
             {Unit_Trooper,{3,5}}, {Unit_Infantry5,{3,20}}, {Unit_Troopers5,{3,24}}}) {
            MapEditorUnitPlaceOperation(entry.second,HOUSE_ATREIDES,entry.first,256,RIGHT,SABOTAGE).perform(&editor);
        }
        editor.saveMap(mapPath);editor.loadMap(mapPath);
        require(editor.getUnitList().size()==4,"editor lost capture squads");
        for(const auto& unit:editor.getUnitList()) require(unit.attackmode==SABOTAGE,"editor mode roundtrip");
    }
    std::ifstream mapFile(mapPath);std::string map((std::istreambuf_iterator<char>(mapFile)),{});
    auto options=effectiveGameOptions;options.immortalHumanPlayer=false;
    options.structuresDegradeOnConcrete=false;
    GameInitSettings init("capture-hunt",map,false,options);
    for(const auto house:{HOUSE_ATREIDES,HOUSE_HARKONNEN,HOUSE_ORDOS}) {
        GameInitSettings::HouseInfo info(house,house==HOUSE_HARKONNEN?2:1);
        info.addPlayerInfo(GameInitSettings::PlayerInfo(house==HOUSE_ATREIDES?settings.general.playerName:"Capture AI",
            house==HOUSE_ATREIDES?HUMANPLAYERCLASS:DEFAULTAIPLAYERCLASS));
        init.addHouseInfo(info);
    }
    const auto save=(std::filesystem::path(output)/(mod+"-capture-hunt.sav")).string();
    Uint32 soldierID=NONE_ID,firstID=NONE_ID,secondID=NONE_ID;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* local=game->getHouse(HOUSE_ATREIDES);
        auto* enemy=game->getHouse(HOUSE_HARKONNEN);
        auto* ally=game->getHouse(HOUSE_ORDOS);
        require(local && enemy && ally && local->getNumItems(Unit_Soldier)==6
            && local->getNumItems(Unit_Trooper)==6,"editor squad expansion");
        UnitBase* soldier=nullptr;UnitBase* trooper=nullptr;
        for(auto* unit:unitList) {
            require(unit->getAttackMode()==SABOTAGE,"game loader did not link capture hunt");
            if(unit->getLocation()==Coord(3,3)) soldier=unit;
            if(unit->getLocation()==Coord(3,5)) trooper=unit;
            unit->doSetAttackMode(STOP);
        }
        require(soldier && trooper,"single infantry placements");soldierID=soldier->getObjectID();
        auto* tank=local->createUnit(Unit_Tank);tank->deploy(Coord(2,15));
        auto* saboteur=local->createUnit(Unit_Saboteur);saboteur->deploy(Coord(28,28));
        tank->doSetAttackMode(SABOTAGE);saboteur->doSetAttackMode(SABOTAGE);
        require(tank->getAttackMode()==GUARD && saboteur->getAttackMode()==GUARD,"noncapturing units accepted capture hunt");
        {
            CaptureHuntUnitUI vehicleUI(tank->getObjectID());
            CaptureHuntUnitUI saboteurUI(saboteur->getObjectID());
            CaptureHuntUnitUI unitUI(soldierID);
            require(!vehicleUI.hasButton() && !saboteurUI.hasButton(),"button shown on noncapturing units");
            require(unitUI.hasButton() && unitUI.belowRetreat(),"single-unit button is not below Retreat");
            unitUI.click();game->getCommandManager().executeCommands(game->getGameCycleCount());unitUI.update();
            require(soldier->getAttackMode()==SABOTAGE && unitUI.selected(),"single-unit button command/toggle");
            SDL_RenderSetClipRect(renderer,nullptr);SDL_SetRenderDrawColor(renderer,20,20,20,255);SDL_RenderClear(renderer);
            unitUI.draw(Point(0,0));
            SDL_Rect area{0,0,SIDEBARWIDTH-25,std::min(550,getRendererHeight())};
            auto pixels=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_ARGB8888));
            require(pixels && SDL_RenderReadPixels(renderer,&area,pixels->format->format,pixels->pixels,pixels->pitch)==0,"button screenshot");
            require(SDL_SaveBMP(pixels.get(),(std::filesystem::path(output)/(mod+"-capture-button.bmp")).string().c_str())==0,"button preview save");
        }
        game->getSelectedList().insert(soldierID);game->getSelectedList().insert(trooper->getObjectID());
        game->getSelectedList().insert(tank->getObjectID());
        {
            CaptureHuntGroupUI ui;require(ui.hasButton() && ui.belowRetreat(),"group button placement");
            ui.click();game->getCommandManager().executeCommands(game->getGameCycleCount());ui.update();
            require(soldier->getAttackMode()==SABOTAGE && trooper->getAttackMode()==SABOTAGE
                && tank->getAttackMode()==GUARD && ui.selected(),"mixed group command did not filter infantry");
            game->getSelectedList().clear();game->getSelectedList().insert(tank->getObjectID());ui.update();
            require(!ui.hasButton(),"vehicle-only group shows sabotage");
        }
        game->getSelectedList().clear();trooper->doSetAttackMode(STOP);tank->doSetAttackMode(STOP);
        auto* enemyVehicle=enemy->createUnit(Unit_Tank);enemyVehicle->deploy(Coord(4,5));enemyVehicle->doSetAttackMode(STOP);
        ally->placeStructure(NONE_ID,Structure_WindTrap,6,3,true,true);
        local->placeStructure(NONE_ID,Structure_WindTrap,9,3,true,true);
        enemy->placeStructure(NONE_ID,Structure_Barracks,6,8,true,true);
        auto* hidden=enemy->placeStructure(NONE_ID,Structure_WindTrap,6,12,true,true);hidden->setVisible(local->getTeamID(),false);
        auto* first=enemy->placeStructure(NONE_ID,Structure_WindTrap,18,3,true,true);
        auto* second=enemy->placeStructure(NONE_ID,Structure_WindTrap,22,3,true,true);
        firstID=first->getObjectID();secondID=second->getObjectID();
        auto* aiInfantry=enemy->createUnit(Unit_Trooper);aiInfantry->deploy(Coord(29,15));
        aiInfantry->doSetAttackMode(SABOTAGE);
        for(const auto& player:enemy->getPlayerList()) player->onDamage(first,1,soldierID);
        require(aiInfantry->getAttackMode()==SABOTAGE,"campaign AI overwrote its capture mission while defending");
        for(const auto& player:enemy->getPlayerList()) player->onDamage(aiInfantry,1,soldierID);
        require(aiInfantry->getAttackMode()==SABOTAGE,"campaign AI abandoned capture when its infantry was attacked");
        aiInfantry->doSetAttackMode(STOP);
        enemyVehicle->doSetAttackMode(STOP);
        soldier->resolvePendingTargetRequest();
        require(soldier->getTarget()==first && soldier->getAttackMode()==SABOTAGE,"nearest valid enemy building was not acquired");
        require(soldier->getDestination()==first->getClosestPoint(soldier->getLocation()),"capture destination");
        soldier->doMove2Pos(Coord(3,4),true);
        require(soldier->getAttackMode()==GUARD && !soldier->getTarget(),"manual move does not cancel sabotage");
        soldier->doSetAttackMode(SABOTAGE);soldier->resolvePendingTargetRequest();
        first->setVisible(local->getTeamID(),false);soldier->update();soldier->resolvePendingTargetRequest();
        require(soldier->getTarget()==second,"invalid building did not trigger retargeting");
        first->setVisible(local->getTeamID(),true);soldier->doSetAttackMode(SABOTAGE);soldier->resolvePendingTargetRequest();
        require(soldier->getTarget()==first,"capture mode did not reset a previous target");
        for(int i=0;i<60;++i) CaptureHuntGameFixture::tick(*game);
        require(first->getHealth()==first->getMaxHealth(),"capture hunter fired at the building");
        require(game->saveGame(save),"capture hunt save");
        // Palace-generated saboteurs keep their ordinary destructive behavior.
        auto* victim=enemy->createUnit(Unit_Tank);victim->deploy(Coord(28,29));
        const auto victimID=victim->getObjectID(),saboteurID=saboteur->getObjectID();
        saboteur->doAttackObject(victim,true);saboteur->update();
        require(!game->getObjectManager().getObject(victimID) && !game->getObjectManager().getObject(saboteurID),"ordinary saboteur detonation changed");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(save),"capture hunt reload");
        auto* soldier=dynamic_cast<UnitBase*>(game->getObjectManager().getObject(soldierID));
        auto* first=dynamic_cast<StructureBase*>(game->getObjectManager().getObject(firstID));
        require(soldier && first && soldier->getAttackMode()==SABOTAGE && soldier->getTarget()==first,"saved mode/target lost");
        // Destroying a target must release the forced capture path and find another.
        first->setHealth(0);first->destroy();soldier->update();soldier->resolvePendingTargetRequest();
        auto* second=dynamic_cast<StructureBase*>(game->getObjectManager().getObject(secondID));
        require(second && soldier->getTarget()==second,"destroyed target left hunter stuck");
        second->setHealth(1);
        for(int i=0;i<5000 && game->getObjectManager().getObject(soldierID);++i) CaptureHuntGameFixture::tick(*game);
        require(!game->getObjectManager().getObject(soldierID),"hunter never reached/captured the building");
        auto* captured=currentGameMap->getTile(22,3)->getNonInfantryGroundObject();
        require(captured && captured->getItemID()==Structure_WindTrap && captured->getOwner()==game->getHouse(HOUSE_ATREIDES),"capture did not transfer building ownership");
        auto* trooper=game->getHouse(HOUSE_ATREIDES)->createUnit(Unit_Trooper);trooper->deploy(Coord(24,20));
        for(auto* structure:structureList) if(structure->getOwner()->getTeamID()!=trooper->getOwner()->getTeamID()) structure->setVisible(trooper->getOwner()->getTeamID(),false);
        trooper->doSetAttackMode(SABOTAGE);trooper->resolvePendingTargetRequest();
        require(!trooper->getTarget() && trooper->getAttackMode()==SABOTAGE,"empty map cancelled capture hunt");
        auto* fresh=game->getHouse(HOUSE_HARKONNEN)->placeStructure(NONE_ID,Structure_WindTrap,26,20,true,true);
        fresh->setHealth(1);const auto trooperID=trooper->getObjectID();
        for(int i=0;i<2000 && game->getObjectManager().getObject(trooperID);++i)CaptureHuntGameFixture::tick(*game);
        require(!game->getObjectManager().getObject(trooperID)
            && currentGameMap->getTile(26,20)->getNonInfantryGroundObject()->getOwner()==game->getHouse(HOUSE_ATREIDES),"idle trooper did not capture a newly available target");
        auto* healthy=game->getHouse(HOUSE_HARKONNEN)->placeStructure(NONE_ID,Structure_WindTrap,28,24,true,true);
        const auto health=healthy->getHealth();
        auto* attacker=game->getHouse(HOUSE_ATREIDES)->createUnit(Unit_Trooper);attacker->deploy(Coord(26,24));
        attacker->doSetAttackMode(SABOTAGE);attacker->resolvePendingTargetRequest();
        const auto attackerID=attacker->getObjectID();
        for(int i=0;i<2000 && game->getObjectManager().getObject(attackerID);++i) CaptureHuntGameFixture::tick(*game);
        require(!game->getObjectManager().getObject(attackerID) && healthy->getOwner()==game->getHouse(HOUSE_HARKONNEN)
            && healthy->getHealth()<health && healthy->getHealth()>0,"healthy-building attempt changed ordinary capture rules");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    options.immortalHumanPlayer=true;
    GameInitSettings immortal("capture-hunt-immortal",map,false,options);
    for(const auto& house:init.getHouseInfoList()) immortal.addHouseInfo(house);
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(immortal);
        UnitBase* soldier=nullptr;
        for(auto* unit:unitList) {
            if(unit->getLocation()==Coord(3,3)) soldier=unit;
            unit->doSetAttackMode(STOP);
        }
        require(soldier,"immortal infantry fixture");
        auto* first=game->getHouse(HOUSE_HARKONNEN)->placeStructure(NONE_ID,Structure_WindTrap,8,3,true,true);
        auto* second=game->getHouse(HOUSE_HARKONNEN)->placeStructure(NONE_ID,Structure_WindTrap,12,3,true,true);
        first->setHealth(1);second->setHealth(1);soldier->doSetAttackMode(SABOTAGE);
        for(int i=0;i<4000;++i) CaptureHuntGameFixture::tick(*game);
        require(soldier->getHealth()>0 && soldier->getAttackMode()==SABOTAGE
            && currentGameMap->getTile(8,3)->getNonInfantryGroundObject()->getOwner()==game->getHouse(HOUSE_ATREIDES)
            && currentGameMap->getTile(12,3)->getNonInfantryGroundObject()->getOwner()==game->getHouse(HOUSE_ATREIDES),"immortal infantry did not continue its capture hunt");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    SDL_Log("CAPTURE HUNT PASS: %s editor singles/five-unit squads, infantry-only UI below Retreat, mixed groups, valid targets, retargeting, movement/capture, AI orders, save/load, ordinary capture rules, immortality and unchanged saboteur",mod.c_str());
}
