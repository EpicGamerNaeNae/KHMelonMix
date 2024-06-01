#ifndef KHRECODED_PLUGIN_H
#define KHRECODED_PLUGIN_H

#include "NDS.h"

namespace melonDS
{
class KHReCodedPlugin
{
public:
    static u32 applyCommandMenuInputMask(melonDS::NDS* nds, u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    static void hudToggle(melonDS::NDS* nds);
    static const char* getNameByGameScene(int newGameScene);
    static bool shouldSkipFrame(melonDS::NDS* nds);
    static int detectGameScene(melonDS::NDS* nds);
    static bool setGameScene(melonDS::NDS* nds, int newGameScene);
    static void debugLogs(melonDS::NDS* nds, int gameScene);
private:
    static int GameScene;
    static int priorGameScene;
    static bool ShowMap;

    static bool _olderHad3DOnTopScreen;
    static bool _olderHad3DOnBottomScreen;
    static bool _had3DOnTopScreen;
    static bool _had3DOnBottomScreen;

    static void hudRefresh(melonDS::NDS* nds);
};
}

#endif
