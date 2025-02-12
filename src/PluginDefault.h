#ifndef PLUGIN_DEFAULT_H
#define PLUGIN_DEFAULT_H

#include "Plugin.h"
#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginDefault : public Plugin
{
public:
    PluginDefault(u32 gameCode) {GameCode = gameCode;};

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void onLoadState(melonDS::NDS* nds) {}

    u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) {
        return InputMask;
    }
    void applyTouchKeyMask(melonDS::NDS* nds, u32 TouchKeyMask) {}
    const char* getGameSceneName() {
        return "";
    }
    bool shouldRenderFrame(melonDS::NDS* nds) {
        return true;
    }
    bool ShouldTerminateIngameCutscene() {return false;}
    bool ShouldStartReplacementCutscene() {return false;}
    bool StartedReplacementCutscene() {return false;}
    bool ShouldStopReplacementCutscene() {return false;}
    bool ShouldReturnToGameAfterCutscene() {return false;}
    CutsceneEntry* CurrentCutscene() {return nullptr;}
    std::string CutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    void onIngameCutsceneIdentified(melonDS::NDS* nds, CutsceneEntry* cutscene) {}
    void onTerminateIngameCutscene(melonDS::NDS* nds) {}
    void onReturnToGameAfterCutscene(melonDS::NDS* nds) {}
    void onReplacementCutsceneStart(melonDS::NDS* nds) {}
    void onReplacementCutsceneEnd(melonDS::NDS* nds) {}
    bool refreshGameScene(melonDS::NDS* nds) {
        return false;
    }
    void setAspectRatio(melonDS::NDS* nds, float aspectRatio) {}
};
}

#endif
