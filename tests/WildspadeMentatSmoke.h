#pragma once
#include <FileClasses/LoadSavePNG.h>
#include <Menu/MentatMenu.h>

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
        for(const auto& frame:animation->getFrames()) require(frame && frame->w==100 && frame->h==50,"UI overlay dimensions");
    }
    for(int state=0;state<5;++state) {
        auto composed=canonical(background);
        auto* e=eyes->getFrames()[state].get();auto* m=mouth->getFrames()[state].get();
        SDL_Rect er{info.eyesX,info.eyesY,100,50},mr{info.mouthX,info.mouthY,100,50};
        SDL_BlitSurface(e,nullptr,composed.get(),&er);SDL_BlitSurface(m,nullptr,composed.get(),&mr);
        require(SavePNG(composed.get(),(std::filesystem::path(output)/(mod+"-wildspade-"+std::to_string(state)+".png")).string().c_str())==0,"PNG animation capture");
        if(state==0) for(int y=0;y<400;++y)
            require(std::memcmp(static_cast<Uint8*>(composed->pixels)+y*composed->pitch,
                                static_cast<Uint8*>(a->pixels)+y*a->pitch,640*4)==0,"rest overlay seam");
    }
    // Compare actual menu pixels as well: Jericho's runtime slot is different
    // from its asset identity, so checking the loaded atlas alone misses aliases.
    {
        MentatMenu menu(getRuntimeHouseForIdentity(HOUSE_WILDSPADE));menu.setText("");
        eyes->setFrameOverride(0);mouth->setFrameOverride(0);
        SDL_RenderSetClipRect(renderer,nullptr);SDL_RenderClear(renderer);menu.draw();
        auto screen=sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0,getRendererWidth(),getRendererHeight(),32,SDL_PIXELFORMAT_RGBA32)};
        require(screen && SDL_RenderReadPixels(renderer,nullptr,screen->format->format,screen->pixels,screen->pitch)==0,"menu capture");
        for(int y=50;y<300;++y) for(int x=50;x<240;++x) {
            const auto* source=static_cast<Uint8*>(a->pixels)+y*a->pitch+x*4;
            const auto* target=static_cast<Uint8*>(screen->pixels)+(y+menu.getPosition().y)*screen->pitch+(x+menu.getPosition().x)*4;
            require(std::memcmp(source,target,3)==0,"wrong cat background in actual faction menu");
        }
        require(SavePNG(screen.get(),(std::filesystem::path(output)/(mod+"-wildspade-menu.png")).string().c_str())==0,"menu PNG output");
        eyes->resetFrameOverride();mouth->resetFrameOverride();
        menu.setText("Wildspade. Campaign briefing.");menu.update();menu.draw();
        require(mouth->getLoopsLeft()>0,"speech animation did not start");
    }
    const auto& neutral=ModManager::instance().getActiveMentatInfo(HOUSE_NEUTRAL);
    require(neutral.backgroundAsset=="ChaniMentat.png","Neutral mentat replaced accidentally");
    SDL_Log("WILDSPADE MENTAT PASS: %s, exact original background, five fitted eye/mouth frames and actual briefing menu",mod.c_str());
}
