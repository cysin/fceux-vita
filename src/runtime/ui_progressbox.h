//
// Created by cpasjuste on 04/02/18.
//

#ifndef C2D_UI_PROGRESSBOX_H
#define C2D_UI_PROGRESSBOX_H

#include <string>

#include "cross2d/c2d.h"
#include "skin/SkinnedRectangle.h"

namespace pemu {
    class UiMain;

    class UIProgressBox : public SkinnedRectangle {
    public:
        explicit UIProgressBox(UiMain *ui);

        void setTitle(const std::string &title);

        void setMessage(const std::string &message);

        void setProgress(float progress);

        c2d::Text *getTitleText();

        c2d::Text *getMessageText();

    private:
        c2d::Text *m_title;
        c2d::Text *m_message;
        c2d::RectangleShape *m_progress_bg;
        c2d::RectangleShape *m_progress_fg;
    };
}

#endif //C2D_UI_PROGRESSBOX_H
