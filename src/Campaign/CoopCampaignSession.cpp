#include <Campaign/CoopCampaignSession.h>
#include <DataTypes.h>

#include <algorithm>
#include <charconv>
#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace coop {
namespace {
using Section = std::map<std::string, std::string>;
constexpr std::size_t MaxDataSize = 8 * 1024 * 1024;

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if(first == std::string::npos) return "";
    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}

std::string lower(std::string value) {
    for(char& c : value) if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return value;
}

std::optional<Section> readSection(const std::string& data, const std::string& name) {
    if(data.size() > MaxDataSize) throw std::invalid_argument("Coop data is too large");
    std::istringstream input(data);
    std::optional<Section> result;
    std::string line;
    bool active = false;
    while(std::getline(input, line)) {
        line = trim(line);
        if(line.empty() || line[0] == ';' || line[0] == '#') continue;
        if(line.front() == '[' && line.back() == ']') {
            active = lower(trim(line.substr(1, line.size() - 2))) == lower(name);
            if(active) {
                if(result) throw std::invalid_argument("Duplicate coop metadata section");
                result.emplace();
            }
        } else if(active) {
            const auto separator = line.find('=');
            if(separator == std::string::npos) throw std::invalid_argument("Invalid coop metadata line");
            const auto key = lower(trim(line.substr(0, separator)));
            if(key.empty() || !result->emplace(key, trim(line.substr(separator + 1))).second)
                throw std::invalid_argument("Duplicate or empty coop metadata key");
        }
    }
    return result;
}

const std::string& require(const Section& section, const std::string& key) {
    const auto found = section.find(lower(key));
    if(found == section.end()) throw std::invalid_argument("Missing coop field: " + key);
    return found->second;
}

template<typename Integer> Integer number(const std::string& text) {
    Integer value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if(result.ec != std::errc() || result.ptr != text.data() + text.size())
        throw std::invalid_argument("Invalid coop number");
    return value;
}

bool boolean(const std::string& text) {
    if(text == "1" || lower(text) == "true") return true;
    if(text == "0" || lower(text) == "false") return false;
    throw std::invalid_argument("Invalid coop boolean");
}

int participantCount(const Section& section) {
    const auto field = section.find("participantcount");
    const int count = field == section.end() ? SlotCount : number<int>(field->second);
    if(count < SlotCount || count > MaxSlotCount)
        throw std::invalid_argument("Invalid coop participant count");
    return count;
}

std::string hex(const std::string& value) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(value.size() * 2);
    for(const unsigned char c : value) {
        result.push_back(digits[c >> 4]);
        result.push_back(digits[c & 15]);
    }
    return result;
}

std::string unhex(const std::string& value) {
    if(value.size() % 2 != 0) throw std::invalid_argument("Invalid coop encoded text");
    const auto digit = [](char c) -> unsigned char {
        if(c >= '0' && c <= '9') return static_cast<unsigned char>(c - '0');
        if(c >= 'a' && c <= 'f') return static_cast<unsigned char>(c - 'a' + 10);
        if(c >= 'A' && c <= 'F') return static_cast<unsigned char>(c - 'A' + 10);
        throw std::invalid_argument("Invalid coop encoded text");
    };
    std::string result;
    result.reserve(value.size() / 2);
    for(std::size_t i = 0; i < value.size(); i += 2)
        result.push_back(static_cast<char>((digit(value[i]) << 4) | digit(value[i + 1])));
    return result;
}

bool safeIdentifier(const std::string& value) {
    return !value.empty() && value.size() <= 64
        && std::all_of(value.begin(), value.end(), [](char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-';
        });
}

bool validPlayerName(const std::string& value) {
    return !value.empty() && value.size() <= 64
        && std::none_of(value.begin(), value.end(), [](unsigned char c) { return c < 32 || c == 127; });
}

