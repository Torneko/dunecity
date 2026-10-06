#include <catch2/catch_all.hpp>

#include <INIMap/MapPlayerSectionUtils.h>
#include <mod/CustomHouseConfig.h>
#include <mod/ModInfo.h>
#include <mod/ModMentatConfig.h>
#include <globals.h>
#include <SpecialVehicle.h>

#include <array>
#include <string>
#include <vector>

TEST_CASE("CustomHouse config: valid numeric fields parse without throwing",
          "[custom-house][config]") {
    int value = -1;
    REQUIRE(CustomHouseConfig::parseInteger("144", value));
    REQUIRE(value == 144);

    REQUIRE(CustomHouseConfig::parseInteger("0", value));
    REQUIRE(value == 0);
}

TEST_CASE("CustomHouse config: malformed numeric fields are rejected without mutation",
          "[custom-house][config][regression]") {
    const std::array<std::string, 5> malformedValues = {
        "",
        "not-a-number",
        "144trailing",
        "144 ",
        "999999999999999999999999999999"
    };

    for(const std::string& malformedValue : malformedValues) {
        CAPTURE(malformedValue);
        int destination = 73;
        REQUIRE_FALSE(CustomHouseConfig::parseInteger(malformedValue, destination));
        REQUIRE(destination == 73);
    }
}

TEST_CASE("PlayerN-only map scans the fixed available-house capacity",
          "[map][players][regression]") {
    constexpr int availableHouseCapacity = 8;
    int sectionsScanned = 0;

    const int numberedPlayerCount = MapPlayerSectionUtils::countNumberedPlayerSections(
        availableHouseCapacity,
        [&sectionsScanned](int playerNumber) {
            ++sectionsScanned;
            return playerNumber == 1 || playerNumber == 8;
        });

    REQUIRE(sectionsScanned == availableHouseCapacity);
    REQUIRE(numberedPlayerCount == 2);
    REQUIRE(0 + numberedPlayerCount == 2);
}

TEST_CASE("Mod Mentat numeric parsing is nonthrowing and rejects malformed values",
          "[mod][mentat][config]") {
    double frameRate = 0.5;
    REQUIRE(ModMentatConfig::parseDouble("5.25", frameRate));
    REQUIRE(frameRate == Catch::Approx(5.25));

    const std::array<std::string, 5> malformedValues = {
        "", "not-a-number", "5.0trailing", "nan", "1e9999"
    };
    for(const std::string& malformedValue : malformedValues) {
        CAPTURE(malformedValue);
        double destination = 3.0;
        REQUIRE_FALSE(ModMentatConfig::parseDouble(malformedValue, destination));
        REQUIRE(destination == Catch::Approx(3.0));
    }
}

TEST_CASE("Mod Mentat assets must use portable relative paths",
          "[mod][mentat][config][security]") {
    REQUIRE(ModMentatConfig::isSafeAssetPath("mentat/custom/background.png"));
    REQUIRE(ModMentatConfig::isSafeAssetPath("MentatEyes.png"));
    REQUIRE_FALSE(ModMentatConfig::isSafeAssetPath("../other-mod/asset.png"));
    REQUIRE_FALSE(ModMentatConfig::isSafeAssetPath("mentat/../asset.png"));
    REQUIRE_FALSE(ModMentatConfig::isSafeAssetPath("C:/local/asset.png"));
    REQUIRE_FALSE(ModMentatConfig::isSafeAssetPath("mentat\\asset.png"));
}

TEST_CASE("Invalid optional Mentat fields disable the override safely",
          "[mod][mentat][config][fallback]") {
    ModMentatInfo valid;
    valid.enabled = true;
    valid.identityHouse = 1;
    valid.backgroundAsset = "mentat/background.png";
    valid.foregroundAsset = "mentat/foreground.png";
    valid.eyesAsset = "mentat/eyes.png";
    valid.mouthAsset = "mentat/mouth.png";
    REQUIRE(ModMentatConfig::isValid(valid));

    ModMentatInfo invalidIdentity = valid;
    invalidIdentity.identityHouse = 8;
    REQUIRE_FALSE(ModMentatConfig::isValid(invalidIdentity));

    ModMentatInfo invalidFrames = valid;
    invalidFrames.eyesFrames = 0;
    REQUIRE_FALSE(ModMentatConfig::isValid(invalidFrames));

    ModMentatInfo invalidPath = valid;
    invalidPath.backgroundAsset = "../leaked.png";
    REQUIRE_FALSE(ModMentatConfig::isValid(invalidPath));

    ModMentatInfo invalidForegroundPath = valid;
    invalidForegroundPath.foregroundAsset = "../other-mod/foreground.png";
    REQUIRE_FALSE(ModMentatConfig::isValid(invalidForegroundPath));
}

