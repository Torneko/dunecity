#include <catch2/catch_all.hpp>
#include <Campaign/CoopCampaignSession.h>
#include <Campaign/CoopCampaignTransition.h>
#include <DataTypes.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>

namespace {
std::string mapTemplate(int stage) {
    return "[BASIC]\nVersion=2\nTechLevel=" + std::to_string(stage)
        + "\n[COOP_TEMPLATE]\nStage=" + std::to_string(stage)
        + "\n[Player1]\nCredits=1200\n[Player2]\nCredits=1200"
        "\n[Player3]\nCredits=500\n[Player4]\nCredits=500\n[Player5]\nCredits=500\n";
}

coop::CoopCampaignSession session(const std::array<int, 2>& factions = {1, 1}) {
    return coop::CoopCampaignSession::create("shared-123", "JerichoLite", {0, 1, 2, 3, 4, 5},
        factions, {"Host", "Guest"}, 0xabcdef01u, true, "legacy");
}

std::string originalTemplate(int stage, int sourceFaction,
    const std::vector<int>& opponents = {2, -1, -1}, const std::string& mod = "JerichoLite") {
    const int tech = mod == "vanilla" && stage == coop::StageCount ? 8 : stage;
    std::string data = "[BASIC]\nVersion=1\nTechLevel=" + std::to_string(tech)
        + "\nMapScale=1\n[MAP]\nSeed=12345\nField=1300,1400\n[COOP_TEMPLATE]\nSchema=1\nMod=" + mod
        + "\nStage=" + std::to_string(stage) + "\nParticipantCount=" + std::to_string(opponents.size() + 2)
        + "\nSourceFaction=" + std::to_string(sourceFaction)
        + "\nOriginalMission=" + std::to_string(coop::OriginalMissionNumbers[stage - 1]);
    for(std::size_t i = 0; i < opponents.size(); ++i)
        data += "\nEnemyFaction" + std::to_string(i + 1) + '=' + std::to_string(opponents[i]);
    for(std::size_t i = 1; i <= opponents.size() + 2; ++i)
        data += "\n[Player" + std::to_string(i) + "]\nCredits=" + (i < 3 ? "1200" : "0");
    return data + '\n';
}

coop::CoopCampaignSession originalSession(const std::string& mod = "JerichoLite") {
    const std::vector<int> roster = mod == "vanilla" ? std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8}
        : std::vector<int>{0, 1, 2, 3, 4, 5};
    return coop::CoopCampaignSession::create("original-123", mod, roster,
        {1, 2}, {"Host", "Guest"}, 0xabcdef01u, true);
}

void replace(std::string& text, const std::string& from, const std::string& to) {
    const auto position = text.find(from);
    REQUIRE(position != std::string::npos);
    text.replace(position, from.size(), to);
}

struct TemporaryDirectory {
    std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("dune-coop-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ~TemporaryDirectory() { std::error_code error; std::filesystem::remove_all(path, error); }
};
}

TEST_CASE("Coop same-faction humans keep independent runtime ownership", "[coop][campaign]") {
    const auto value = session();
    const auto& context = value.context();
    REQUIRE(context.slots[0].faction == 1);
    REQUIRE(context.slots[1].faction == 1);
    REQUIRE(context.slots[0].house != context.slots[1].house);
    REQUIRE(coop::factionForHouse(context, context.slots[1].house) == 1);
    REQUIRE(coop::mapSlotForHouse(context, context.slots[0].house) == 1);
    REQUIRE(coop::mapSlotForHouse(context, context.slots[1].house) == 2);
    std::set<int> houses;
    std::set<int> opponents;
    for(int i = 0; i < coop::SlotCount; ++i) {
        REQUIRE(houses.insert(context.slots[i].house).second);
        REQUIRE(context.slots[i].team() == (i < 2 ? 1 : 2));
        if(i >= 2) {
            REQUIRE(context.slots[i].faction != 1);
            REQUIRE(opponents.insert(context.slots[i].faction).second);
        }
    }
}

