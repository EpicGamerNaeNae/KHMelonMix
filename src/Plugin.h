#ifndef PLUGIN_H
#define PLUGIN_H

#define DEBUG_MODE_ENABLED false

#include "NDS.h"

#include "OpenGLSupport.h"

namespace Plugins
{
using namespace melonDS;

class Plugin
{
public:
    virtual ~Plugin() { };

    u32 GameCode = 0;
    static bool isCart(u32 gameCode) {return true;};

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual const char* gpu3DOpenGL_VS_Z() { return nullptr; };

    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) { };
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) { };
    virtual void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags) { };
    virtual void gpu3DOpenGL_VS_Z_updateVariables(u32 flags) { };

    virtual u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldSkipFrame(melonDS::NDS* nds) = 0;

    virtual bool refreshGameScene(melonDS::NDS* nds) = 0;

    virtual void setAspectRatio(melonDS::NDS* nds, float aspectRatio) = 0;
};
}

#endif