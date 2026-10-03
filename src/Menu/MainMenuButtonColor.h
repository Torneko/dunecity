#pragma once

#include <globals.h>
#include <mod/ModManager.h>

namespace MainMenuButtonColor {

template<typename ButtonWidget>
inline void apply(ButtonWidget& button) {
    ModManager& modManager = ModManager::instance();
    if(!modManager.isInitialized()) {
        return;
    }

    const std::string activeMod = modManager.getActiveModName();
    if(activeMod == "Tornie") {
        button.setTextColor(getHouseColorRGB(HOUSE_REBELS, 3));
    } else if(activeMod == "Jericho") {
        button.setTextColor(getHouseColorRGB(HOUSECOLOR_CUSTOM_APPLE_GREEN, 3));
    }
}

} // namespace MainMenuButtonColor
