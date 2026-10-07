#ifndef COOP_CAMPAIGN_MENU_H
#define COOP_CAMPAIGN_MENU_H

#include <Menu/MenuBase.h>
#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/TextButton.h>
#include <GUI/TextView.h>
#include <GUI/Label.h>
#include <GUI/Checkbox.h>

// The host creates or resumes the common campaign; faction choice happens in
// the network lobby. Progress is separate from the single-player campaigns.
class CoopCampaignMenu : public MenuBase {
public:
    explicit CoopCampaignMenu(bool LANServer);
    void onChildWindowClose(Window* child) override;
private:
    void onNewCampaign();
    void onResumeCampaign();
    void onBack();
    bool LANServer;
    StaticContainer windowWidget;
    VBox mainVBox;
    Label caption;
    TextView description;
    Checkbox chaosMode;
    TextButton newCampaignButton;
    TextButton resumeButton;
    TextButton backButton;
};

#endif