void validate(const Context& context) {
    if(context.campaign != "common" || !safeIdentifier(context.sessionId) || !safeIdentifier(context.modName))
        throw std::invalid_argument("Invalid coop campaign identity");
    if(context.stage < 1 || context.stage > StageCount + 1
       || context.completedMask != ((1u << (context.stage - 1)) - 1u))
        throw std::invalid_argument("Invalid coop campaign progression");
    std::set<int> roster;
    for(const int faction : context.roster) {
        if(faction < 0 || faction >= NUM_HOUSES || !roster.insert(faction).second)
            throw std::invalid_argument("Invalid coop faction roster");
    }
    if(context.slots.size() < SlotCount || context.slots.size() > MaxSlotCount
       || context.enemyPresent.size() != context.slots.size() - 2
       || ((context.modName != "vanilla" || !context.followsOriginalCampaign()) && context.slots.size() != SlotCount))
        throw std::invalid_argument("Invalid coop participant count");
    if(roster.size() < context.slots.size() || roster.size() > NUM_HOUSES)
        throw std::invalid_argument("Coop requires at least five available runtime houses");
    if(context.mapLayout != "legacy" && context.mapLayout != "original")
        throw std::invalid_argument("Invalid coop map layout");
    if(context.followsOriginalCampaign()) {
        if(roster.count(context.sourceFaction) == 0
           || std::none_of(context.enemyPresent.begin(), context.enemyPresent.end(), [](bool present) { return present; }))
            throw std::invalid_argument("Invalid coop campaign source");
    } else if(context.sourceFaction != -1
              || std::any_of(context.enemyPresent.begin(), context.enemyPresent.end(), [](bool present) { return !present; })) {
        throw std::invalid_argument("Invalid legacy coop campaign source");
    }
    std::set<int> houses;
    std::set<int> enemies;
    for(std::size_t i = 0; i < context.slots.size(); ++i) {
        const auto& slot = context.slots[i];
        if(roster.count(slot.house) == 0 || roster.count(slot.faction) == 0
           || !houses.insert(slot.house).second || slot.color < -1 || slot.color >= NUM_HOUSE_COLOR_SLOTS)
            throw std::invalid_argument("Invalid coop slot identity");
        if(i < 2) {
            if(slot.playerClass != "HumanPlayer" || !validPlayerName(slot.playerName))
                throw std::invalid_argument("Coop player slots must be human");
        } else {
            if(slot.playerClass != "CampaignAIPlayer" || !enemies.insert(slot.faction).second
               || (!context.followsOriginalCampaign()
                   && (slot.faction == context.slots[0].faction || slot.faction == context.slots[1].faction)))
                throw std::invalid_argument("Invalid coop opponent");
        }
    }
    if(context.slots[0].playerName == context.slots[1].playerName)
        throw std::invalid_argument("Coop players must have distinct network names");
    if(context.modName == "vanilla" && context.chaosEligible)
        throw std::invalid_argument("Vanilla does not support Chaos campaign progression");
}

void bindEnemySlots(Context& context, const std::vector<int>& factions,
    const std::vector<bool>& present) {
    if(factions.size() < SlotCount - 2 || factions.size() > MaxSlotCount - 2
       || factions.size() != present.size())
        throw std::invalid_argument("Invalid coop opponent count");
    context.slots.resize(factions.size() + 2);
    std::set<int> usedHouses{context.slots[0].house, context.slots[1].house};
    for(std::size_t i = 0; i < factions.size(); ++i) {
        auto& slot = context.slots[i + 2];
        slot.faction = factions[i];
        slot.house = factions[i];
        if(usedHouses.count(slot.house)) {
            const auto free = std::find_if(context.roster.begin(), context.roster.end(),
                [&usedHouses](int house) { return !usedHouses.count(house); });
            if(free == context.roster.end()) throw std::logic_error("No free coop runtime house");
            slot.house = *free;
        }
        usedHouses.insert(slot.house);
        slot.color = -1;
        slot.playerClass = "CampaignAIPlayer";
        slot.playerName = "Opponent " + std::to_string(i + 1);
    }
    context.enemyPresent = present;
    validate(context);
}

