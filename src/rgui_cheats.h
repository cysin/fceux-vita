//
// RGUI cheats menu - FCEUX version (Game Genie codes)
//

#ifndef FCEUX_VITA_RGUI_CHEATS_H
#define FCEUX_VITA_RGUI_CHEATS_H

#include "rgui_menu.h"
#include "cross2d/c2d.h"
#include <string>
#include <vector>

struct FceuxCheat {
    std::string name;
    std::string gg_code;  // Game Genie code (6 or 8 chars)
    bool enabled;
};

class RguiCheats {
public:
    RguiCheats(c2d::Renderer *renderer, c2d::Font *font);
    ~RguiCheats();

    void refresh();

    // returns: 0=done, 1=cancelled, -1=navigating
    int handleInput(c2d::Input *input);

    void draw(c2d::Transform &t);

    static void saveState(const char *romName);
    static void loadState(const char *romName);

private:
    c2d::Renderer *m_renderer;
    RguiMenu *m_menu;
};

extern std::vector<FceuxCheat> g_fceux_cheats;

#endif // FCEUX_VITA_RGUI_CHEATS_H
