#pragma once
#include <FileClasses/LoadSavePNG.h>
#include <Menu/BriefingMenu.h>

inline void verifyWildspadeMentat(const std::string& output, const std::string& mod) {
    if(mod != "Tornie" && mod != "Jericho") return;
    auto require=[](bool value,const std::string& message) {
        if(!value) throw std::runtime_error("Wildspade mentat check: "+message);
    };
    const auto& info=ModManager::instance().getActiveMentatInfo(HOUSE_WILDSPADE);
    require(info.enabled && info.backgroundAsset=="MentatCat_Wildspade.png", "dedicated override/house slot");
    auto original=LoadPNG_RW(pFileManager->openFile(info.backgroundAsset).get());
    auto* background=pGFXManager->getUIGraphicSurface(UI_MentatBackground,HOUSE_WILDSPADE);
    require(original && background && background->w==640 && background->h==400,"original background dimensions");
    auto canonical=[](SDL_Surface* source) {return sdl2::surface_ptr{SDL_ConvertSurfaceFormat(source,SDL_PIXELFORMAT_RGBA32,0)};};
    auto a=canonical(original.get()), b=canonical(background);
    for(int y=0;y<400;++y)
        require(std::memcmp(static_cast<Uint8*>(a->pixels)+y*a->pitch,
                            static_cast<Uint8*>(b->pixels)+y*b->pitch,640*4)==0,"background changed");
    auto* eyes=pGFXManager->getMentatEyesAnimation(HOUSE_WILDSPADE);
    auto* mouth=pGFXManager->getMentatMouthAnimation(HOUSE_WILDSPADE);
    for(auto* animation:{eyes,mouth}) {
        require(animation && animation->getNumberOfFrames()==5,"five animation states");
        const int w=animation==eyes ? info.eyesWidth : info.mouthWidth;
        const int h=animation==eyes ? info.eyesHeight : info.mouthHeight;
        for(const auto& frame:animation->getFrames()) require(frame && frame->w==w && frame->h==h,"UI overlay dimensions");
    }
    for(int state=0;state<5;++state) {
        auto composed=canonical(background);
        auto* e=eyes->getFrames()[state].get();auto* m=mouth->getFrames()[state].get();
        SDL_Rect er{info.eyesX,info.eyesY,info.eyesWidth,info.eyesHeight},mr{info.mouthX,info.mouthY,info.mouthWidth,info.mouthHeight};
        SDL_BlitSurface(e,nullptr,composed.get(),&er);SDL_BlitSurface(m,nullptr,composed.get(),&mr);
        require(SavePNG(composed.get(),(std::filesystem::path(output)/(mod+"-wildspade-"+std::to_string(state)+".png")).string().c_str())==0,"PNG animation capture");
        if(state==0) for(int y=0;y<400;++y)
            require(std::memcmp(static_cast<Uint8*>(composed->pixels)+y*composed->pitch,
                                static_cast<Uint8*>(a->pixels)+y*a->pitch,640*4)==0,"rest overlay seam");
        // The active patches must never redraw the nose or whisker roots.
        for(int y=178;y<200;++y) for(int x=160;x<235;++x)
            require(std::memcmp(static_cast<Uint8*>(composed->pixels)+y*composed->pitch+x*4,
                                static_cast<Uint8*>(a->pixels)+y*a->pitch+x*4,4)==0,"animation moved nose/whiskers");
        // Look variants retain both eye anchors instead of shifting the face.
        if(state>0 && state<4) {
            for(const SDL_Rect area : {SDL_Rect{142,145,29,18},SDL_Rect{205,145,19,21}}) {
                int green=0;
                for(int y=area.y;y<area.y+area.h;++y) for(int x=area.x;x<area.x+area.w;++x) {
                    const auto* p=static_cast<Uint8*>(composed->pixels)+y*composed->pitch+x*4;
                    if(p[1]>40 && p[1]>p[0]*1.3 && p[1]>p[2]*1.3) ++green;
                }
                require(green>10,"green iris outside original eye anchor");
            }
        }
    }
    // Compare actual menu pixels as well: Jericho's runtime slot is different
    // from its asset identity, so checking the loaded atlas alone misses aliases.
    {
        BriefingMenu menu(getRuntimeHouseForIdentity(HOUSE_WILDSPADE),15,BRIEFING);
        menu.setText("");menu.onMentatTextFinished();
        eyes->setFrameOverride(0);mouth->setFrameOverride(0);
        SDL_RenderSetClipRect(renderer,nullptr);SDL_RenderClear(renderer);menu.draw();
        auto screen=sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0,getRendererWidth(),getRendererHeight(),32,SDL_PIXELFORMAT_RGBA32)};
        require(screen && SDL_RenderReadPixels(renderer,nullptr,screen->format->format,screen->pixels,screen->pitch)==0,"menu capture");
        for(int y=50;y<300;++y) for(int x=50;x<240;++x) {
            const auto* source=static_cast<Uint8*>(a->pixels)+y*a->pitch+x*4;
            const auto* target=static_cast<Uint8*>(screen->pixels)+(y+menu.getPosition().y)*screen->pitch+(x+menu.getPosition().x)*4;
            require(std::memcmp(source,target,3)==0,"wrong cat background in actual faction menu");
        }
        require(pGFXManager->getMentatForeground(HOUSE_WILDSPADE)!=nullptr,"missing foreground layer");
        // These shoulder pixels lie inside the actual animated briefing area.
        for(int y=310;y<316;++y) for(int x=258;x<264;++x) {
            const auto* source=static_cast<Uint8*>(a->pixels)+y*a->pitch+x*4;
            const auto* target=static_cast<Uint8*>(screen->pixels)+(y+menu.getPosition().y)*screen->pitch+(x+menu.getPosition().x)*4;
            require(std::memcmp(source,target,3)==0,"briefing animation covers shoulder");
        }
        const auto* video=static_cast<Uint8*>(screen->pixels)+(150+menu.getPosition().y)*screen->pitch+(400+menu.getPosition().x)*4;
        require(video[0] || video[1] || video[2],"foreground obscures briefing video");
        require(SavePNG(screen.get(),(std::filesystem::path(output)/(mod+"-wildspade-menu.png")).string().c_str())==0,"menu PNG output");
        for(int state=1;state<5;++state) {
            eyes->setFrameOverride(state);mouth->setFrameOverride(state);menu.draw();
            require(SDL_RenderReadPixels(renderer,nullptr,screen->format->format,screen->pixels,screen->pitch)==0,"active briefing capture");
            require(SavePNG(screen.get(),(std::filesystem::path(output)/(mod+"-wildspade-briefing-"+std::to_string(state)+".png")).string().c_str())==0,"active briefing PNG");
        }
        eyes->resetFrameOverride();mouth->resetFrameOverride();
        menu.setText("Wildspade. Campaign briefing.");menu.update();menu.draw();
        require(mouth->getLoopsLeft()>0,"speech animation did not start");
    }
    const auto& neutral=ModManager::instance().getActiveMentatInfo(HOUSE_NEUTRAL);
    require(neutral.backgroundAsset=="ChaniMentat.png","Neutral mentat replaced accidentally");
    const auto& paul=ModManager::instance().getActiveMentatInfo(HOUSE_ATREIDES);
    require(!paul.foregroundAsset.empty(),"Paul foreground configuration missing");
    auto foreground=LoadPNG_RW(pFileManager->openFile(paul.foregroundAsset).get());
    auto paulPixels=canonical(foreground.get());
    BriefingMenu paulMenu(getRuntimeHouseForIdentity(HOUSE_ATREIDES),15,BRIEFING);
    paulMenu.setText("");paulMenu.onMentatTextFinished();
    SDL_RenderClear(renderer);paulMenu.draw();
    auto screen=sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0,getRendererWidth(),getRendererHeight(),32,SDL_PIXELFORMAT_RGBA32)};
    require(screen && SDL_RenderReadPixels(renderer,nullptr,screen->format->format,screen->pixels,screen->pitch)==0,"Paul briefing capture");
    int checked=0;
    for(int y=260;y<320;++y) for(int x=256;x<320;++x) {
        const auto* source=static_cast<Uint8*>(paulPixels->pixels)+y*paulPixels->pitch+x*4;
        if(source[3]!=255) continue;
        const auto* target=static_cast<Uint8*>(screen->pixels)+(y+paulMenu.getPosition().y)*screen->pitch+(x+paulMenu.getPosition().x)*4;
        require(std::memcmp(source,target,3)==0,"Paul foreground regression");++checked;
    }
    require(checked>10,"Paul comparison did not cover overlapping shoulder");
    SDL_Log("WILDSPADE MENTAT PASS: %s, original nose/whiskers, stable eye anchors, actual animated BriefingMenu and shoulder foreground",mod.c_str());
}
