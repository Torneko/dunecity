#pragma once
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/SFXManager.h>
#include <misc/sound_util.h>
#include <cstring>

inline void verifyJerichoHeralds(const std::string& output, const std::string& stage) {
    if(ModManager::instance().getActiveModName()!="Jericho")return;
    for(auto identity:{HOUSE_NEUTRAL,HOUSE_REBELS,HOUSE_WILDSPADE,HOUSE_KLESHMERSH}) {
        const char* name=identity==HOUSE_NEUTRAL ? "HeraldNeu.png" : identity==HOUSE_REBELS ? "HeraldRebels.png"
            : identity==HOUSE_WILDSPADE ? "HeraldWildspade.png" : "HeraldKleshmersh.png";
        auto expected=LoadPNG_RW(pFileManager->openFile(name).get());
        auto* actual=pGFXManager->getUIGraphicSurface(UI_Herald_Colored,getRuntimeHouseForIdentity(identity));
        if(!expected || !actual || expected->w!=actual->w || expected->h!=actual->h)
            throw std::runtime_error(std::string("Faction presentation check: herald dimensions for ")+name);
        auto a=sdl2::surface_ptr(SDL_ConvertSurfaceFormat(actual,SDL_PIXELFORMAT_ARGB8888,0));
        auto b=sdl2::surface_ptr(SDL_ConvertSurfaceFormat(expected.get(),SDL_PIXELFORMAT_ARGB8888,0));
        if(!a || !b)throw std::runtime_error("Faction presentation check: herald pixel conversion");
        for(int y=0;y<a->h;++y) {
            const auto* rowA=reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(a->pixels)+y*a->pitch);
            const auto* rowB=reinterpret_cast<const Uint32*>(static_cast<const Uint8*>(b->pixels)+y*b->pitch);
            for(int x=0;x<a->w;++x)if((rowA[x]&0xffffff)!=(rowB[x]&0xffffff))
                throw std::runtime_error(std::string("Faction presentation check: wrong faction herald ")+name);
        }
        if(SDL_SaveBMP(actual,(std::filesystem::path(output)/(std::string(name)+"-"+stage+".bmp")).string().c_str())!=0)
            throw std::runtime_error("Faction presentation check: herald capture");
    }
    SDL_Log("JERICHO HERALDS PASS: %s, Neutral/Rebels/Wildspade/Kleshmersh exact faction pixels",stage.c_str());
}

inline void verifyVanillaKleshmershVoice() {
    if(ModManager::instance().getActiveModName()!="vanilla")return;
    // The engine mounts voice PAKs for the startup language. Test each language
    // in a separate process rather than changing the language under FileManager.
    const auto& language=settings.general.language;
    auto expected=getChunkFromFile("KLESHMERSH.VOC");
    if(!expected || expected->alen==0)throw std::runtime_error("Faction presentation check: missing vanilla Kleshmersh voice");
    auto* actual=pSFXManager->getVoice(HouseHarkonnen,HOUSE_KLESHMERSH);
    auto* rebels=pSFXManager->getVoice(HouseHarkonnen,HOUSE_REBELS);
    if(!actual || actual->alen!=expected->alen || std::memcmp(actual->abuf,expected->abuf,actual->alen)!=0)
        throw std::runtime_error("Faction presentation check: Kleshmersh name voice replaced in "+language);
    if(!rebels || (rebels->alen==actual->alen && std::memcmp(rebels->abuf,actual->abuf,actual->alen)==0))
        throw std::runtime_error("Faction presentation check: Kleshmersh uses Rebels voice in "+language);
    if(language!="fr" && language!="de") {
        auto harvester=getChunkFromFile("HHARVEST.VOC");auto deployed=getChunkFromFile("HDEPLOY.VOC");
        auto announcement=concat3Chunks(expected.get(),harvester.get(),deployed.get());
        actual=pSFXManager->getVoice(HarvesterDeployed,HOUSE_KLESHMERSH);
        if(!actual || actual->alen!=announcement->alen || std::memcmp(actual->abuf,announcement->abuf,actual->alen)!=0)
            throw std::runtime_error("Faction presentation check: Kleshmersh deployment announces another house");
    }
    SDL_Log("VANILLA KLESHMERSH VOICE PASS: %s, own name and announcements",language.c_str());
}
