//
// RGUI-style binding prompt used by controls/hotkeys
//

#ifndef FCEUX_VITA_RGUI_BINDING_PROMPT_H
#define FCEUX_VITA_RGUI_BINDING_PROMPT_H

#include "cross2d/c2d.h"
#include <string>

class RguiBindingPrompt : public c2d::RectangleShape {
public:
    static constexpr int TIMEOUT = -2;

    RguiBindingPrompt(c2d::Renderer *renderer, c2d::Font *font);
    ~RguiBindingPrompt() override = default;

    int capture(c2d::Input *input, const std::string &title, const std::string &message, int timeout_seconds = 9);

private:
    void createChildren();
    void updateLayout();

    c2d::Renderer *m_renderer;
    c2d::Font *m_font;
    c2d::RectangleShape *m_overlay = nullptr;
    c2d::RectangleShape *m_panel = nullptr;
    c2d::RectangleShape *m_title_bar = nullptr;
    c2d::Text *m_title_text = nullptr;
    c2d::Text *m_message_text = nullptr;
    c2d::Text *m_hint_text = nullptr;
    c2d::Text *m_timer_text = nullptr;
};

#endif // FCEUX_VITA_RGUI_BINDING_PROMPT_H
