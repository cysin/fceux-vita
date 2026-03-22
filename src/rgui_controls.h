//
// RGUI controls menu
//

#ifndef FCEUX_VITA_RGUI_CONTROLS_H
#define FCEUX_VITA_RGUI_CONTROLS_H

#include "rgui_binding_prompt.h"
#include "rgui_menu.h"
#include "cross2d/c2d.h"

namespace pemu {
    class UiMain;
}

class RguiControls {
public:
    RguiControls(pemu::UiMain *ui, c2d::Renderer *renderer, c2d::Font *font);
    ~RguiControls();

    void refresh(bool in_game);

    // returns: 1=cancelled, -1=navigating
    int handleInput(c2d::Input *input);

    void draw(c2d::Transform &t);

private:
    void cycleDeadZone(int direction);
    void captureBinding(int opt_id);
    void saveCurrentScope() const;
    std::string getBindingLabel(int value) const;
    std::string getOptionValue(int opt_id) const;

    pemu::UiMain *m_ui;
    c2d::Renderer *m_renderer;
    RguiMenu *m_menu;
    RguiBindingPrompt *m_prompt;
    bool m_in_game = false;
};

#endif // FCEUX_VITA_RGUI_CONTROLS_H
