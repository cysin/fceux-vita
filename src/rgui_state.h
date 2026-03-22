//
// RGUI save/load state menu
//

#ifndef FCEUX_VITA_RGUI_STATE_H
#define FCEUX_VITA_RGUI_STATE_H

#include "rgui_menu.h"
#include "cross2d/c2d.h"
#include <string>

namespace pemu {
    class UiMain;
}

class RguiStateMenu {
public:
    enum Mode { SAVE, LOAD };

    RguiStateMenu(c2d::Renderer *renderer, c2d::Font *font, pemu::UiMain *ui);
    ~RguiStateMenu();

    void setMode(Mode mode);
    void refresh();

    // returns: 0=action done, 1=cancelled, -1=navigating
    int handleInput(c2d::Input *input);

    void draw(c2d::Transform &t);

private:
    static constexpr int SLOT_COUNT = 4;

    c2d::Renderer *m_renderer;
    pemu::UiMain *m_ui;
    RguiMenu *m_menu;
    Mode m_mode = SAVE;
    std::string m_paths[SLOT_COUNT];
    std::string m_shots[SLOT_COUNT];
};

#endif // FCEUX_VITA_RGUI_STATE_H
