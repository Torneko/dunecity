#pragma once
#include <structures/BuilderBase.h>
#include <structures/Worfinery.h>
#include <GUI/dune/BuilderList.h>
#include <GUI/StaticContainer.h>
#include <FileClasses/LoadSavePNG.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <units/UnitBase.h>
#include <filesystem>
#include <stdexcept>

inline void verifyWorfinerySquad(const std::string& output, const std::string& mod) {
    auto require=[](bool value,const char* message) {
        if(!value) throw std::runtime_error(std::string("Worfinery squad: ")+message);
    };
    std::string terrain="[BASIC]\nVersion=2\nTechLevel=6\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y=0;y<32;++y) terrain+=fmt::sprintf("%03d=",y)+std::string(32,'%')+"\n";
    terrain+="[Player1]\nCredits=100000\n";
    auto options=effectiveGameOptions;options.instantBuild=true;
    GameInitSettings init("worfinery-squad-smoke",terrain,false,options);
    GameInitSettings::HouseInfo info(HOUSE_ATREIDES,1);
    info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName,HUMANPLAYERCLASS));init.addHouseInfo(info);
    const auto save=(std::filesystem::path(output)/(mod+"-squad5.sav")).string();
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* house=game->getHouse(HOUSE_ATREIDES);
        auto* worfinery=dynamic_cast<Worfinery*>(house->placeStructure(NONE_ID,Structure_Worfinery,4,4,true,true));
        require(worfinery,"fixture Worfinery missing");
        require(!worfinery->isAllowedToUpgrade() && !worfinery->isAvailableToBuild(Unit_Troopers5),"available at tech 6");
        game->techLevel=7;worfinery->updateBuildList();
        require(!worfinery->isAllowedToUpgrade() && !worfinery->doUpgrade(),"upgrade available without IX");
        require(house->placeStructure(NONE_ID,Structure_IX,13,4,true,true),"fixture IX missing");
        worfinery->updateBuildList();
        require(worfinery->isAllowedToUpgrade(),"no tech 7 / IX upgrade");
        require(!worfinery->isAvailableToBuild(Unit_Troopers5),"squad available before upgrade");
        require(worfinery->doUpgrade(),"upgrade did not start");worfinery->update();
        require(worfinery->getCurrentUpgradeLevel()==1,"upgrade did not complete");
        // Captured buildings retain their original house technology and price.
        for(int h=0;h<NUM_HOUSES;++h) {
            worfinery->setOriginalHouseID(h);
            const auto& list=worfinery->getBuildList();
            require(!list.empty() && list.front().itemID==Unit_Troopers5,"squad is not first for every house");
            require(list.front().price==3*game->objectData.data[Unit_Trooper][h].price,"wrong house price");
            require(worfinery->isAvailableToBuild(Unit_Troopers),"three-Trooper order removed");
        }
        worfinery->setOriginalHouseID(HOUSE_ATREIDES);
        {
            StaticContainer panel;
            auto* buttons=BuilderList::create(worfinery->getObjectID());
            panel.addWidget(buttons,Point(10,10),Point(WIDGET_WIDTH,400));
            const int count=worfinery->getBuildListSize();
            require(buttons->getItemIDFromIndex(0)==Unit_Troopers5,"five-Trooper order moved from the first button");
            require(buttons->getItemIDFromIndex(count-1)==Unit_Harvester,"Harvester is not the last displayed/clickable product");
            require(buttons->getItemIDFromIndex(count)==ItemID_Invalid,"out-of-range product lookup");
            SDL_RenderClear(renderer);panel.draw(Point(0,0));
            auto pixels=renderReadSurface(renderer);
            require(pixels && SavePNG(pixels.get(),(std::filesystem::path(output)/(mod+"-worfinery-order-538.png")).string().c_str())==0,"Worfinery order capture");
        }
        auto* wor=dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID,Structure_WOR,21,4,true,true));
        require(wor && !wor->isAvailableToBuild(Unit_Troopers5),"squad available from WOR");
        const int before=house->getNumItems(Unit_Trooper);
        worfinery->doProduceItem(Unit_Troopers5);
        for(int i=0;i<100 && house->getNumItems(Unit_Trooper)==before;++i) worfinery->update();
        require(house->getNumItems(Unit_Trooper)==before+5,"production did not deploy exactly five Troopers");
        worfinery->doProduceItem(Unit_Troopers5);
        require(game->saveGame(save),"queued squad save failed");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();
        require(game->loadSaveGame(save),"queued squad load failed");
        Worfinery* worfinery=nullptr;
        for(auto* structure:structureList) if(structure->getItemID()==Structure_Worfinery)
            worfinery=static_cast<Worfinery*>(structure);
        require(worfinery && worfinery->getCurrentUpgradeLevel()==1,"saved upgrade lost");
        require(worfinery->getCurrentProducedItem()==Unit_Troopers5,"saved squad order lost");
        {
            StaticContainer panel;
            auto* buttons=BuilderList::create(worfinery->getObjectID());
            panel.addWidget(buttons,Point(0,0),buttons->getMinimumSize());
            require(buttons->getItemIDFromIndex(worfinery->getBuildListSize()-1)==Unit_Harvester,"loaded Harvester button order");
            require(worfinery->getCurrentProducedItem()==Unit_Troopers5,"display ordering changed a saved production order");
        }
        const int before=worfinery->getOwner()->getNumItems(Unit_Trooper);
        for(int i=0;i<100 && worfinery->getOwner()->getNumItems(Unit_Trooper)==before;++i) worfinery->update();
        require(worfinery->getOwner()->getNumItems(Unit_Trooper)==before+5,"saved squad did not finish");
        game->techLevel=6;worfinery->updateBuildList();
        require(!worfinery->isAvailableToBuild(Unit_Troopers5),"upgraded squad bypasses tech 7");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    SDL_Log("WORFINERY SQUAD PASS: %s gates, all house prices/order, five Troopers, queued save/load",mod.c_str());
}