TEST_CASE("Coop replayed losses preserve stage and victories advance one common campaign", "[coop][campaign]") {
    auto host = session({1, 2});
    auto peer = session({1, 2});
    for(int stage = 1; stage <= coop::StageCount; ++stage) {
        REQUIRE(host.context().stage == stage);
        REQUIRE(host.missionFilename() == "coop0" + std::to_string(stage) + ".ini");
        REQUIRE(host.prepareMap(mapTemplate(stage)) == peer.prepareMap(mapTemplate(stage)));
        auto recovered = coop::CoopCampaignSession::fromMapData(host.prepareMap(mapTemplate(stage)));
        REQUIRE(recovered.context().sessionId == "shared-123");
        REQUIRE(recovered.context().seed == host.context().seed);
        REQUIRE(recovered.context().slots[0].playerName == "Host");
        REQUIRE(recovered.context().slots[1].playerName == "Guest");
        const auto previousMask = host.context().completedMask;
        host.completeMission(false);
        REQUIRE(host.context().stage == stage);
        REQUIRE(host.context().completedMask == previousMask);
        REQUIRE(host.prepareMap(mapTemplate(stage)) == peer.prepareMap(mapTemplate(stage)));
        host.completeMission(true);
        peer.completeMission(true);
    }
    REQUIRE(host.isComplete());
    REQUIRE(host.context().completedMask == 511);
    REQUIRE_THROWS_AS(host.completeMission(true), std::logic_error);
    REQUIRE_THROWS_AS(host.prepareMap(mapTemplate(9)), std::logic_error);
}

TEST_CASE("Coop metadata survives names and rejects invalid bindings/progression", "[coop][campaign]") {
    auto value = session();
    value.setPlayerName(0, "Tornie [cat] = \"bonjour\"");
    value.setPlayerColor(1, NUM_HOUSE_COLOR_SLOTS - 1);
    auto data = value.prepareMap(mapTemplate(1));
    const auto decoded = coop::readContext(data);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->slots[0].playerName == "Tornie [cat] = \"bonjour\"");
    REQUIRE(decoded->slots[1].color == NUM_HOUSE_COLOR_SLOTS - 1);
    REQUIRE_FALSE(coop::readContext(mapTemplate(1)).has_value());
    REQUIRE_THROWS_AS(value.prepareMap(mapTemplate(2)), std::invalid_argument);
    SECTION("Unproven stage advance") {
        replace(data, "Stage=1\nCompletedMask", "Stage=2\nCompletedMask");
        REQUIRE_THROWS_AS(coop::readContext(data), std::invalid_argument);
    }
    SECTION("Duplicate runtime house") {
        replace(data, "Slot2House=" + std::to_string(decoded->slots[1].house),
            "Slot2House=" + std::to_string(decoded->slots[0].house));
        REQUIRE_THROWS_AS(coop::readContext(data), std::invalid_argument);
    }
    SECTION("Opponent using the player faction") {
        replace(data, "Slot3Faction=" + std::to_string(decoded->slots[2].faction), "Slot3Faction=1");
        REQUIRE_THROWS_AS(coop::readContext(data), std::invalid_argument);
    }
    SECTION("Unsupported schema") {
        replace(data, "[COOP]\nSchema=1", "[COOP]\nSchema=2");
        REQUIRE_THROWS_AS(coop::readContext(data), std::invalid_argument);
    }
    REQUIRE_THROWS_AS(value.setPlayerName(0, "Bad\n[COOP]"), std::invalid_argument);
    REQUIRE_THROWS_AS(value.setPlayerColor(1, NUM_HOUSE_COLOR_SLOTS), std::invalid_argument);
}

TEST_CASE("Coop lobby reconfiguration retains shared progression", "[coop][campaign]") {
    auto value = session();
    value.completeMission(true);
    value.completeMission(true);
    value.reconfigurePlayers({3, 3}, {"Friend", "Host"});
    const auto& context = value.context();
    REQUIRE(context.stage == 3);
    REQUIRE(context.completedMask == 3);
    REQUIRE(context.seed == 0xabcdef01u);
    REQUIRE(context.sessionId == "shared-123");
    REQUIRE(context.slots[0].faction == 3);
    REQUIRE(context.slots[1].faction == 3);
    REQUIRE(context.slots[0].house != context.slots[1].house);
    value.setChaosEligible(false);
    value.setChaosEligible(true);
    REQUIRE_FALSE(value.context().chaosEligible);
}