TEST_CASE("Generated mentat layouts reject incomplete or unsafe rectangles",
          "[mod][mentat][config]") {
    ModMentatInfo info;
    info.eyesCropHeight = 234; info.eyesWidth = 100; info.eyesHeight = 50;
    REQUIRE(ModMentatConfig::isValid(info));
    info.eyesHeight = 0;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
    info.eyesHeight = 50; info.eyesCropY = -1;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
    info.eyesCropY = 0; info.restFromBackground = true;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
    info.backgroundAsset = "cat.png";
    info.eyesX = 132; info.eyesY = 135; info.mouthX = 147; info.mouthY = 188;
    info.mouthCropHeight = 190; info.mouthWidth = 100; info.mouthHeight = 50;
    info.doubleEyes = false; info.doubleMouth = false;
    REQUIRE(ModMentatConfig::isValid(info));
    info.mouthWidth = 100000;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
}

TEST_CASE("Custom-house presentation numbers parse safely and remain bounded",
          "[custom-house][presentation][config]") {
    double value = 1.0;
    REQUIRE(CustomHouseConfig::parseDouble("1.15", value));
    REQUIRE(value == Catch::Approx(1.15));
    REQUIRE(CustomHouseConfig::isValidVoicePlaybackRate(1.06));
    REQUIRE(CustomHouseConfig::isValidVoiceGain(1.15));
    REQUIRE_FALSE(CustomHouseConfig::isValidVoicePlaybackRate(0.0));
    REQUIRE_FALSE(CustomHouseConfig::isValidVoiceGain(5.0));

    const std::array<std::string, 5> malformedValues = {
        "", "not-a-number", "1.0trailing", "nan", "1e9999"
    };
    for(const std::string& malformedValue : malformedValues) {
        CAPTURE(malformedValue);
        double destination = 1.0;
        REQUIRE_FALSE(CustomHouseConfig::parseDouble(malformedValue, destination));
        REQUIRE(destination == Catch::Approx(1.0));
    }
}

TEST_CASE("Custom-house presentation assets use portable mod-relative paths",
          "[custom-house][presentation][config][security]") {
    REQUIRE(CustomHouseConfig::isSafeAssetPath("presentation/herald.png"));
    REQUIRE(CustomHouseConfig::isSafeAssetPath("HouseName.VOC"));
    REQUIRE(CustomHouseConfig::isSafeAssetPath(""));
    REQUIRE_FALSE(CustomHouseConfig::isSafeAssetPath("../other-mod/herald.png"));
    REQUIRE_FALSE(CustomHouseConfig::isSafeAssetPath("presentation/../herald.png"));
    REQUIRE_FALSE(CustomHouseConfig::isSafeAssetPath("C:/local/voice.voc"));
    REQUIRE_FALSE(CustomHouseConfig::isSafeAssetPath("presentation\\voice.voc"));
}

TEST_CASE("Custom-house presentation defaults request safe fallbacks",
          "[custom-house][presentation][fallback]") {
    const CustomHouseInfo info;
    REQUIRE(info.heraldAsset.empty());
    REQUIRE(info.houseNameVoiceAsset.empty());
    REQUIRE(info.voicePlaybackRate == Catch::Approx(1.0));
    REQUIRE(info.voiceGain == Catch::Approx(1.0));
}