inline void verifyBarracksSquad(const std::string& output,const std::string& mod) {
    auto require=[](bool v,const char* m){if(!v)throw std::runtime_error(std::string("Barracks squad: ")+m);};
    std::string terrain="[BASIC]\nVersion=2\nTechLevel=3\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y=0;y<32;++y)terrain+=fmt::sprintf("%03d=",y)+std::string(32,'%')+"\n";
    terrain+="[Player1]\nCredits=100000\n";
    auto options=effectiveGameOptions;options.instantBuild=true;
    GameInitSettings init("barracks-squad-smoke",terrain,false,options);
    GameInitSettings::HouseInfo info(HOUSE_ATREIDES,1);
    info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName,HUMANPLAYERCLASS));init.addHouseInfo(info);
    const auto save=(std::filesystem::path(output)/(mod+"-soldiers5.sav")).string();
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* house=game->getHouse(HOUSE_ATREIDES);
        auto* barracks=dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID,Structure_Barracks,4,4,true,true));
        require(barracks,"fixture Barracks missing");
        require(!barracks->isAvailableToBuild(Unit_Infantry5),"five Soldiers available at tech 3");
        require(barracks->doUpgrade(),"normal first upgrade failed");barracks->update();
        require(barracks->getCurrentUpgradeLevel()==1 && !barracks->isAllowedToUpgrade(),"second upgrade available before tech 4");
        game->techLevel=4;barracks->updateBuildList();
        require(!barracks->isAvailableToBuild(Unit_Infantry5),"order available before second upgrade");
        require(house->getNumItems(Structure_IX)==0 && barracks->doUpgrade(),"tech 4 upgrade wrongly requires IX");barracks->update();
        require(barracks->getCurrentUpgradeLevel()==2,"second Barracks upgrade failed");
        for(int h=0;h<NUM_HOUSES;++h) {
            barracks->setOriginalHouseID(h);const auto& list=barracks->getBuildList();
            require(!list.empty() && list.front().itemID==Unit_Infantry5,"five Soldiers not first for every house");
            require(list.front().price==3*game->objectData.data[Unit_Soldier][h].price,"wrong single-Soldier house price x3");
        }
        barracks->setOriginalHouseID(HOUSE_ATREIDES);
        require(barracks->isAvailableToBuild(Unit_Infantry),"normal three Soldiers removed");
        require(!barracks->isAvailableToBuild(Unit_Troopers5),"five Troopers leaked into Barracks");
        {
            StaticContainer panel;
            auto* buttons=BuilderList::create(barracks->getObjectID());
            panel.addWidget(buttons,Point(10,10),Point(WIDGET_WIDTH,400));
            auto* single=buttons->getProductionPortrait(Unit_Soldier);
            auto* three=buttons->getProductionPortrait(Unit_Infantry);
            auto* five=buttons->getProductionPortrait(Unit_Infantry5);
            require(single && three && five,"missing Barracks production portrait variant");
            require(single!=three && single!=five && three!=five,"Barracks variants share the same texture");
            require(buttons->getItemIDFromIndex(0)==Unit_Infantry5,"five-Soldier button order");
            SDL_RenderClear(renderer);panel.draw(Point(0,0));
            auto pixels=renderReadSurface(renderer);
            require(pixels && SavePNG(pixels.get(),(std::filesystem::path(output)/(mod+"-barracks-portraits-538.png")).string().c_str())==0,"Barracks portrait capture");
        }
        const int before=house->getNumItems(Unit_Soldier);barracks->doProduceItem(Unit_Infantry5);
        for(int i=0;i<100 && house->getNumItems(Unit_Soldier)==before;++i)barracks->update();
        require(house->getNumItems(Unit_Soldier)==before+5,"order did not deploy exactly five Soldiers");
        barracks->doProduceItem(Unit_Infantry5);require(game->saveGame(save),"queued five Soldiers save failed");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(save),"queued five Soldiers load failed");
        BuilderBase* barracks=nullptr;
        for(auto* structure:structureList)if(structure->getItemID()==Structure_Barracks)barracks=static_cast<BuilderBase*>(structure);
        require(barracks && barracks->getCurrentProducedItem()==Unit_Infantry5 && barracks->getCurrentUpgradeLevel()==2,"saved Soldiers order/upgrade lost");
        const int before=barracks->getOwner()->getNumItems(Unit_Soldier);
        for(int i=0;i<100 && barracks->getOwner()->getNumItems(Unit_Soldier)==before;++i)barracks->update();
        require(barracks->getOwner()->getNumItems(Unit_Soldier)==before+5,"saved five Soldiers did not finish");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    SDL_Log("BARRACKS SQUAD PASS: %s tech 4/no IX, all house prices/order, five Soldiers, queued save/load",mod.c_str());
}
