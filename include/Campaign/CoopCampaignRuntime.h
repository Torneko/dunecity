#ifndef COOP_CAMPAIGN_RUNTIME_H
#define COOP_CAMPAIGN_RUNTIME_H

#include <Campaign/CoopCampaignSession.h>
#include <GameInitSettings.h>

namespace coop {

std::string readMissionTemplate(const CoopCampaignSession& session);
GameInitSettings makeGameSettings(const CoopCampaignSession& session,
    const SettingsClass::GameOptionsClass& options, const std::string& serverName);
std::string progressPath(const Context& context);
std::string checkpointPath(const Context& context);

// The lobby stays alive throughout this call and owns the network connection.
// Ordinary multiplayer still runs one game; [COOP] maps retain the connection
// through shared results, readiness and all subsequent campaign missions.
void runMultiplayerSession(const GameInitSettings& initialSettings);

} // namespace coop
#endif
