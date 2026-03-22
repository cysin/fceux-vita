//
// Created by cpasjuste on 14/01/18.
//

#ifndef GUI_H
#define GUI_H

#include "cross2d/c2d.h"
#include "skin.h"

#define INPUT_DELAY 200

namespace pemu {
    class PEMUConfig;
    class Skin;
    class UiEmu;
    class UiStatusBox;
    class UIProgressBox;

    class UiMain : public c2d::C2DRenderer {
    public:
        explicit UiMain(const c2d::Vector2f &size = c2d::Vector2f(0, 0));

        void init(UiEmu *emu);

        void setConfig(PEMUConfig *cfg);

        void onUpdate() override;

        void updateInputMapping(bool isRomCfg);

        Skin *getSkin() { return pSkin; }

        void setSkin(Skin *s) { pSkin = s; }

        PEMUConfig *getConfig() { return pConfig; }

        UiEmu *getUiEmu() { return pEmu; }

        UiStatusBox *getUiStatusBox() { return pStatusBox; }

        UIProgressBox *getUiProgressBox() { return pProgressBox; }

        c2d::MessageBox *getUiMessageBox() { return pMessageBox; }

        int getFontSize() { return (int)((float)C2D_DEFAULT_CHAR_SIZE * pSkin->getScaling().y); }

        c2d::Vector2f getScaling() { return pSkin->getScaling(); }

        bool done = false;

    private:
        PEMUConfig *pConfig = nullptr;
        Skin *pSkin = nullptr;
        UiEmu *pEmu = nullptr;
        UIProgressBox *pProgressBox = nullptr;
        c2d::MessageBox *pMessageBox = nullptr;
        UiStatusBox *pStatusBox = nullptr;
        c2d::C2DClock mTimer;
        unsigned int mOldKeys = 0;
    };
}

#endif //GUI_H