TEST_CASE("Coop progress persistence preserves settings and recovers an interrupted replacement", "[coop][campaign]") {
    TemporaryDirectory directory;
    const auto path = directory.path / "nested" / "progress.ini";
    const std::string binarySettings("rules\0\1\2", 8);
    auto value = session();
    value.saveProgress(path.string(), binarySettings, "shared-123.dls");
    auto loaded = coop::CoopCampaignSession::loadProgress(path.string());
    REQUIRE(loaded.settingsBlob() == binarySettings);
    REQUIRE(loaded.checkpointPath() == "shared-123.dls");
    REQUIRE(loaded.context().stage == 1);
    value.completeMission(true);
    value.saveProgress(path.string(), "new rules", "");
    loaded = coop::CoopCampaignSession::loadProgress(path.string());
    REQUIRE(loaded.context().stage == 2);
    REQUIRE(loaded.context().completedMask == 1);
    REQUIRE(loaded.settingsBlob() == "new rules");
    REQUIRE(loaded.checkpointPath().empty());
    const auto backup = std::filesystem::path(path.string() + ".bak");
    std::filesystem::rename(path, backup);
    loaded = coop::CoopCampaignSession::loadProgress(path.string());
    REQUIRE(loaded.context().stage == 2);
    REQUIRE(loaded.settingsBlob() == "new rules");
    // A damaged replacement must not discard the last complete record.
    {
        std::ofstream damaged(path, std::ios::binary);
        damaged << "[COOP]\nSchema=1\nStage=9\n";
    }
    loaded = coop::CoopCampaignSession::loadProgress(path.string());
    REQUIRE(loaded.context().stage == 2);
    REQUIRE(loaded.settingsBlob() == "new rules");
}

TEST_CASE("Original co-op retains its source campaign after player reconfiguration and resume", "[coop][campaign]") {
    auto value = originalSession();
    REQUIRE(value.context().followsOriginalCampaign());
    REQUIRE(value.context().sourceFaction == 1);
    REQUIRE(value.missionFilename() == "faction1/coop01.ini");
    value.setPlayerColor(0, NUM_HOUSE_COLOR_SLOTS - 1);
    value.setPlayerColor(1, 12);
    const auto data = value.prepareMap(originalTemplate(1, 1));
    REQUIRE(data.find("MapScale=1\n") != std::string::npos);
    REQUIRE(data.find("Seed=12345\nField=1300,1400\n") != std::string::npos);
    value = coop::CoopCampaignSession::fromMapData(data);
    REQUIRE(value.context().slots[2].faction == value.context().slots[1].faction);
    REQUIRE(value.context().slots[2].house != value.context().slots[1].house);
    REQUIRE((value.context().enemyPresent == std::vector<bool>{true, false, false}));
    std::set<int> houses;
    for(const auto& slot : value.context().slots) REQUIRE(houses.insert(slot.house).second);
    value.completeMission(true);
    value.reconfigurePlayers({3, 3}, {"New host", "New guest"});
    REQUIRE(value.context().sourceFaction == 1);
    REQUIRE(value.context().stage == 2);
    REQUIRE(value.context().completedMask == 1);
    REQUIRE(value.context().slots[0].color == NUM_HOUSE_COLOR_SLOTS - 1);
    REQUIRE(value.context().slots[1].color == 12);
    REQUIRE(value.missionFilename() == "faction1/coop02.ini");
    // Original foes belong to the source campaign even if the new host picks one.
    value = coop::CoopCampaignSession::fromMapData(value.prepareMap(originalTemplate(2, 1, {3, 4, -1})));
    REQUIRE(value.context().slots[2].faction == 3);
    REQUIRE((value.context().enemyPresent == std::vector<bool>{true, true, false}));
    TemporaryDirectory directory;
    const auto path = directory.path / "original.ini";
    value.saveProgress(path.string(), "prepared settings", "original-123.dls");
    const auto resumed = coop::CoopCampaignSession::loadProgress(path.string());
    REQUIRE(resumed.context().sourceFaction == 1);
    REQUIRE(resumed.context().mapLayout == "original");
    REQUIRE(resumed.context().enemyPresent == value.context().enemyPresent);
    REQUIRE(resumed.context().stage == 2);
    REQUIRE(resumed.context().completedMask == 1);
    REQUIRE(resumed.settingsBlob() == "prepared settings");
    REQUIRE(resumed.checkpointPath() == "original-123.dls");
}

