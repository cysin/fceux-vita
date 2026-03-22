//
// Created by cpasjuste on 08/12/18.
//

#ifndef C2DUI_STATUS_BOX_H
#define C2DUI_STATUS_BOX_H

#include <string>

#include "cross2d/c2d.h"
#include "skin/SkinnedRectangle.h"

namespace pemu {
    class UiMain;

    class UiStatusBox : public SkinnedRectangle {
    public:
        explicit UiStatusBox(UiMain *main);

        ~UiStatusBox() override;

        void show(const std::string &text);

        void show(const char *fmt, ...);

        void hide();

    private:
        void onDraw(c2d::Transform &transform, bool draw = true) override;
        void applyStyle();

        UiMain *main;
        c2d::Text *text;
        c2d::TweenAlpha *tween;
        c2d::Clock *clock;
    };
}

#endif //C2DUI_STATUS_BOX_H
