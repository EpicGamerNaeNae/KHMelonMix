#include "KHDays_Plugin.h"

#include "GPU3D_OpenGL.h"

#include <math.h>

namespace melonDS
{

int priorGameScene = -1;
bool isBlackTopScreen = false;
bool isBlackBottomScreen = false;

// If you want to undertand that, check GPU2D_Soft.cpp, at the bottom of the SoftRenderer::DrawScanline function
#define PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(b) (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0xF)
#define PARSE_BRIGHTNESS_FOR_BLACK_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : 0)
#define PARSE_BRIGHTNESS_FOR_UNKNOWN_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0))

bool KHDaysPlugin::isBufferBlack(unsigned int* buffer)
{
    // when the result is 'null' (filled with zeros), it's a false positive, so we need to exclude that scenario
    bool newIsNullScreen = true;
    bool newIsBlackScreen = true;
    for (int i = 0; i < 192*256; i++) {
        unsigned int color = buffer[i] & 0xFFFFFF;
        newIsNullScreen = newIsNullScreen && color == 0;
        newIsBlackScreen = newIsBlackScreen &&
                (color == 0 || color == 0x000080 || color == 0x010000 || (buffer[i] & 0xFFFFE0) == 0x018000);
        if (!newIsBlackScreen) {
            break;
        }
    }
    return !newIsNullScreen && newIsBlackScreen;
}

void KHDaysPlugin::fetchScreenStatus(melonDS::NDS* nds, int frontbuf)
{
    // checking if bottom screen is totally black
    u32* topBuffer = nds->GPU.Framebuffer[frontbuf][0].get();
    u32* bottomBuffer = nds->GPU.Framebuffer[frontbuf][1].get();
    if (topBuffer) {
        isBlackTopScreen = isBufferBlack(topBuffer);
    }
    if (bottomBuffer) {
        isBlackBottomScreen = isBufferBlack(bottomBuffer);
    }
}

int KHDaysPlugin::getSizeByGameScene(int newGameScene)
{
    int size = 0;
    int screenSizing_TopOnly = 4;
    int screenSizing_BotOnly = 5;
    switch (newGameScene) {
        case gameScene_Intro: break;
        case gameScene_MainMenu: size = screenSizing_TopOnly; break;
        case gameScene_IntroLoadMenu: size = screenSizing_BotOnly; break;
        case gameScene_DayCounter: size = screenSizing_TopOnly; break;
        case gameScene_Cutscene: size = isBlackBottomScreen ? screenSizing_TopOnly : size; break;
        case gameScene_BottomCutscene: size = screenSizing_BotOnly; break;
        case gameScene_InGameWithMap: size = screenSizing_TopOnly; break;
        case gameScene_InGameWithoutMap: size = screenSizing_TopOnly; break;
        case gameScene_InGameMenu: break;
        case gameScene_InGameSaveMenu: size = screenSizing_TopOnly; break;
        case gameScene_InHoloMissionMenu: break;
        case gameScene_PauseMenu: size = screenSizing_TopOnly; break;
        case gameScene_PauseMenuWithGauge: size = screenSizing_TopOnly; break;
        case gameScene_Tutorial: size = screenSizing_BotOnly; break;
        case gameScene_RoxasThoughts: size = screenSizing_TopOnly; break;
        case gameScene_Shop: break;
        case gameScene_BlackScreen: size = screenSizing_TopOnly; break;
        default: break;
    }
    return size;
}

float* KHDaysPlugin::getBackgroundColorByGameScene(melonDS::NDS* nds, int newGameScene)
{
    float backgroundColor = 0.0;
    if (newGameScene == gameScene_Intro)
    {
        if (isBlackBottomScreen && isBlackTopScreen)
        {
            backgroundColor = 0;
        }
        else {
            backgroundColor = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_A.MasterBrightness) / 15.0;
            backgroundColor = (sqrt(backgroundColor)*3 + pow(backgroundColor, 2)) / 4;
        }
    }
    if (newGameScene == gameScene_MainMenu)
    {
        backgroundColor = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_B.MasterBrightness) / 15.0;
        backgroundColor = (sqrt(backgroundColor)*3 + pow(backgroundColor, 2)) / 4;
    }
    if (newGameScene == gameScene_InGameWithoutMap || newGameScene == gameScene_Other2D)
    {
        if (nds->GPU.GPU2D_B.MasterBrightness == 0) {
            backgroundColor = 0;
        }
        else if (nds->GPU.GPU2D_B.MasterBrightness & (1 << 14)) {
            backgroundColor = PARSE_BRIGHTNESS_FOR_BLACK_BACKGROUND(nds->GPU.GPU2D_B.MasterBrightness) / 15.0;
            backgroundColor = (sqrt(backgroundColor)*3 + pow(backgroundColor, 2)) / 4;
        }
    }
    float* bgColors = new float[3];
    bgColors[0] = backgroundColor;
    bgColors[1] = backgroundColor;
    bgColors[2] = backgroundColor;
    return bgColors;
}

