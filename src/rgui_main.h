//
// RGUI main menu controller
//

#ifndef FCEUX_VITA_RGUI_MAIN_H
#define FCEUX_VITA_RGUI_MAIN_H

#include "rgui_menu.h"
#include "rgui_filebrowser.h"
#include "rgui_state.h"
#include "rgui_cheats.h"
#include "rgui_controls.h"
#include "rgui_hotkeys.h"
#include "runtime/pemu_config.h"
#include "cross2d/c2d.h"

namespace pemu {
    class UiMain;
    class UiEmu;
}

class RguiMain : public c2d::RectangleShape {
public:
    explicit RguiMain(pemu::UiMain *ui);
    ~RguiMain() override;

    void show(bool in_game);
    void hide();
    bool isVisible() const;

    bool onInput(c2d::Input::Player *players) override;
    void onDraw(c2d::Transform &t, bool draw) override;

private:
    enum Screen {
        SCREEN_MAIN,
        SCREEN_SETTINGS,
        SCREEN_FILEBROWSER,
        SCREEN_SAVESTATE,
        SCREEN_LOADSTATE,
        SCREEN_CHEATS,
        SCREEN_CONTROLS,
        SCREEN_HOTKEYS
    };

    enum MainMenuId {
        ID_RESUME = 0,
        ID_LOAD_ROM,
        ID_SAVE_STATE,
        ID_LOAD_STATE,
        ID_CONTROLS,
        ID_CHEATS,
        ID_SETTINGS,
        ID_QUIT
    };

    enum SettingsMenuId {
        ID_SETTINGS_CONTROLS = -1001,
        ID_SETTINGS_HOTKEYS = -1002
    };

    void buildMainMenu();
    void buildSettingsMenu();
    void handleMainAction(RguiMenu::Action action);
    void handleSettingsAction(RguiMenu::Action action);
    void resumeGame();
    void cycleOption(int optId, int direction);
    std::string getOptionValue(int optId);

    std::string getCurrentRomName() const;

    pemu::UiMain *m_ui;
    c2d::Renderer *m_renderer;
    bool m_visible = false;
    bool m_in_game = false;
    Screen m_screen = SCREEN_MAIN;

    RguiMenu *m_main_menu = nullptr;
    RguiMenu *m_settings_menu = nullptr;
    RguiFileBrowser *m_filebrowser = nullptr;
    RguiStateMenu *m_state_menu = nullptr;
    RguiCheats *m_cheats_menu = nullptr;
    RguiControls *m_controls_menu = nullptr;
    RguiHotkeys *m_hotkeys_menu = nullptr;
    c2d::Font *m_rgui_font = nullptr;
    std::string m_last_browse_path;
    std::string m_last_rom_name;
};

#endif // FCEUX_VITA_RGUI_MAIN_H
