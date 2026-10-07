#ifndef COOP_CAMPAIGN_SESSION_H
#define COOP_CAMPAIGN_SESSION_H

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace coop {

constexpr int StageCount = 9;
constexpr int SlotCount = 5;
constexpr int MaxSlotCount = 7;
constexpr std::array<int, StageCount> OriginalMissionNumbers{1, 2, 5, 8, 11, 14, 17, 20, 22};

// A runtime house owns objects/commands; a faction supplies its rules and art.
// They can differ when both humans select the same faction.
struct Slot {
    int house = -1;
    int faction = -1;
    int color = -1;
    std::string playerName;
    std::string playerClass;
    int team() const { return playerClass == "HumanPlayer" ? 1 : 2; }
};

struct Context {
    std::string campaign = "common";
    std::string sessionId;
    std::string modName;
    // Missing fields in older metadata select the original generated templates.
    std::string mapLayout = "legacy";
    int sourceFaction = -1;
    int stage = 1;
    std::uint32_t completedMask = 0;
    std::uint32_t seed = 0;
    bool chaosEligible = false;
    std::vector<int> roster;
    std::vector<Slot> slots = std::vector<Slot>(SlotCount);
    std::vector<bool> enemyPresent = std::vector<bool>(SlotCount - 2, true);
    bool followsOriginalCampaign() const { return mapLayout == "original"; }
    bool isComplete() const { return stage == StageCount + 1; }
};

// Map data already travels in GameInitSettings and game saves, so this context
// needs no change to the binary save or network packet layout.
std::optional<Context> readContext(const std::string& mapData);
int factionForHouse(const Context& context, int runtimeHouse);
int mapSlotForHouse(const Context& context, int runtimeHouse);

class CoopCampaignSession {
public:
    static CoopCampaignSession create(const std::string& sessionId,
        const std::string& modName, const std::vector<int>& roster,
        const std::array<int, 2>& playerFactions,
        const std::array<std::string, 2>& playerNames,
        std::uint32_t seed, bool chaosEnabled, const std::string& mapLayout = "original");
    static CoopCampaignSession fromMapData(const std::string& mapData);
    static CoopCampaignSession loadProgress(const std::string& path);

    const Context& context() const { return context_; }
    bool isComplete() const { return context_.isComplete(); }
    std::string missionFilename() const;
    std::string prepareMap(const std::string& templateData) const;
    void completeMission(bool won);
    void reconfigurePlayers(const std::array<int, 2>& playerFactions,
        const std::array<std::string, 2>& playerNames);
    void setPlayerName(int playerSlot, const std::string& playerName);
    void setPlayerColor(int playerSlot, int color);
    void setChaosEligible(bool eligible);

    // The optional settings blob is the caller's serialized GameInitSettings.
    // Keeping it opaque avoids coupling progression to the engine's save format.
    void saveProgress(const std::string& path, const std::string& settingsBlob = "",
        const std::string& checkpointPath = "") const;
    const std::string& settingsBlob() const { return settingsBlob_; }
    const std::string& checkpointPath() const { return checkpointPath_; }

private:
    explicit CoopCampaignSession(Context context);
    void chooseOpponents();
    Context context_;
    std::string settingsBlob_;
    std::string checkpointPath_;
};

} // namespace coop
#endif