std::string serialize(const Context& context) {
    validate(context);
    std::ostringstream output;
    output << "[COOP]\nSchema=1\nCampaign=" << context.campaign
        << "\nSession=" << context.sessionId << "\nMod=" << context.modName
        << "\nStage=" << context.stage << "\nCompletedMask=" << context.completedMask
        << "\nSeed=" << context.seed << "\nChaosEligible=" << (context.chaosEligible ? 1 : 0)
        << "\nRoster=";
    for(std::size_t i = 0; i < context.roster.size(); ++i) {
        if(i) output << ',';
        output << context.roster[i];
    }
    output << '\n';
    if(context.followsOriginalCampaign()) {
        output << "MapLayout=original\nSourceFaction=" << context.sourceFaction
            << "\nParticipantCount=" << context.slots.size() << '\n';
        for(std::size_t i = 0; i < context.enemyPresent.size(); ++i)
            output << "EnemyPresent" << i + 1 << '=' << (context.enemyPresent[i] ? 1 : 0) << '\n';
    }
    for(std::size_t i = 0; i < context.slots.size(); ++i) {
        const auto& slot = context.slots[i];
        const auto key = "Slot" + std::to_string(i + 1);
        output << key << "House=" << slot.house << '\n'
            << key << "Faction=" << slot.faction << '\n'
            << key << "Color=" << slot.color << '\n'
            << key << "NameHex=" << hex(slot.playerName) << '\n'
            << key << "Class=" << slot.playerClass << '\n';
    }
    return output.str();
}

std::uint32_t nextRandom(std::uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

std::string withoutMetadata(const std::string& data) {
    std::istringstream input(data);
    std::ostringstream output;
    std::string line;
    bool skip = false;
    while(std::getline(input, line)) {
        const auto cleaned = trim(line);
        if(!cleaned.empty() && cleaned.front() == '[' && cleaned.back() == ']') {
            const auto section = lower(trim(cleaned.substr(1, cleaned.size() - 2)));
            skip = section == "coop" || section == "progress";
        }
        if(!skip) output << line << '\n';
    }
    return output.str();
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if(!input) throw std::runtime_error("Cannot open coop progress: " + path.string());
    input.seekg(0, std::ios::end);
    const auto size = input.tellg();
    if(size < 0 || static_cast<std::uint64_t>(size) > MaxDataSize)
        throw std::runtime_error("Invalid coop progress size");
    std::string data(static_cast<std::size_t>(size), '\0');
    input.seekg(0);
    if(!data.empty() && !input.read(&data[0], static_cast<std::streamsize>(data.size())))
        throw std::runtime_error("Cannot read coop progress");
    return data;
}
} // namespace

std::optional<Context> readContext(const std::string& mapData) {
    const auto section = readSection(mapData, "COOP");
    if(!section) return std::nullopt;
    if(require(*section, "Schema") != "1") throw std::invalid_argument("Unsupported coop schema");
    Context context;
    context.campaign = require(*section, "Campaign");
    context.sessionId = require(*section, "Session");
    context.modName = require(*section, "Mod");
    if(const auto layout = section->find("maplayout"); layout != section->end())
        context.mapLayout = layout->second;
    const int count = participantCount(*section);
    context.slots.resize(count);
    context.enemyPresent.resize(count - 2, true);
    if(context.followsOriginalCampaign()) {
        context.sourceFaction = number<int>(require(*section, "SourceFaction"));
        for(std::size_t i = 0; i < context.enemyPresent.size(); ++i) {
            const auto key = "enemypresent" + std::to_string(i + 1);
            if(const auto present = section->find(key); present != section->end())
                context.enemyPresent[i] = boolean(present->second);
        }
    }
    context.stage = number<int>(require(*section, "Stage"));
    context.completedMask = number<std::uint32_t>(require(*section, "CompletedMask"));
    context.seed = number<std::uint32_t>(require(*section, "Seed"));
    context.chaosEligible = boolean(require(*section, "ChaosEligible"));
    std::istringstream roster(require(*section, "Roster"));
    std::string entry;
    while(std::getline(roster, entry, ',')) context.roster.push_back(number<int>(trim(entry)));
    for(std::size_t i = 0; i < context.slots.size(); ++i) {
        auto& slot = context.slots[i];
        const auto key = "Slot" + std::to_string(i + 1);
        slot.house = number<int>(require(*section, key + "House"));
        slot.faction = number<int>(require(*section, key + "Faction"));
        slot.color = number<int>(require(*section, key + "Color"));
        slot.playerName = unhex(require(*section, key + "NameHex"));
        slot.playerClass = require(*section, key + "Class");
    }
    validate(context);
    return context;
}

