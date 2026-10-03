#ifndef MAPPLAYERSECTIONUTILS_H
#define MAPPLAYERSECTIONUTILS_H

namespace MapPlayerSectionUtils {

/** Count numbered PlayerN sections across the fixed available-house capacity. */
template<typename HasSection>
int countNumberedPlayerSections(int availableHouseCapacity, HasSection&& hasSection) {
    int count = 0;
    for(int playerNumber = 1; playerNumber <= availableHouseCapacity; ++playerNumber) {
        if(hasSection(playerNumber)) {
            ++count;
        }
    }

    return count;
}

} // namespace MapPlayerSectionUtils

#endif // MAPPLAYERSECTIONUTILS_H