TEST_CASE("Original co-op rejects inconsistent campaign markers and missing participants", "[coop][campaign]") {
    const auto value = originalSession();
    auto data = originalTemplate(1, 1);
    SECTION("Wrong stage") { replace(data, "Stage=1", "Stage=2"); }
    SECTION("Wrong schema") { replace(data, "Schema=1", "Schema=2"); }
    SECTION("Wrong faction") { replace(data, "SourceFaction=1", "SourceFaction=3"); }
    SECTION("Wrong mission") { replace(data, "OriginalMission=1", "OriginalMission=2"); }
    SECTION("Wrong mod") { replace(data, "Mod=JerichoLite", "Mod=TornieLite"); }
    SECTION("Wrong technology") { replace(data, "TechLevel=1", "TechLevel=2"); }
    SECTION("Unavailable enemy") { replace(data, "EnemyFaction1=2", "EnemyFaction1=8"); }
    SECTION("Self opponent") { replace(data, "EnemyFaction1=2", "EnemyFaction1=1"); }
    SECTION("Repeated opponent") { replace(data, "EnemyFaction2=-1", "EnemyFaction2=2"); }
    SECTION("No opponent") { replace(data, "EnemyFaction1=2", "EnemyFaction1=-1"); }
    SECTION("Presence without faction") { replace(data, "EnemyFaction2=-1", "EnemyFaction2=-1\nEnemyPresent2=1"); }
    SECTION("Missing player") { replace(data, "[Player5]", "[UnexpectedPlayer]"); }
    REQUIRE_THROWS_AS(value.prepareMap(data), std::invalid_argument);
}

TEST_CASE("Original co-op follows nine source missions with the effective Vanilla final tech", "[coop][campaign]") {
    for(const std::string mod : {"vanilla", "JerichoLite"}) {
        auto value = originalSession(mod);
        for(int stage = 1; stage <= coop::StageCount; ++stage) {
            REQUIRE(value.missionFilename() == "faction1/coop0" + std::to_string(stage) + ".ini");
            const auto prepared = value.prepareMap(originalTemplate(stage, 1, {2, 3, 4}, mod));
            value = coop::CoopCampaignSession::fromMapData(prepared);
            REQUIRE(value.context().stage == stage);
            REQUIRE((value.context().enemyPresent == std::vector<bool>{true, true, true}));
            value.completeMission(true);
        }
        REQUIRE(value.isComplete());
        REQUIRE(value.context().completedMask == 511);
        REQUIRE(value.context().sourceFaction == 1);
    }
}

TEST_CASE("Original Jericho co-op keeps the active runtime faction as its campaign source", "[coop][campaign]") {
    // Jericho's Wildspade rules occupy runtime column6; no canonical remapping is done here.
    auto value = coop::CoopCampaignSession::create("jericho-runtime-source", "Jericho",
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {6, 6}, {"Host", "Guest"}, 123, false);
    REQUIRE(value.context().sourceFaction == 6);
    REQUIRE(value.missionFilename() == "faction6/coop01.ini");
    value = coop::CoopCampaignSession::fromMapData(value.prepareMap(originalTemplate(1, 6, {7, 8, -1}, "Jericho")));
    REQUIRE(value.context().slots[0].faction == 6);
    REQUIRE(value.context().slots[1].faction == 6);
    REQUIRE(value.context().slots[0].house != value.context().slots[1].house);
    value.reconfigurePlayers({10, 10}, {"Rejoined host", "Rejoined guest"});
    REQUIRE(value.context().sourceFaction == 6);
    REQUIRE(value.missionFilename() == "faction6/coop01.ini");
}