int factionForHouse(const Context& context, int runtimeHouse) {
    for(const auto& slot : context.slots) if(slot.house == runtimeHouse) return slot.faction;
    return runtimeHouse;
}

int mapSlotForHouse(const Context& context, int runtimeHouse) {
    for(std::size_t i = 0; i < context.slots.size(); ++i)
        if(context.slots[i].house == runtimeHouse) return static_cast<int>(i) + 1;
    return -1;
}

CoopCampaignSession::CoopCampaignSession(Context context) : context_(std::move(context)) {
    validate(context_);
}

CoopCampaignSession CoopCampaignSession::create(const std::string& sessionId,
    const std::string& modName, const std::vector<int>& roster,
    const std::array<int, 2>& playerFactions, const std::array<std::string, 2>& playerNames,
    std::uint32_t seed, bool chaosEnabled, const std::string& mapLayout) {
    Context context;
    context.sessionId = sessionId;
    context.modName = modName;
    context.mapLayout = mapLayout;
    context.sourceFaction = context.followsOriginalCampaign() ? playerFactions[0] : -1;
    context.roster = roster;
    context.seed = seed;
    context.chaosEligible = chaosEnabled && modName != "vanilla";
    if(roster.size() < SlotCount) throw std::invalid_argument("Not enough coop factions");
    std::set<int> used;
    for(int i = 0; i < 2; ++i) {
        auto& slot = context.slots[i];
        slot.faction = playerFactions[i];
        slot.house = slot.faction;
        slot.playerName = playerNames[i];
        slot.playerClass = "HumanPlayer";
        if(std::find(roster.begin(), roster.end(), slot.faction) == roster.end())
            throw std::invalid_argument("Selected coop faction is unavailable");
        if(used.count(slot.house)) {
            const auto free = std::find_if(roster.begin(), roster.end(), [&used](int house) { return !used.count(house); });
            if(free == roster.end()) throw std::invalid_argument("No free coop runtime house");
            slot.house = *free;
        }
        used.insert(slot.house);
    }
    // Populate valid provisional enemy slots before the first validation.
    std::vector<int> enemies;
    for(const int faction : roster)
        if(faction != playerFactions[0] && faction != playerFactions[1]) enemies.push_back(faction);
    if(enemies.size() < 3) throw std::invalid_argument("Not enough coop opponents");
    for(int i = 2; i < SlotCount; ++i) {
        auto& slot = context.slots[i];
        slot.faction = enemies[i - 2];
        slot.house = slot.faction;
        if(used.count(slot.house)) {
            const auto free = std::find_if(roster.begin(), roster.end(), [&used](int house) { return !used.count(house); });
            if(free == roster.end()) throw std::invalid_argument("No free coop runtime house");
            slot.house = *free;
        }
        used.insert(slot.house);
        slot.playerClass = "CampaignAIPlayer";
        slot.playerName = "Opponent " + std::to_string(i - 1);
    }
    CoopCampaignSession session(std::move(context));
    session.chooseOpponents();
    return session;
}

CoopCampaignSession CoopCampaignSession::fromMapData(const std::string& mapData) {
    auto context = readContext(mapData);
    if(!context) throw std::invalid_argument("Not a coop campaign map");
    return CoopCampaignSession(std::move(*context));
}

std::string CoopCampaignSession::missionFilename() const {
    if(isComplete()) throw std::logic_error("Coop campaign is complete");
    return (context_.followsOriginalCampaign()
        ? "faction" + std::to_string(context_.sourceFaction) + "/" : "")
        + "coop0" + std::to_string(context_.stage) + ".ini";
}

