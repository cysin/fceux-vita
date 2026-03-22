//
// Created by cpasjuste on 03/02/18.
//

#ifndef C2DUI_UI_EMU_H
#define C2DUI_UI_EMU_H

#include "cross2d/c2d.h"
#include "game_types.h"

namespace pemu {
    class UiMain;
    class C2DUIVideo;

    class UiEmu : public c2d::RectangleShape {
    public:
        explicit UiEmu(UiMain *ui);

        virtual int load(const Game &game);

        virtual void stop();

        virtual void pause();

        virtual void resume();

        UiMain *getUi();

        C2DUIVideo *getVideo();

        c2d::Audio *getAudio();

        void addAudio(c2d::Audio *audio);

        void addAudio(int rate = 48000, int samples = 2048, c2d::Audio::C2DAudioCallback cb = nullptr);

        void addVideo(C2DUIVideo *video);

        void addVideo(uint8_t **pixels, int *pitch, const c2d::Vector2i &size,
                      const c2d::Vector2i &aspect = {4, 3},
                      c2d::Texture::Format format = c2d::Texture::Format::RGB565);

        c2d::Text *getFpsText();

        bool isPaused();

        bool onInput(c2d::Input::Player *players) override;

        Game getCurrentGame() const {
            return currentGame;
        }

        void setExitOnStop(bool value) { mExitOnStop = value; }

        bool getExitOnStop() { return mExitOnStop; }

    protected:
        void onUpdate() override;

        Game currentGame;
        c2d::Text *fpsText = nullptr;
        UiMain *pMain = nullptr;
        C2DUIVideo *video = nullptr;
        c2d::Audio *audio = nullptr;
        char fpsString[32];
        float targetFps = 60;
        bool paused = true;
        bool mExitOnStop = false;
    };
}

#endif //C2DUI_UI_EMU_H