TEST_CASE("Original co-op preserves the faction of an enemy arriving through reinforcements", "[coop][campaign]") {
    const auto value = originalSession();
    auto data = originalTemplate(1, 1, {2, 3, -1});
    replace(data, "EnemyFaction2=3", "EnemyFaction2=3\nEnemyPresent2=0\nEnemyDeferred2=1");
    const auto prepared = coop::readContext(value.prepareMap(data));
    REQUIRE(prepared.has_value());
    REQUIRE(prepared->slots[3].faction == 3);
    REQUIRE_FALSE(prepared->enemyPresent[1]);
    REQUIRE(prepared->enemyPresent[0]);
    REQUIRE_FALSE(prepared->enemyPresent[2]);
}

TEST_CASE("Co-op without layout metadata retains its legacy templates and enemy rules", "[coop][campaign]") {
    const auto value = session({1, 2});
    auto data = value.prepareMap(mapTemplate(1));
    REQUIRE(data.find("MapLayout=") == std::string::npos);
    REQUIRE(data.find("SourceFaction=") == std::string::npos);
    const auto restored = coop::CoopCampaignSession::fromMapData(data);
    REQUIRE(restored.context().mapLayout == "legacy");
    REQUIRE(restored.context().sourceFaction == -1);
    REQUIRE(restored.missionFilename() == "coop01.ini");
    REQUIRE(restored.prepareMap(mapTemplate(1)) == data);
    replace(data, "Slot3Faction=" + std::to_string(restored.context().slots[2].faction), "Slot3Faction=2");
    REQUIRE_THROWS_AS(coop::readContext(data), std::invalid_argument);
}

TEST_CASE("Original Vanilla co-op preserves five simultaneous enemy armies", "[coop][campaign]") {
    auto value = originalSession("vanilla");
    const auto data = value.prepareMap(originalTemplate(1, 1, {2, 3, 4, 5, 6}, "vanilla"));
    value = coop::CoopCampaignSession::fromMapData(data);
    REQUIRE(value.context().slots.size() == 7);
    REQUIRE(value.context().enemyPresent.size() == 5);
    REQUIRE(coop::mapSlotForHouse(value.context(), value.context().slots[6].house) == 7);
    std::set<int> controlHouses;
    for(const auto& slot : value.context().slots) REQUIRE(controlHouses.insert(slot.house).second);
    REQUIRE(value.context().slots[2].faction == value.context().slots[1].faction);
    REQUIRE(value.context().slots[2].house != value.context().slots[1].house);
    value.reconfigurePlayers({6, 6}, {"Rejoined host", "Rejoined guest"});
    REQUIRE(value.context().slots.size() == 7);
    REQUIRE(value.context().sourceFaction == 1);
    value.completeMission(true);
    // Later source missions may require a different count and have empty IA slots.
    value = coop::CoopCampaignSession::fromMapData(value.prepareMap(originalTemplate(2, 1, {2, 3, -1, 5}, "vanilla")));
    REQUIRE(value.context().slots.size() == 6);
    REQUIRE((value.context().enemyPresent == std::vector<bool>{true, true, false, true}));
    const auto prepared = value.prepareMap(originalTemplate(2, 1, {2, 3, -1, 5}, "vanilla"));
    const auto revalidated = coop::readContext(coop::CoopCampaignSession::fromMapData(prepared).prepareMap(prepared));
    REQUIRE(revalidated.has_value());
    REQUIRE(revalidated->slots.size() == 6);
    REQUIRE(revalidated->enemyPresent == value.context().enemyPresent);
    TemporaryDirectory directory;
    const auto path = directory.path / "six-slots.ini";
    value.saveProgress(path.string());
    REQUIRE(coop::CoopCampaignSession::loadProgress(path.string()).context().slots.size() == 6);
}