std::string CoopCampaignSession::prepareMap(const std::string& templateData) const {
    if(isComplete()) throw std::logic_error("Coop campaign is complete");
    const auto basic = readSection(templateData, "BASIC");
    const auto marker = readSection(templateData, "COOP_TEMPLATE");
    if(!basic || !marker || number<int>(require(*marker, "Stage")) != context_.stage)
        throw std::invalid_argument("Coop map does not match campaign stage");
    auto prepared = context_;
    if(context_.followsOriginalCampaign()) {
        const int techLevel = context_.modName == "vanilla" && context_.stage == StageCount
            ? 8 : context_.stage;
        if(require(*marker, "Schema") != "1"
           || number<int>(require(*basic, "TechLevel")) != techLevel
           || number<int>(require(*marker, "SourceFaction")) != context_.sourceFaction
           || number<int>(require(*marker, "OriginalMission")) != OriginalMissionNumbers[context_.stage - 1]
           || require(*marker, "Mod") != context_.modName)
            throw std::invalid_argument("Coop map does not match the original campaign source");
        const int count = participantCount(*marker);
        if(count > SlotCount && (context_.modName != "vanilla" || context_.roster.size() < static_cast<std::size_t>(count)))
            throw std::invalid_argument("Unsupported original campaign participant count");
        std::vector<int> factions(count - 2);
        std::vector<bool> present(count - 2);
        std::set<int> usedFactions;
        for(std::size_t i = 0; i < factions.size(); ++i) {
            factions[i] = number<int>(require(*marker, "EnemyFaction" + std::to_string(i + 1)));
            present[i] = factions[i] != -1;
            const auto presence = marker->find("enemypresent" + std::to_string(i + 1));
            if(presence != marker->end()) present[i] = boolean(presence->second);
            const bool invalidFaction = factions[i] != -1
                && (std::find(context_.roster.begin(), context_.roster.end(), factions[i]) == context_.roster.end()
                    || factions[i] == context_.sourceFaction || !usedFactions.insert(factions[i]).second);
            if((factions[i] == -1 && present[i]) || invalidFaction)
                throw std::invalid_argument("Invalid original campaign opponent");
        }
        for(std::size_t i = 0; i < factions.size(); ++i) if(factions[i] == -1) {
            const auto free = std::find_if(context_.roster.begin(), context_.roster.end(),
                [&](int faction) { return !usedFactions.count(faction)
                    && faction != context_.sourceFaction && faction != context_.slots[0].faction
                    && faction != context_.slots[1].faction; });
            if(free == context_.roster.end()) throw std::invalid_argument("No spare inactive coop faction");
            factions[i] = *free;
            usedFactions.insert(*free);
        }
        bindEnemySlots(prepared, factions, present);
    } else if(number<int>(require(*basic, "TechLevel")) != context_.stage) {
        throw std::invalid_argument("Coop map does not match campaign stage");
    }
    for(std::size_t i = 1; i <= prepared.slots.size(); ++i)
        if(!readSection(templateData, "Player" + std::to_string(i)))
            throw std::invalid_argument("Coop map is missing a participant slot");
    return withoutMetadata(templateData) + '\n' + serialize(prepared);
}

void CoopCampaignSession::chooseOpponents() {
    std::vector<int> enemies;
    for(const int faction : context_.roster)
        if(faction != context_.slots[0].faction && faction != context_.slots[1].faction) enemies.push_back(faction);
    std::uint32_t random = context_.seed ^ 0x434f4f50u;
    if(random == 0) random = 1;
    for(std::size_t i = enemies.size(); i > 1; --i)
        std::swap(enemies[i - 1], enemies[nextRandom(random) % i]);
    std::set<int> used{context_.slots[0].house, context_.slots[1].house};
    for(int i = 2; i < SlotCount; ++i) {
        auto& slot = context_.slots[i];
        slot.faction = enemies[(context_.stage - 1 + i - 2) % enemies.size()];
        slot.house = slot.faction;
        if(used.count(slot.house)) {
            const auto free = std::find_if(context_.roster.begin(), context_.roster.end(),
                [&used](int house) { return !used.count(house); });
            if(free == context_.roster.end()) throw std::logic_error("No free coop runtime house");
            slot.house = *free;
        }
        used.insert(slot.house);
        slot.color = -1;
    }
    validate(context_);
}

void CoopCampaignSession::completeMission(bool won) {
    if(isComplete()) throw std::logic_error("Coop campaign is already complete");
    if(!won) return;
    context_.completedMask |= 1u << (context_.stage - 1);
    ++context_.stage;
    if(!isComplete() && !context_.followsOriginalCampaign()) chooseOpponents();
    validate(context_);
}

