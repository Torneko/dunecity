#pragma once
#include <structures/Refinery.h>
#include <structures/Scoutpost.h>
#include <structures/Palace.h>
#include <CursorManager.h>
#include <units/Harvester.h>
#include <units/Carryall.h>
#include <units/InfantryBase.h>
#include <MapEditor/MapEditor.h>

struct DoublefineryEditorFixture {
    static void draw(MapEditor& editor) {editor.drawMap(screenborder,false);}
};

inline void verifyDoublefinery538Graphics(const std::string& output, const std::string& mod) {
    auto require=[](bool value,const char* message) {
        if(!value) throw std::runtime_error(std::string("Doublefinery rendering: ")+message);
    };
    constexpr int width=5*D2_TILESIZE,height=2*D2_TILESIZE;
    SDL_RenderSetClipRect(renderer,nullptr);
    for(int house=0;house<NUM_HOUSE_COLOR_SLOTS;++house) {
        auto* preview=pGFXManager->getUIGraphicSurface(UI_MapEditor_Doublefinery,house);
        require(preview && preview->w==width && preview->h==height,"editor footprint");
        for(unsigned zoom=0;zoom<NUM_ZOOMLEVEL;++zoom) {
            const int scale=static_cast<int>(zoom)+1,w=width*scale,h=height*scale;
            auto* texture=pGFXManager->getZoomedObjPic(ObjPic_Doublefinery,house,zoom);
            int atlasWidth=0,atlasHeight=0;
            require(texture && SDL_QueryTexture(texture,nullptr,nullptr,&atlasWidth,&atlasHeight)==0
                && atlasWidth==10*w && atlasHeight==h,"ten-frame atlas dimensions");
            auto pixels=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,2*w,h,32,SDL_PIXELFORMAT_ARGB8888));
            require(pixels!=nullptr,"pixel surface");
            for(int frame=0;frame<10;++frame) {
                SDL_SetRenderDrawColor(renderer,3,7,11,255);SDL_RenderClear(renderer);
                SDL_Rect source{frame*w,0,w,h},destination{0,0,w,h},area{0,0,2*w,h};
                require(SDL_RenderCopy(renderer,texture,&source,&destination)==0,"frame draw");
                if(frame==2) {
                    SDL_Rect right{w,0,w,h};
                    if(zoom==0) require(SDL_RenderCopy(renderer,pGFXManager->getUIGraphic(UI_MapEditor_Doublefinery,house),nullptr,&right)==0,"editor draw");
                    else {
                        SDL_Rect occupied{8*w,0,w,h};
                        require(SDL_RenderCopy(renderer,texture,&occupied,&right)==0,"occupied frame draw");
                    }
                }
                require(SDL_RenderReadPixels(renderer,&area,pixels->format->format,pixels->pixels,pixels->pitch)==0,"frame capture");
                int leftPixels=0,rightPixels=0;
                for(int y=0;y<h;++y) {
                    const auto* row=reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(pixels->pixels)+y*pixels->pitch);
                    for(int x=0;x<w;++x) {
                        if((row[x]&0xffffff)!=0x03070b) (x<w/2?leftPixels:rightPixels)++;
                        if(frame==2 && zoom==0) require(row[x]==row[x+w],"editor differs from map sprite");
                    }
                }
                // Detect both an empty scaled surface and a missing refinery half.
                require(leftPixels>300*scale*scale && rightPixels>300*scale*scale,"building is empty or partially missing");
                if(frame==2 && house==HOUSE_HARKONNEN)
                    require(SDL_SaveBMP(pixels.get(),(std::filesystem::path(output)/(mod+"-doublefinery-zoom-"+std::to_string(zoom)+".bmp")).string().c_str())==0,"render preview save");
            }
        }
    }
    const auto previousZoom=currentZoomlevel;
    {
        MapEditor editor;
        editor.setMap(MapData(32,32,Terrain_Sand),MapInfo());
        screenborder->adjustScreenBorderToMapsize(32,32);
        for(int zoom=0;zoom<NUM_ZOOMLEVEL;++zoom) {
            currentZoomlevel=zoom;
            screenborder->adjustScreenBorderToMapsize(32,32);
            screenborder->setNewScreenCenter(Coord(10*TILESIZE,9*TILESIZE));
            editor.getStructureList().emplace_back(1,HOUSE_HARKONNEN,Structure_Doublefinery,256,Coord(8,8));
            SDL_SetRenderDrawColor(renderer,0,0,0,255);SDL_RenderClear(renderer);
            DoublefineryEditorFixture::draw(editor);
            SDL_Rect area{screenborder->world2screenX(8*TILESIZE),screenborder->world2screenY(8*TILESIZE),width*(zoom+1),height*(zoom+1)};
            require(area.x>=0 && area.y>=0 && area.x+area.w<=getRendererWidth() && area.y+area.h<=getRendererHeight(),"placed building lies outside capture");
            auto actual=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_ARGB8888));
            auto expected=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_ARGB8888));
            require(actual && expected && SDL_RenderReadPixels(renderer,&area,actual->format->format,actual->pixels,actual->pitch)==0,"placed editor building capture");
            auto scene=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,getRendererWidth(),getRendererHeight(),32,SDL_PIXELFORMAT_ARGB8888));
            require(scene && SDL_RenderReadPixels(renderer,nullptr,scene->format->format,scene->pixels,scene->pitch)==0,"editor scene capture");
            require(SDL_SaveBMP(scene.get(),(std::filesystem::path(output)/(mod+"-doublefinery-editor-map-"+std::to_string(zoom)+".bmp")).string().c_str())==0,"editor scene save");
            editor.getStructureList().clear();
            DoublefineryEditorFixture::draw(editor);
            SDL_Rect source{2*area.w,0,area.w,area.h};
            require(SDL_RenderCopy(renderer,pGFXManager->getZoomedObjPic(ObjPic_Doublefinery,HOUSE_HARKONNEN,zoom),&source,&area)==0,"expected placed building draw");
            require(SDL_RenderReadPixels(renderer,&area,expected->format->format,expected->pixels,expected->pitch)==0,"expected placed building capture");
            for(int y=0;y<area.h;++y)
                require(std::memcmp(static_cast<const Uint8*>(actual->pixels)+y*actual->pitch,static_cast<const Uint8*>(expected->pixels)+y*expected->pitch,area.w*4)==0,"placed editor building differs from its dedicated sprite");
        }
    }
    currentZoomlevel=previousZoom;
    SDL_Log("538 DOUBLEFINERY GRAPHICS PASS: %s, 21 colour slots, three zooms, ten nonempty complete frames, exact editor preview and placed map pixels",mod.c_str());
}