int KHDaysPlugin::detectGameScene(melonDS::NDS* nds)
{
    // printf("0x021D08B8: %d\n",   nds->ARM7Read8(0x021D08B8));
    // printf("0x0223D38C: %d\n\n", nds->ARM7Read8(0x0223D38C));

    // Also happens during intro, during the start of the mission review, on some menu screens; those seem to use real 2D elements
    bool no3D = nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 0;

    // 3D element mimicking 2D behavior
    bool doesntLook3D = nds->GPU.GPU3D.RenderNumPolygons < 10;

    bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;

    // The second screen can still look black and not be empty (invisible elements)
    bool noElementsOnBottomScreen = nds->GPU.GPU2D_B.BlendCnt == 0;

    // Scale of brightness, from 0 (black) to 15 (every element is visible)
    u8 topScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_A.MasterBrightness);
    u8 botScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_B.MasterBrightness);

    // Shop has 2D and 3D segments, which is why it's on the top
    bool isShop = (nds->GPU.GPU3D.RenderNumPolygons == 264 && nds->GPU.GPU2D_A.BlendCnt == 0 && 
                   nds->GPU.GPU2D_B.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendAlpha == 16) ||
            (nds->GPU.GameScene == gameScene_Shop && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0);
    if (isShop)
    {
        return gameScene_Shop;
    }

    if (isBlackTopScreen && (nds->PowerControl9 >> 9) == 1)
    {
        return gameScene_BlackScreen;
    }

    if (doesntLook3D)
    {
        // Intro save menu
        bool isIntroLoadMenu = nds->GPU.GPU2D_B.BlendCnt == 4164 && (nds->GPU.GPU2D_A.EVA == 0 || nds->GPU.GPU2D_A.EVA == 16) && 
             nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 0 &&
            (nds->GPU.GPU2D_B.EVA < 10 && nds->GPU.GPU2D_B.EVA >= 0) && 
            (nds->GPU.GPU2D_B.EVB >  7 && nds->GPU.GPU2D_B.EVB <= 16) && nds->GPU.GPU2D_B.EVY == 0;
        bool mayBeMainMenu = nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;

        if (isIntroLoadMenu)
        {
            return gameScene_IntroLoadMenu;
        }
        if (nds->GPU.GameScene == gameScene_IntroLoadMenu)
        {
            if (mayBeMainMenu)
            {
                return gameScene_MainMenu;
            }
            if (nds->GPU.GPU3D.NumVertices != 8)
            {
                return gameScene_IntroLoadMenu;
            }
        }

        if ((nds->PowerControl9 >> 9) == 1 && nds->GPU.GameScene == gameScene_InGameMenu)
        {
            return gameScene_InGameMenu;
        }

        // Mission Mode / Story Mode - Challenges (happens if you press L/R repeatedly)
        bool inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 129 && nds->GPU.GPU2D_B.BlendCnt == 159;
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // Day counter
        if (nds->GPU.GameScene == gameScene_DayCounter && !no3D)
        {
            return gameScene_DayCounter;
        }
        if (nds->GPU.GameScene != gameScene_Intro)
        {
            if (nds->GPU.GPU3D.NumVertices == 8 && nds->GPU.GPU3D.NumPolygons == 2 && nds->GPU.GPU3D.RenderNumPolygons == 2)
            {
                return gameScene_DayCounter;
            }
            if (nds->GPU.GPU3D.NumVertices == 12 && nds->GPU.GPU3D.NumPolygons == 3 && nds->GPU.GPU3D.RenderNumPolygons == 3)
            {
                return gameScene_DayCounter;
            }
        }

        // Main menu
        if (mayBeMainMenu)
        {
            return gameScene_MainMenu;
        }

        // Intro
        if (nds->GPU.GameScene == -1 || nds->GPU.GameScene == gameScene_Intro)
        {
            return gameScene_Intro;
        }

        if (isBlackTopScreen && isBlackBottomScreen)
        {
            return gameScene_BlackScreen;
        }

        // Intro cutscene
        if (nds->GPU.GameScene == gameScene_Cutscene)
        {
            if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons >= 0 && nds->GPU.GPU3D.RenderNumPolygons <= 3)
            {
                return gameScene_Cutscene;
            }
        }
        if (nds->GPU.GameScene == gameScene_MainMenu && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 1)
        {
            return gameScene_Cutscene;
        }
        if (nds->GPU.GameScene == gameScene_BlackScreen && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons >= 0 && nds->GPU.GPU3D.RenderNumPolygons <= 3)
        {
            return gameScene_Cutscene;
        }

        // In Game Save Menu
        bool isGameSaveMenu = nds->GPU.GPU2D_A.BlendCnt == 4164 && (nds->GPU.GPU2D_B.EVA == 0 || nds->GPU.GPU2D_B.EVA == 16) && 
             nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0 &&
            (nds->GPU.GPU2D_A.EVA < 10 && nds->GPU.GPU2D_A.EVA >= 2) && 
            (nds->GPU.GPU2D_A.EVB >  7 && nds->GPU.GPU2D_A.EVB <= 14);
        if (isGameSaveMenu) 
        {
            return gameScene_InGameSaveMenu;
        }

        if ((nds->PowerControl9 >> 9) == 1) 
        {
            return gameScene_InGameMenu;
        }

        // Roxas thoughts scene
        if (isBlackBottomScreen)
        {
            if (has3DOnTopScreen)
            {
                return gameScene_BlackScreen;
            }
            else
            {
                return gameScene_RoxasThoughts;
            }
        }

        // Bottom cutscene
        bool isBottomCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
             nds->GPU.GPU2D_A.EVA == 16 && nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 9 &&
             nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (isBottomCutscene)
        {
            return gameScene_BottomCutscene;
        }

        if (nds->GPU.GameScene == gameScene_BlackScreen)
        {
            return gameScene_BlackScreen;
        }

        // Unknown 2D
        return gameScene_Other2D;
    }

    if (has3DOnTopScreen)
    {
        // Pause Menu
        bool inMissionPauseMenu = nds->GPU.GPU2D_A.EVY == 8 && nds->GPU.GPU2D_B.EVY == 8;
        if (inMissionPauseMenu)
        {
            if (nds->GPU.GameScene == gameScene_InGameWithMap)
            {
                return gameScene_PauseMenuWithGauge;  
            }
            if (nds->GPU.GameScene == gameScene_PauseMenu || nds->GPU.GameScene == gameScene_PauseMenuWithGauge)
            {
                return nds->GPU.GameScene;
            }
            return gameScene_PauseMenu;
        }
        else if (nds->GPU.GameScene == gameScene_PauseMenu || nds->GPU.GameScene == gameScene_PauseMenuWithGauge)
        {
            return priorGameScene;
        }

        // Tutorial
        if (nds->GPU.GameScene == gameScene_Tutorial && topScreenBrightness < 15)
        {
            return gameScene_Tutorial;
        }
        bool inTutorialScreen = topScreenBrightness == 8 && botScreenBrightness == 15;
        if (inTutorialScreen)
        {
            return gameScene_Tutorial;
        }
        bool inTutorialScreenWithoutWarningOnTop = nds->GPU.GPU2D_A.BlendCnt == 193 && nds->GPU.GPU2D_B.BlendCnt == 172 && 
                                                   nds->GPU.GPU2D_B.MasterBrightness == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (inTutorialScreenWithoutWarningOnTop)
        {
            return gameScene_Tutorial;
        }

        bool inGameMenu = (nds->GPU.GPU3D.NumVertices > 940 || nds->GPU.GPU3D.NumVertices == 0) &&
                          nds->GPU.GPU3D.RenderNumPolygons > 340 && nds->GPU.GPU3D.RenderNumPolygons < 360 &&
                          nds->GPU.GPU2D_A.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inGameMenu)
        {
            return gameScene_InGameMenu;
        }

        // Story Mode - Normal missions
        bool inHoloMissionMenu = ((nds->GPU.GPU3D.NumVertices == 344 && nds->GPU.GPU3D.NumPolygons == 89 && nds->GPU.GPU3D.RenderNumPolygons == 89) ||
                                  (nds->GPU.GPU3D.NumVertices == 348 && nds->GPU.GPU3D.NumPolygons == 90 && nds->GPU.GPU3D.RenderNumPolygons == 90)) &&
                                 nds->GPU.GPU2D_A.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inHoloMissionMenu || nds->GPU.GameScene == gameScene_InHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // Mission Mode / Story Mode - Challenges
        inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 129 && nds->GPU.GPU2D_B.BlendCnt == 159;
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // I can't remember
        inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 2625 && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // Regular gameplay without a map
        if (noElementsOnBottomScreen || isBlackBottomScreen)
        {
            return gameScene_InGameWithoutMap;
        }
    
        // Regular gameplay with a map
        return gameScene_InGameWithMap;
    }
    
    // Unknown
    return gameScene_Other;
}

bool KHDaysPlugin::setGameScene(melonDS::NDS* nds, int newGameScene)
{
    bool updated = false;
    if (nds->GPU.GameScene != newGameScene) 
    {
        updated = true;

        // Game scene
        priorGameScene = nds->GPU.GameScene;
        nds->GPU.GameScene = newGameScene;
    }

    // Updating GameScene inside shader
    static_cast<GLRenderer&>(nds->GPU.GetRenderer3D()).GetCompositor().SetGameScene(newGameScene);
    return updated;
}

}