TEST_CASE("Tornie custom house prefers its ObjectData IX vehicles",
          "[custom-house][special-vehicle][tornie]") {
    std::vector<HouseSpecialVehicleCandidateData> objectData(ItemID_LastID + 1);
    const HouseSpecialVehicleCandidateData enabledIxVehicle{
        true,
        Structure_HeavyFactory,
        true
    };

    objectData[Unit_Deviator] = enabledIxVehicle;
    objectData[Unit_EliteLauncher] = enabledIxVehicle;
    objectData[Unit_Ornithopter] = enabledIxVehicle;
    objectData[Unit_Devastator] = { true, ItemID_Invalid, true };
    objectData[Unit_Trooper] = enabledIxVehicle;
    objectData[Unit_Harvester] = enabledIxVehicle;
    objectData[Unit_FlameTank] = { true, Structure_HeavyFactory, false };

    const auto tornieIxCandidates = discoverHouseSpecialVehicleCandidates(
        [&](int itemID) { return objectData[itemID]; });
    const std::vector<int> expectedCorruptiqueCandidates = {
        Unit_Devastator,
        Unit_EliteSiegeTank
    };

    REQUIRE(resolveSpecialVehiclePoolForHouse(
                HOUSE_CUSTOM, true, false, tornieIxCandidates)
            == expectedCorruptiqueCandidates);
    REQUIRE(resolveSpecialVehiclePoolForHouse(
                HOUSE_CUSTOM, true, false, tornieIxCandidates, true)
            == expectedCorruptiqueCandidates);
    REQUIRE_FALSE(isHouseSpecialVehicleCandidate(
        Unit_Ornithopter, objectData[Unit_Ornithopter]));
    REQUIRE_FALSE(isHouseSpecialVehicleCandidate(
        Unit_Devastator, objectData[Unit_Devastator]));
    REQUIRE_FALSE(isHouseSpecialVehicleCandidate(
        Unit_Trooper, objectData[Unit_Trooper]));
    REQUIRE_FALSE(isHouseSpecialVehicleCandidate(
        Unit_Harvester, objectData[Unit_Harvester]));
}

TEST_CASE("House fallback follows the active faction plan when ObjectData has no candidates",
          "[custom-house][special-vehicle][fallback]") {
    const std::vector<int> noModOwnedCandidates;
    const std::vector<int> genericFallback = {
        Unit_SonicTank,
        Unit_Devastator
    };
    const std::vector<int> expectedTornieFremen = {
        Unit_EliteSiegeTank,
        Unit_FlameTank
    };

    REQUIRE(resolveSpecialVehiclePoolForHouse(
                HOUSE_FREMEN, false, false, noModOwnedCandidates)
            == genericFallback);
    REQUIRE(resolveSpecialVehiclePoolForHouse(
                HOUSE_FREMEN, true, false, noModOwnedCandidates)
            == expectedTornieFremen);
}

TEST_CASE("Mentat feature patches reject malformed cells and invalid destinations", "[mod][mentat][config]") {
    std::vector<ModMentatPatch> patches;
    REQUIRE(ModMentatConfig::parsePatch("0,0,20,10;10,20,40,30;11,20,40,30", patches));
    REQUIRE(patches.size() == 1);
    REQUIRE(patches[0].sources.size() == 2);
    for(const auto& text : {"0,0,20,10;", "0,0,0,10;0,0,5,5", "0,0,20,10;1,2,3", "0,0,20,10;-1,2,3,4", "0,0,20,10;1,2,3,4garbage"}) {
        REQUIRE_FALSE(ModMentatConfig::parsePatch(text, patches));
        REQUIRE(patches.size() == 1);
    }
    ModMentatInfo info;
    info.backgroundAsset="cat.png"; info.restFromBackground=true;
    info.doubleEyes=false; info.doubleMouth=false;
    info.eyesX=134; info.eyesY=137; info.mouthX=174; info.mouthY=201;
    info.eyesCropHeight=112; info.eyesWidth=98; info.eyesHeight=32;
    info.mouthCropHeight=95; info.mouthWidth=40; info.mouthHeight=23;
    info.eyesFrames=2; info.eyesPatches=patches;
    REQUIRE(ModMentatConfig::isValid(info));
    info.eyesFrames=5;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
    info.eyesFrames=2; info.eyesPatches[0].destination.x=95;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
    info.eyesPatches.clear();
    REQUIRE(ModMentatConfig::parsePolygon("248,268,297,400,248,400",info.foregroundPolygon));
    REQUIRE(ModMentatConfig::isValid(info));
    REQUIRE_FALSE(ModMentatConfig::parsePolygon("1,2,3,4,5",info.foregroundPolygon));
    REQUIRE_FALSE(ModMentatConfig::parsePolygon("1,2,3,4,-5,6",info.foregroundPolygon));
    REQUIRE_FALSE(ModMentatConfig::parsePolygon("1,2,3,4,5,6,",info.foregroundPolygon));
    info.foregroundPolygon[0].x=5000;
    REQUIRE_FALSE(ModMentatConfig::isValid(info));
}
