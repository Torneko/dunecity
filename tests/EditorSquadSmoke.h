#pragma once
#include <MapEditor/MapEditor.h>
#include <Trigger/ReinforcementTrigger.h>
#include <players/PlayerFactory.h>
#include <FileClasses/LoadSavePNG.h>
#include <fstream>
#include <iterator>

inline void verifyEditorUnitScrolling(const std::string& output, const std::string& mod) {
    auto require=[](bool value,const char* message) {
        if(!value) throw std::runtime_error(std::string("Editor unit scrolling: ")+message);
    };
    MapEditor editor;
    editor.setMap(MapData(32,32,Terrain_Rock),MapInfo());
    MapEditorInterface ui(&editor);
    ui.onModeButton(3);
    const int width=ui.editorModeUnits_MainVBox.getSize().x;
    for(const int screenHeight:{480,600,900}) {
        const int height=screenHeight-200;
        ui.editorModeUnits_MainVBox.resize(width,height);
        auto& view=ui.editorModeUnits_ScrollView;
        auto& content=ui.editorModeUnits_VBox;
        require(view.getSize().y==height,"unit viewport does not fill the available panel");
        const bool overflow=content.getMinimumSize().y>height;
        const int contentWidth=width-(overflow ? GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().x : 0);
        content.forEachChildWidget([&](Widget* child) {
            if(auto* row=dynamic_cast<HBox*>(child)) row->forEachChildWidget([&](Widget* cell) {
                if(auto* tile=dynamic_cast<SymbolButton*>(cell)) {
                    const auto pos=row->getWidgetPosition(tile);
                    require(pos.x>=0 && pos.x+tile->getSize().x<=contentWidth,"a unit tile is horizontally clipped beside the scrollbar");
                    require(pos.x % (2*D2_TILESIZE + 6)==0,"unit columns do not line up vertically");
                    require(tile->getSize().x==2*D2_TILESIZE+4,"unit cells have unequal widths");
                }
            });
        });
        require(view.handleMouseWheel(5,5,false)==overflow,"scrollbar/wheel visibility does not follow available height");
        for(int i=0;i<50;++i)view.handleMouseWheel(5,5,false);
        const int offset=std::max(0,content.getSize().y-height);
        const Point row=content.getWidgetPosition(&ui.editorModeUnits_HBoxTornieChemical);
        const Point button=ui.editorModeUnits_HBoxTornieChemical.getWidgetPosition(&ui.editorModeUnits_ChemicalCarryall);
        const int x=row.x+button.x+ui.editorModeUnits_ChemicalCarryall.getSize().x/2;
        const int y=row.y+button.y+ui.editorModeUnits_ChemicalCarryall.getSize().y/2-offset;
        require(y>=0 && y<height,"last unit remains outside the viewport");
        view.handleMouseLeft(x,y,true);view.handleMouseLeft(x,y,false);
        require(ui.editorModeUnits_ChemicalCarryall.getToggleState(),"last unit cannot be selected after scrolling");
        if(screenHeight==480) {
            SDL_RenderSetClipRect(renderer,nullptr);
            SDL_SetRenderDrawColor(renderer,20,20,20,255);SDL_RenderClear(renderer);
            ui.draw(Point(0,0));
            SDL_Rect area{getRendererWidth()-SIDEBARWIDTH,0,SIDEBARWIDTH,std::min(480,getRendererHeight())};
            auto pixels=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_ARGB8888));
            require(pixels && SDL_RenderReadPixels(renderer,&area,pixels->format->format,pixels->pixels,pixels->pitch)==0,"sidebar capture");
            require(SDL_SaveBMP(pixels.get(),(std::filesystem::path(output)/(mod+"-editor-units-scrolled.bmp")).string().c_str())==0,"sidebar preview save");
        }
        for(int i=0;i<50;++i)view.handleMouseWheel(5,5,true);
        const Point first=content.getWidgetPosition(&ui.editorModeUnits_HBox1);
        view.handleMouseLeft(ui.editorModeUnits_Soldier.getSize().x/2,first.y+ui.editorModeUnits_Soldier.getSize().y/2,true);
        view.handleMouseLeft(ui.editorModeUnits_Soldier.getSize().x/2,first.y+ui.editorModeUnits_Soldier.getSize().y/2,false);
        require(ui.editorModeUnits_Soldier.getToggleState(),"first unit cannot be selected after scrolling back");
    }
    if(ui.tornieContentVisible_) {
        ui.onModeButton(2);
        ui.changeHouseDropDown(mod == "Tornie" || mod == "Jericho" ? HOUSE_WILDSPADE : HOUSE_ATREIDES);
        SDL_RenderSetClipRect(renderer,nullptr);
        SDL_SetRenderDrawColor(renderer,176,126,16,255);SDL_RenderClear(renderer);
        auto& layout=ui.editorModeStructs_AdvancedLayout;
        layout.draw(Point(8,8));
        const Point size=layout.getMinimumSize();
        SDL_Rect area{0,0,size.x+16,size.y+16};
        auto pixels=sdl2::surface_ptr(SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SCREEN_FORMAT));
        require(pixels && SDL_RenderReadPixels(renderer,&area,pixels->format->format,pixels->pixels,pixels->pitch)==0,"building palette capture");
        require(SavePNG(pixels.get(),(std::filesystem::path(output)/(mod+"-editor-building-layout.png")).string().c_str())==0,"building palette preview save");
        ui.editorModeStructs_MainVBox.resize(ui.editorModeStructs_MainVBox.getSize().x,280);
        for(int i=0;i<50;++i)ui.editorModeStructs_ScrollView.handleMouseWheel(5,5,false);
        SDL_RenderClear(renderer);ui.draw(Point(0,0));
        auto scene=renderReadSurface(renderer);
        require(scene && SavePNG(scene.get(),(std::filesystem::path(output)/(mod+"-editor-building-sidebar.png")).string().c_str())==0,"building sidebar preview save");
    }
    SDL_Log("EDITOR UNIT SCROLL PASS: %s heights 480/600/900, scrollbar/wheel and first/last unit selection",mod.c_str());
}