TEST_CASE("Co-op participant limits reject truncated or oversized original maps", "[coop][campaign]") {
    auto value = originalSession("vanilla");
    auto data = originalTemplate(1, 1, {2, 3, 4, 5, 6}, "vanilla");
    SECTION("Too many participants") { replace(data, "ParticipantCount=7", "ParticipantCount=8"); }
    SECTION("Too few participants") { replace(data, "ParticipantCount=7", "ParticipantCount=4"); }
    SECTION("Missing final participant") { replace(data, "[Player7]", "[MissingPlayer]"); }
    SECTION("Missing final opponent marker") { replace(data, "EnemyFaction5=6", "UnknownFaction5=6"); }
    SECTION("Mod editions retain their three campaign opponents") {
        value = originalSession();
        data = originalTemplate(1, 1, {2, 3, 4, 5}, "JerichoLite");
    }
    REQUIRE_THROWS_AS(value.prepareMap(data), std::invalid_argument);
}

TEST_CASE("Coop transitions authenticate the agreed peer, session and mission", "[coop][network]") {
    coop::MissionBarrier barrier("shared-123", 2, "Guest");
    REQUIRE_FALSE(barrier.decision(coop::Outcome::Won, 9).has_value());
    REQUIRE_FALSE(barrier.receive("Stranger", "shared-123", 2, coop::Outcome::Won));
    REQUIRE_FALSE(barrier.receive("Guest", "other-session", 2, coop::Outcome::Won));
    REQUIRE_FALSE(barrier.receive("Guest", "shared-123", 1, coop::Outcome::Won));
    REQUIRE_FALSE(barrier.receive("Guest", "shared-123", 2, static_cast<coop::Outcome>(255)));
    REQUIRE_FALSE(barrier.decision(coop::Outcome::Won, 9).has_value());
    REQUIRE(barrier.receive("Guest", "shared-123", 2, coop::Outcome::Won));
    REQUIRE(barrier.decision(coop::Outcome::Won, 9) == coop::Advance::Next);
    REQUIRE(barrier.receive("Guest", "shared-123", 2, coop::Outcome::Won));
    REQUIRE(barrier.decision(coop::Outcome::Won, 9) == coop::Advance::Next);
    REQUIRE(barrier.receive("Guest", "shared-123", 2, coop::Outcome::Lost));
    REQUIRE(barrier.decision(coop::Outcome::Won, 9) == coop::Advance::Aborted);
}

TEST_CASE("Coop loss, departure and final victory cannot advance to an invalid mission", "[coop][network]") {
    coop::MissionBarrier ordinary("shared-123", 2, "Guest");
    REQUIRE(ordinary.receive("Guest", "shared-123", 2, coop::Outcome::Lost));
    REQUIRE(ordinary.decision(coop::Outcome::Lost, 9) == coop::Advance::Retry);
    REQUIRE_FALSE(ordinary.accepts(coop::Advance::Next, coop::Outcome::Lost, 9));
    REQUIRE_FALSE(ordinary.accepts(coop::Advance::Complete, coop::Outcome::Won, 9));
    REQUIRE(ordinary.decision(coop::Outcome::Aborted, 9) == coop::Advance::Aborted);
    coop::MissionBarrier final("shared-123", 9, "Guest");
    REQUIRE(final.receive("Guest", "shared-123", 9, coop::Outcome::Won));
    REQUIRE(final.decision(coop::Outcome::Won, 9) == coop::Advance::Complete);
    REQUIRE(final.accepts(coop::Advance::Complete, coop::Outcome::Won, 9));
    REQUIRE_FALSE(final.accepts(coop::Advance::Next, coop::Outcome::Won, 9));
    REQUIRE_FALSE(final.accepts(coop::Advance::Retry, coop::Outcome::Won, 9));
    REQUIRE(final.accepts(coop::Advance::Aborted, coop::Outcome::Won, 9));
}
