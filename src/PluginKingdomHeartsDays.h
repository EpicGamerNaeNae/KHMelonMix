#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsDays : public Plugin
{
public:
    PluginKingdomHeartsDays(u32 gameCode);

    u32 GameCode;
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };
    bool isJapanCartRev1() { return false; }; // TODO: KH Add support to Rev1

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGL_VS_Z();

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress);
    const char* getGameSceneName();
    bool shouldSkipFrame(melonDS::NDS* nds);
    void setAspectRatio(melonDS::NDS* nds, float aspectRatio);
    bool refreshGameScene(melonDS::NDS* nds);
    void debugLogs(melonDS::NDS* nds, int gameScene);
private:
    bool PausedInGame;
    int HUDState;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    int priorGameScene;
    int GameScene;
    int UIScale = 4;
    float AspectRatio;
    bool ShowMap;
    bool ShowTarget;
    bool ShowMissionGauge;
    bool ShowMissionInfo;

    std::map<GLuint, GLuint[10]> CompGpuLoc{};
    std::map<u32, GLuint[3]> CompGpu3DLoc{};

    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    bool _hasVisible3DOnBottomScreen;

    u32 PriorHotkeyMask, PriorPriorHotkeyMask;
    u32 LastLockOnPress, LastSwitchTargetPress;
    bool SwitchTargetPressOnHold;

    int detectGameScene(melonDS::NDS* nds);
    bool setGameScene(melonDS::NDS* nds, int newGameScene);

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack(melonDS::NDS* nds);
    bool isBottomScreen2DTextureBlack(melonDS::NDS* nds);
    void hudToggle(melonDS::NDS* nds);
};
}

#endif