void CoopCampaignSession::setPlayerName(int playerSlot, const std::string& playerName) {
    if(playerSlot < 0 || playerSlot >= 2 || !validPlayerName(playerName)
       || playerName == context_.slots[1 - playerSlot].playerName)
        throw std::invalid_argument("Invalid coop player name");
    context_.slots[playerSlot].playerName = playerName;
}

void CoopCampaignSession::reconfigurePlayers(const std::array<int, 2>& playerFactions,
    const std::array<std::string, 2>& playerNames) {
    auto revised = create(context_.sessionId, context_.modName, context_.roster,
        playerFactions, playerNames, context_.seed, context_.chaosEligible, context_.mapLayout);
    revised.context_.sourceFaction = context_.sourceFaction;
    revised.context_.stage = context_.stage;
    revised.context_.completedMask = context_.completedMask;
    for(int i = 0; i < 2; ++i) revised.context_.slots[i].color = context_.slots[i].color;
    if(context_.followsOriginalCampaign()) {
        std::vector<int> enemies(context_.enemyPresent.size());
        for(std::size_t i = 0; i < enemies.size(); ++i) enemies[i] = context_.slots[i + 2].faction;
        bindEnemySlots(revised.context_, enemies, context_.enemyPresent);
    } else if(!revised.isComplete()) revised.chooseOpponents();
    context_.slots = std::move(revised.context_.slots);
}

void CoopCampaignSession::setPlayerColor(int playerSlot, int color) {
    if(playerSlot < 0 || playerSlot >= 2 || color < -1 || color >= NUM_HOUSE_COLOR_SLOTS)
        throw std::invalid_argument("Invalid coop player color");
    context_.slots[playerSlot].color = color;
}

void CoopCampaignSession::setChaosEligible(bool eligible) {
    // Disabling Chaos during the campaign cannot be repaired by re-enabling it.
    context_.chaosEligible = context_.chaosEligible && eligible;
}

void CoopCampaignSession::saveProgress(const std::string& path, const std::string& settingsBlob,
    const std::string& checkpointPath) const {
    const auto content = serialize(context_) + "\n[PROGRESS]\nSettingsHex=" + hex(settingsBlob)
        + "\nCheckpointHex=" + hex(checkpointPath) + '\n';
    if(content.size() > MaxDataSize) throw std::invalid_argument("Coop progress is too large");
    const std::filesystem::path target(path);
    if(!target.parent_path().empty()) std::filesystem::create_directories(target.parent_path());
    const auto temporary = std::filesystem::path(path + ".tmp");
    const auto backup = std::filesystem::path(path + ".bak");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if(!output || !output.write(content.data(), static_cast<std::streamsize>(content.size())) || !output.flush())
            throw std::runtime_error("Cannot write coop progress: " + path);
    }
    const bool hadTarget = std::filesystem::exists(target);
    if(hadTarget) {
        // Windows rename does not replace a file. Keep the previous complete
        // record recoverable until the new record has been installed.
        if(std::filesystem::exists(backup)) std::filesystem::remove(backup);
        std::filesystem::rename(target, backup);
    }
    try {
        std::filesystem::rename(temporary, target);
    } catch(...) {
        if(hadTarget && !std::filesystem::exists(target)) std::filesystem::rename(backup, target);
        throw;
    }
    if(hadTarget) {
        std::error_code cleanupError;
        std::filesystem::remove(backup, cleanupError);
    }
}

CoopCampaignSession CoopCampaignSession::loadProgress(const std::string& path) {
    std::exception_ptr failure;
    for(const std::filesystem::path candidate : {std::filesystem::path(path), std::filesystem::path(path + ".bak")}) {
        if(!std::filesystem::exists(candidate)) continue;
        try {
            const auto data = readFile(candidate);
            auto session = fromMapData(data);
            if(const auto progress = readSection(data, "PROGRESS")) {
                session.settingsBlob_ = unhex(require(*progress, "SettingsHex"));
                session.checkpointPath_ = unhex(require(*progress, "CheckpointHex"));
            }
            return session;
        } catch(...) {
            if(!failure) failure = std::current_exception();
        }
    }
    if(failure) std::rethrow_exception(failure);
    throw std::runtime_error("Cannot open coop progress: " + path);
}

} // namespace coop
