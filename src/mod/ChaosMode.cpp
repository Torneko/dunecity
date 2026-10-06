#include <mod/ChaosMode.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/Random.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

void ChaosMode::reset() {
    enabled = false;
    for(int house = 0; house < NUM_HOUSES; ++house) donors[house].fill(house);
}

bool ChaosMode::includesStructure(int item) {
    return std::find(Structures.begin(), Structures.end(), item) != Structures.end();
}

int ChaosMode::getTechnologyHouse(int house, int structure) const {
    if(!enabled || house < 0 || house >= NUM_HOUSES) return house;
    const auto found = std::find(Structures.begin(), Structures.end(), structure);
    return found == Structures.end() ? house : donors[house][found - Structures.begin()];
}

void ChaosMode::generate(ObjectData& objectData, bool requested, Uint32 seed) {
    reset();
    if(!requested) return;
    enabled = true;
    std::array<std::array<ObjectData::ObjectDataStruct, NUM_HOUSES>, Structures.size()> original;
    for(size_t index = 0; index < Structures.size(); ++index)
        std::copy(std::begin(objectData.data[Structures[index]]),
                  std::end(objectData.data[Structures[index]]), original[index].begin());
    // A private deterministic stream leaves terrain, reinforcement and combat RNG alone.
    Random draw(seed ^ 0x4348414fU);
    for(size_t index = 0; index < Structures.size(); ++index) {
        const int structure = Structures[index];
        for(int house = 0; house < NUM_HOUSES; ++house) {
            const auto& recipient = original[index][house];
            if(recipient.techLevel < 0) continue;
            std::vector<int> candidates;
            for(int donor = 0; donor < NUM_HOUSES; ++donor) {
                const auto& source = original[index][donor];
                if(source.enabled && source.builder != ItemID_Invalid
                   && source.techLevel == recipient.techLevel) candidates.push_back(donor);
            }
            // Prefer an actual exchange when another faction has this same-level building.
            if(candidates.size() > 1) candidates.erase(
                std::remove(candidates.begin(), candidates.end(), house), candidates.end());
            if(candidates.empty()) continue;
            const int donor = candidates[draw.rand(0, static_cast<int>(candidates.size()) - 1)];
            donors[house][index] = donor;
            objectData.data[structure][house] = original[index][donor];
        }
    }
}

void ChaosMode::save(OutputStream& stream) const {
    stream.writeBool(enabled);
    if(!enabled) return;
    stream.writeUint32(NUM_HOUSES);
    stream.writeUint32(static_cast<Uint32>(Structures.size()));
    for(size_t index = 0; index < Structures.size(); ++index) {
        stream.writeUint32(Structures[index]);
        for(int house = 0; house < NUM_HOUSES; ++house) stream.writeUint8(donors[house][index]);
    }
}

void ChaosMode::load(InputStream& stream) {
    reset();
    if(!stream.readBool()) return;
    if(stream.readUint32() != NUM_HOUSES || stream.readUint32() != Structures.size())
        throw std::runtime_error("Invalid Chaos Mode faction/building table");
    std::array<bool, Structures.size()> seen{};
    for(size_t entry = 0; entry < Structures.size(); ++entry) {
        const auto item = stream.readUint32();
        const auto found = std::find(Structures.begin(), Structures.end(), static_cast<int>(item));
        if(found == Structures.end()) throw std::runtime_error("Invalid Chaos Mode building");
        const auto index = static_cast<size_t>(found - Structures.begin());
        if(seen[index]) throw std::runtime_error("Duplicate Chaos Mode building");
        seen[index] = true;
        for(int house = 0; house < NUM_HOUSES; ++house) {
            const auto donor = stream.readUint8();
            if(donor >= NUM_HOUSES) throw std::runtime_error("Invalid Chaos Mode donor faction");
            donors[house][index] = donor;
        }
    }
    enabled = true;
}