inline void verifyFollowup538(const std::string& output, const std::string& mod) {
    if(mod == "vanilla") return;
    verifyDoublefinery538Graphics(output,mod);
    auto require=[](bool value,const char* message) {
        if(!value) throw std::runtime_error(std::string("538 followup: ")+message);
    };
    std::string terrain="[BASIC]\nVersion=2\nTechLevel=7\n[MAP]\nSizeX=32\nSizeY=32\n";
    for(int y=0;y<32;++y) terrain+=fmt::sprintf("%03d=",y)+std::string(32,'%')+"\n";
    terrain+="[Player1]\nCredits=100000\n[Player2]\nCredits=0\n";
    auto options=effectiveGameOptions;
    options.chaosMode=false; options.instantBuild=true; options.immortalHumanPlayer=false;
    options.structuresDegradeOnConcrete=false; options.maximumNumberOfHarvestersOverride=3;
    auto makeSettings=[&]() {
        GameInitSettings init("538-followup",terrain,false,options);
        GameInitSettings::HouseInfo local(HOUSE_ATREIDES,1),enemy(HOUSE_HARKONNEN,2);
        local.addPlayerInfo({settings.general.playerName,HUMANPLAYERCLASS});
        enemy.addPlayerInfo({"Followup AI",DEFAULTAIPLAYERCLASS});
        init.addHouseInfo(local);init.addHouseInfo(enemy);return init;
    };
    for(bool groundCap : {false,true}) {
      options.maximumNumberOfUnitsOverride=groundCap?3:0;
      options.maximumNumberOfHarvestersOverride=groundCap?0:3;
      for(int freeSlots : {0,1,2}) {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(makeSettings());
        auto* house=game->getHouse(HOUSE_ATREIDES);
        auto* yard=dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID,Structure_ConstructionYard,2,2,true,true));
        require(yard,"construction yard");yard->setOriginalHouseID(HOUSE_ORDOS);
        for(int i=0;i<3-freeSlots;++i) house->createUnit(Unit_Harvester,true);
        const int before=house->getNumItems(Unit_Harvester);
        require(house->placeStructure(yard->getObjectID(),Structure_Doublefinery,7,3,false,true),"gift construction");
        require(house->getNumItems(Unit_Harvester)==before+freeSlots,"0/1/2 available gift slots");
      }
    }
    options.maximumNumberOfUnitsOverride=0;options.maximumNumberOfHarvestersOverride=3;
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    const auto save=(std::filesystem::path(output)/(mod+"-doublefinery-538.sav")).string();
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(makeSettings());
        auto* house=game->getHouse(HOUSE_ATREIDES);
        auto* enemy=game->getHouse(HOUSE_HARKONNEN);
        house->returnCredits(10000);
        for(int h=0;h<NUM_HOUSES;++h) {
            const char letter=getHouseScenarioLetter(static_cast<HOUSETYPE>(h));
            const bool allowed=letter=='O'||letter=='M'||letter=='N'||letter=='C';
            const auto& data=game->objectData.data[Structure_Doublefinery][h];
            require(data.enabled && data.techLevel==7 && data.hitpoints==900 && data.price==800
                && data.power==60 && data.capacity==2500,"Doublefinery statistics");
            require((data.builder==Structure_ConstructionYard)==allowed,"four eligible faction identities");
        }
        auto* yard=dynamic_cast<BuilderBase*>(house->placeStructure(NONE_ID,Structure_ConstructionYard,2,2,true,true));
        require(yard && !yard->isAvailableToBuild(Structure_Doublefinery),"native Atreides restriction");
        yard->setOriginalHouseID(HOUSE_ORDOS);
        require(!yard->isAvailableToBuild(Structure_Doublefinery),"missing prerequisites");
        house->placeStructure(NONE_ID,Structure_WindTrap,2,7,true,true);
        house->placeStructure(NONE_ID,Structure_Refinery,5,7,true,true);
        require(!yard->isAvailableToBuild(Structure_Doublefinery),"missing IX");
        house->placeStructure(NONE_ID,Structure_IX,9,7,true,true);yard->updateBuildList();
        require(yard->isAvailableToBuild(Structure_Doublefinery),"captured Ordos yard plans");
        game->techLevel=6;yard->updateBuildList();
        require(!yard->isAvailableToBuild(Structure_Doublefinery),"tech 6 restriction");
        game->techLevel=7;yard->updateBuildList();
        const int before=house->getNumItems(Unit_Harvester);
        auto* refinery=dynamic_cast<Refinery*>(house->placeStructure(NONE_ID,Structure_Doublefinery,14,7,false,true,HOUSE_ORDOS));
        require(refinery && refinery->canBeCaptured() && refinery->getStructureSizeX()==5
            && refinery->getStructureSizeY()==2,"capturable 5x2 footprint");
        require(getStructureSize(Structure_Doublefinery)==Coord(5,2),"placement dimensions");
        for(int y=0;y<2;++y) for(int x=0;x<5;++x)
            require(currentGameMap->getTile(14+x,7+y)->getNonInfantryGroundObject()==refinery,"ten occupied structure tiles");
        require(!currentGameMap->okayToPlaceStructure(28,20,5,2,false,nullptr),"right map edge must reject the fifth column");
        // Simulate the missing column in a prototype save, including an obstruction
        // in its second cell. A failed migration must not partially claim it.
        for(int y=0;y<2;++y) currentGameMap->getTile(18,7+y)->unassignNonInfantryGroundObject(refinery->getObjectID());
        auto* obstruction=enemy->placeStructure(NONE_ID,Structure_Wall,18,8,true,true);
        require(obstruction && !refinery->restoreLegacyDoublefineryFootprint(),"occupied legacy extension must be rejected");
        require(!currentGameMap->getTile(18,7)->hasAGroundObject()
            && currentGameMap->getTile(18,8)->getNonInfantryGroundObject()==obstruction,"legacy extension overwrote an obstacle or partially assigned");
        obstruction->destroy();
        require(refinery->restoreLegacyDoublefineryFootprint(),"free legacy extension");
        require(house->getNumItems(Unit_Harvester)==before,"no gift from capture/scenario recreation");
        for(int i=0;i<2;++i) {
            auto* harvester=static_cast<Harvester*>(house->createUnit(Unit_Harvester,true));
            harvester->setAmountOfSpice(100);harvester->setActive(false);harvester->setVisible(VIS_ALL,false);
            refinery->book();require(refinery->receiveHarvester(harvester),"second bay rejected");
        }
        require(!refinery->isFree() && refinery->getContainedHarvesters().size()==2,"two occupied bays");
        const auto credits=house->getStoredCredits();refinery->update();
        require(house->getStoredCredits()-credits==1.25_fix,"two simultaneous extraction streams");
        screenborder->setNewScreenCenter(refinery->getLocation()*TILESIZE+Coord(2*TILESIZE,TILESIZE));
        CaptureHuntGameFixture::start(*game);
        SDL_RenderSetClipRect(renderer,nullptr);game->drawScreen();
        auto scene=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,getRendererWidth(),getRendererHeight(),32,SDL_PIXELFORMAT_ARGB8888));
        require(scene && SDL_RenderReadPixels(renderer,nullptr,scene->format->format,scene->pixels,scene->pitch)==0,"in-game Doublefinery capture");
        require(SDL_SaveBMP(scene.get(),(std::filesystem::path(output)/(mod+"-doublefinery-game.bmp")).string().c_str())==0,"in-game Doublefinery preview");
        require(game->saveGame(save),"two occupied bays save");
        // The upgrade source follows a captured construction yard's technology.
        for(const auto item:{Structure_Scoutpost,Structure_Flamepost,Structure_Chemipost}) {
            auto* post=house->placeStructure(NONE_ID,item,2+(item%3)*2,14,true,true);
            require(post && !post->canBeCaptured(),"noncapturable Scoutpost family");
        }
        auto* post=static_cast<Scoutpost*>(house->placeStructure(NONE_ID,Structure_Scoutpost,10,14,true,true));
        post->setOriginalHouseID(getRuntimeHouseForIdentity(HOUSE_KLESHMERSH));
        require(post->canUpgradeToFlamepost(),"captured Kleshmersh upgrade plans");
        if(mod=="Tornie" || mod=="Jericho") {
            post->setOriginalHouseID(getRuntimeHouseForIdentity(HOUSE_THARPIQUE));
            require(post->canUpgradeToChemipost(),"captured Tharpique upgrade plans");
            game->techLevel=6;require(!post->canUpgradeToChemipost(),"Chemipost tier gate");game->techLevel=7;
            const auto baseline=game->objectData;
            bool flameDraw=false,chemiDraw=false;
            post->setOriginalHouseID(HOUSE_ATREIDES);
            for(Uint32 seed=0;seed<200 && !(flameDraw && chemiDraw);++seed) {
                game->objectData=baseline;
                CaptureHuntGameFixture::chaos(*game,game->objectData,seed);
                const auto donor=getHouseFactionIdentity(static_cast<HOUSETYPE>(post->getTechnologyHouseID()));
                require(post->isFlamepostUpgradeEligible()==(donor==HOUSE_KLESHMERSH),"Chaos Flamepost donor eligibility");
                require(post->isChemipostUpgradeEligible()==(donor==HOUSE_THARPIQUE),"Chaos Chemipost donor eligibility");
                flameDraw |= post->canUpgradeToFlamepost();chemiDraw |= post->canUpgradeToChemipost();
            }
            require(flameDraw && chemiDraw,"both upgrade paths can be drawn in Chaos");
            game->objectData=baseline;
            CaptureHuntGameFixture::resetChaos(*game);
        }
        auto* palace=static_cast<Palace*>(house->placeStructure(NONE_ID,Structure_Palace,20,3,true,true));
        palace->setOriginalHouseID(getRuntimeHouseForIdentity(HOUSE_REBELS));
        if(palace->usesTornieMainRebelsRandomSpecial()) {
            bool found=false;
            for(int draw=0;draw<30 && !found;++draw) {
                while(!palace->isSpecialWeaponReady()) palace->update();
                if(palace->usesTargetedSpecialWeapon()) found=true;
                else {palace->doSpecialWeapon();palace->update();}
            }
            require(found,"random missile draw");
            CursorManager cursor;
            require(cursor.canSetCursorMode(Game::CursorMode_Attack,{palace->getObjectID()}),"random missile cursor");
            const auto bullets=bulletList.size();palace->handleDeathhandClick(25,25);
            game->getCommandManager().executeCommands(game->getGameCycleCount());
            require(bulletList.size()==bullets+1 && !palace->isSpecialWeaponReady(),"targeted random missile command");
            require(!cursor.canSetCursorMode(Game::CursorMode_Attack,{palace->getObjectID()}),"spent missile still targetable");
        }
        require(enemy,"enemy fixture");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(save),"Doublefinery load");
        Refinery* refinery=nullptr;
        for(auto* building:structureList) if(building->getItemID()==Structure_Doublefinery) refinery=static_cast<Refinery*>(building);
        require(refinery && refinery->getContainedHarvesters().size()==2,"both occupants restored");
        const auto credits=refinery->getOwner()->getStoredCredits();refinery->update();
        require(refinery->getOwner()->getStoredCredits()-credits==1.25_fix,"two extractions after load");
        auto cargo=refinery->getContainedHarvesters();
        auto* first=static_cast<Harvester*>(cargo[0]);auto* second=static_cast<Harvester*>(cargo[1]);
        auto* carrier=static_cast<Carryall*>(refinery->getOwner()->createUnit(Unit_Carryall,true));
        second->bookCarrier(carrier);second->setGuardPoint({25,25});
        refinery->deployContainedHarvester(carrier);
        require(refinery->getContainedHarvesters().size()==1 && refinery->getContainedHarvester()==first,
            "carryall picked wrong bay");
        refinery->deployContainedHarvester();require(refinery->isFree(),"ground deployment from remaining bay");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    {
        auto game=std::make_unique<Game>();currentGame=game.get();require(game->loadSaveGame(save),"capture fixture load");
        Refinery* refinery=nullptr;
        for(auto* building:structureList) if(building->getItemID()==Structure_Doublefinery) refinery=static_cast<Refinery*>(building);
        require(refinery,"capture refinery fixture");
        for(auto* unit:refinery->getContainedHarvesters()) harvesterSetAmountOfSpice(unit,1000);
        auto* enemy=game->getHouse(HOUSE_HARKONNEN);
        const int before=enemy->getNumItems(Unit_Harvester);
        const Coord location=refinery->getLocation();refinery->setHealth(100);
        auto* infantry=static_cast<InfantryBase*>(enemy->placeUnit(Unit_Soldier,location.x-1,location.y,true));
        require(infantry,"capture infantry");const auto infantryID=infantry->getObjectID();infantry->doCaptureStructure(refinery);
        for(int tick=0;tick<2000;++tick) {
            CaptureHuntGameFixture::tick(*game);
            auto* building=currentGameMap->getTile(location)->getNonInfantryGroundObject();
            if(building && building->getOwner()==enemy) break;
        }
        auto* captured=dynamic_cast<Refinery*>(currentGameMap->getTile(location)->getNonInfantryGroundObject());
        auto* survivor=game->getObjectManager().getObject(infantryID);
        SDL_Log("538 CAPTURE CHECK: refinery=%d owner=%d bays=%d infantry=%d pos=%d,%d",captured!=nullptr,
            captured?captured->getOwner()->getHouseID():-1,captured?int(captured->getContainedHarvesters().size()):-1,
            survivor!=nullptr,survivor?survivor->getX():-1,survivor?survivor->getY():-1);
        require(captured && captured->getOwner()==enemy && captured->getContainedHarvesters().size()==2,
            "capture did not transfer both occupied bays");
        require(enemy->getNumItems(Unit_Harvester)==before+2,"capture duplicated construction gifts");
        for(auto* unit:captured->getContainedHarvesters())
            require(unit->getOwner()==enemy && harvesterGetAmountOfSpice(unit)>0,"captured cargo ownership/spice");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    if(const char* oldDirectory=std::getenv("DUNELEGACY_OLD_DOUBLE_DIR")) {
        const auto oldSave=(std::filesystem::path(oldDirectory)/(mod+"-doublefinery-538.sav")).string();
        auto game=std::make_unique<Game>();currentGame=game.get();
        require(game->loadSaveGame(oldSave),"prototype 9830 Doublefinery save migration");
        require(game->getLoadedSavegameVersion()==9830,"prototype fixture format");
        Refinery* refinery=nullptr;
        for(auto* building:structureList) if(building->getItemID()==Structure_Doublefinery) refinery=static_cast<Refinery*>(building);
        require(refinery && refinery->getStructureSizeX()==5 && refinery->getContainedHarvesters().size()==2,"legacy Doublefinery occupants or footprint");
        for(int y=0;y<2;++y) require(currentGameMap->getTile(refinery->getLocation()+Coord(4,y))->getNonInfantryGroundObject()==refinery,"legacy fifth column not restored");
        require(game->saveGame(save),"migrated 9831 checkpoint");
        SDL_Log("538 DOUBLEFINERY LEGACY PASS: %s 9830 prototype migrated to 5x2 with both occupants",mod.c_str());
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    SDL_Log("538 FOLLOWUP PASS: %s 5x2, faction/tech/IX gates, gifts 0/1/2, two unloading bays/save/load/carrier, Scoutposts and random missile cursor",mod.c_str());
}