inline void verifyEditorSquads(const std::string& output, const std::string& mod) {
    auto require=[](bool value,const char* message) {
        if(!value) throw std::runtime_error(std::string("Editor squads: ")+message);
    };
    const auto path=(std::filesystem::path(output)/(mod+"-editor-squads.ini")).string();
    {
        MapEditor editor;
        editor.setMap(MapData(32,32,Terrain_Rock),MapInfo());
        for(auto& p:editor.getPlayers()) {
            p.bActive=p.house==HOUSE_ATREIDES;p.bAnyHouse=false;
        }
        editor.setEditorMode(MapEditor::EditorMode(HOUSE_ATREIDES,Unit_Infantry5,256,RIGHT,GUARD));
        MapEditorUnitPlaceOperation(Coord(5,5),HOUSE_ATREIDES,Unit_Infantry5,256,RIGHT,GUARD).perform(&editor);
        editor.setEditorMode(MapEditor::EditorMode(HOUSE_ATREIDES,Unit_Troopers5,128,LEFT,AREAGUARD));
        MapEditorUnitPlaceOperation(Coord(8,8),HOUSE_ATREIDES,Unit_Troopers5,128,LEFT,AREAGUARD).perform(&editor);
        editor.getReinforcements().emplace_back(HOUSE_ATREIDES,Unit_Infantry5,Drop_North,1,false);
        editor.getReinforcements().emplace_back(HOUSE_ATREIDES,Unit_Troopers5,Drop_North,2,false);
        require(editor.getUnitList().size()==2,"placement did not create both group markers");
        editor.saveMap(path);editor.loadMap(path);
        require(editor.getUnitList().size()==2 && editor.getReinforcements().size()==2,"map roundtrip lost groups");
        const auto& soldiers=editor.getUnitList()[0];const auto& troopers=editor.getUnitList()[1];
        require(soldiers.itemID==Unit_Infantry5 && soldiers.position==Coord(5,5)
            && soldiers.health==256 && soldiers.angle==RIGHT && soldiers.attackmode==GUARD,"Soldier group fields changed");
        require(troopers.itemID==Unit_Troopers5 && troopers.position==Coord(8,8)
            && troopers.health==128 && troopers.angle==LEFT && troopers.attackmode==AREAGUARD,"Trooper group fields changed");
    }
    // The saved group marker expands into five ordinary units on a real game load.
    std::ifstream file(path);std::string map((std::istreambuf_iterator<char>(file)),{});
    GameInitSettings init("editor-squads-smoke",map,false,effectiveGameOptions);
    GameInitSettings::HouseInfo info(HOUSE_ATREIDES,1);
    info.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName,HUMANPLAYERCLASS));init.addHouseInfo(info);
    {
        auto game=std::make_unique<Game>();currentGame=game.get();game->initGame(init);
        auto* house=game->getHouse(HOUSE_ATREIDES);
        require(house && house->getNumItems(Unit_Soldier)==5 && house->getNumItems(Unit_Trooper)==5,
            "saved map did not deploy exactly five of each unit");
        int soldierDrops=0,trooperDrops=0;
        for(const auto& trigger:game->getTriggerManager().getTriggers()) {
            auto* drop=dynamic_cast<ReinforcementTrigger*>(trigger.get());if(!drop)continue;
            for(const auto id:drop->getDroppedUnits()) {
                if(id==Unit_Soldier)++soldierDrops;if(id==Unit_Trooper)++trooperDrops;
            }
        }
        require(soldierDrops==5 && trooperDrops==5,"reinforcement groups did not expand to five");
    }
    currentGame=nullptr;pLocalHouse=nullptr;pLocalPlayer=nullptr;
    for(int h=0;h<NUM_HOUSES;++h) {
        for(const unsigned id:{UI_MapEditor_Infantry5,UI_MapEditor_Troopers5}) {
            auto* quad=pGFXManager->getUIGraphicSurface(id==UI_MapEditor_Infantry5 ? UI_MapEditor_Infantry : UI_MapEditor_Troopers,h);
            auto* icon=pGFXManager->getUIGraphicSurface(id,h);
            require(quad && icon && quad->w==icon->w && quad->h==icon->h,"Squad icon dimensions changed");
            auto q=sdl2::surface_ptr(SDL_ConvertSurfaceFormat(quad,SDL_PIXELFORMAT_ARGB8888,0));
            auto i=sdl2::surface_ptr(SDL_ConvertSurfaceFormat(icon,SDL_PIXELFORMAT_ARGB8888,0));
            require(q && i,"icon conversion");
            int changed=0,blue=0;
            for(int y=0;y<i->h;++y) {
                auto* qr=reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(q->pixels)+y*q->pitch);
                auto* ir=reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(i->pixels)+y*i->pitch);
                for(int x=0;x<i->w;++x) {
                    if(qr[x]!=ir[x])++changed;
                    Uint8 r,g,b,a;SDL_GetRGBA(ir[x],i->format,&r,&g,&b,&a);
                    if(a && b>r+30 && b>g+20)++blue;
                    if(y<i->h/2)require(qr[x]==ir[x],"Squad base art changed above the marker");
                }
            }
            require(changed>0 && blue>0,"custom blue star is missing");
            if(h==HOUSE_ATREIDES) require(SDL_SaveBMP(icon,(std::filesystem::path(output)
                /(mod+"-editor-"+std::to_string(id)+".bmp")).string().c_str())==0,"icon preview save");
        }
    }
    SDL_Log("EDITOR SQUADS PASS: %s placement, map reload, five units/reinforcements, house-colored Infantry/Troopers Squad + blue star",mod.c_str());
}
