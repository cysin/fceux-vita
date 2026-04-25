//
// RGUI main menu controller - FCEUX version
//

#include "rgui_main.h"
#include "runtime/runtime.h"
#include "fceux_vita_config.h"
#include "rgui_cheats.h"
#include "rgui_controls.h"
#include "rgui_hotkeys.h"
#include <cstdio>

using namespace c2d;
using namespace pemu;

extern char szAppRomPath[];
extern char szAppConfigPath[];

namespace {
    const char *RGUI_LAST_ROM_PATH_FILE = "last_rom_path.cfg";
    const char *RGUI_FONT_FILE = "rgui.ttf";

    std::string getDefaultBrowsePath(c2d::Io *io) {
        const std::string vitaRoot = "ux0:/";
        if (io && io->getType(vitaRoot) == c2d::Io::Type::Directory) {
            return vitaRoot;
        }
        return szAppRomPath;
    }

    std::string normalizeBrowsePath(c2d::Io *io, const std::string &path) {
        std::string normalized = Utility::trim(path);
        if (normalized.empty()) {
            return {};
        }
        if (normalized.back() != '/') {
            normalized += "/";
        }
        if (io && io->getType(normalized) != c2d::Io::Type::Directory) {
            return {};
        }
        return normalized;
    }

    std::string loadBrowsePath(c2d::Io *io, std::string &outLastRom) {
        outLastRom.clear();
        char path[1024];
        snprintf(path, sizeof(path), "%s%s", szAppConfigPath, RGUI_LAST_ROM_PATH_FILE);

        FILE *f = fopen(path, "r");
        if (!f) {
            return getDefaultBrowsePath(io);
        }

        char line[1024];
        std::string browsePath;
        if (fgets(line, sizeof(line), f)) {
            browsePath = normalizeBrowsePath(io, line);
        }
        if (fgets(line, sizeof(line), f)) {
            std::string rom = line;
            while (!rom.empty() && (rom.back() == '\n' || rom.back() == '\r')) {
                rom.pop_back();
            }
            outLastRom = rom;
        }
        fclose(f);

        if (browsePath.empty()) {
            return getDefaultBrowsePath(io);
        }
        return browsePath;
    }

    void saveBrowsePath(const std::string &browsePath, const std::string &romName = {}) {
        if (browsePath.empty()) {
            return;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s%s", szAppConfigPath, RGUI_LAST_ROM_PATH_FILE);

        FILE *f = fopen(path, "w");
        if (!f) {
            return;
        }
        fprintf(f, "%s\n", browsePath.c_str());
        if (!romName.empty()) {
            fprintf(f, "%s\n", romName.c_str());
        }
        fclose(f);
    }

    c2d::Font *loadRguiFont(pemu::UiMain *ui) {
        auto skin = ui->getSkin();
        auto skinFont = skin->getFont();
        for (const auto &skinPath: skin->getPaths()) {
            std::string fontPath = skinPath + RGUI_FONT_FILE;
            if (!ui->getIo()->exist(fontPath)) {
                continue;
            }

            auto *font = new C2DFont();
            if (!font->loadFromFile(fontPath)) {
                delete font;
                continue;
            }

            font->setFilter(skinFont->getFilter());
            font->setOffset(skinFont->getOffset());
            return font;
        }

        return nullptr;
    }
}

RguiMain::RguiMain(UiMain *ui)
    : RectangleShape(ui->getSize()), m_ui(ui), m_renderer(ui) {
    RectangleShape::setFillColor(Color::Transparent);

    m_last_browse_path = loadBrowsePath(m_renderer->getIo(), m_last_rom_name);
    m_rgui_font = loadRguiFont(m_ui);
    std::string root_browse_path = getDefaultBrowsePath(m_renderer->getIo());

    // create sub-menus
    Font *font = m_rgui_font ? m_rgui_font : m_ui->getSkin()->getFont();
    m_main_menu = new RguiMenu(m_renderer, font, "FCEUX Vita", {});
    m_settings_menu = new RguiMenu(m_renderer, font, "Settings", {});
    m_filebrowser = new RguiFileBrowser(m_renderer, font, root_browse_path,
                                        m_last_browse_path, m_ui->getConfig()->getCoreSupportedExt());
    m_state_menu = new RguiStateMenu(m_renderer, font, m_ui);
    m_cheats_menu = new RguiCheats(m_renderer, font);
    m_controls_menu = new RguiControls(m_ui, m_renderer, font);
    m_hotkeys_menu = new RguiHotkeys(m_ui, m_renderer, font);

    RectangleShape::setVisibility(Visibility::Hidden);
}

RguiMain::~RguiMain() {
    delete m_main_menu;
    delete m_settings_menu;
    delete m_filebrowser;
    delete m_state_menu;
    delete m_cheats_menu;
    delete m_controls_menu;
    delete m_hotkeys_menu;
    delete m_rgui_font;
}

std::string RguiMain::getCurrentRomName() const {
    if (!m_ui->getUiEmu()) return {};
    auto game = m_ui->getUiEmu()->getCurrentGame();
    return Utility::removeExt(game.path);
}

void RguiMain::buildMainMenu() {
    std::vector<RguiMenuItem> items;

    if (m_in_game) {
        items.push_back({"Resume Game", "", ID_RESUME, false});
    }

    items.push_back({"Load ROM", "", ID_LOAD_ROM, true});

    if (m_in_game) {
        items.push_back({"Save State", "", ID_SAVE_STATE, true});
        items.push_back({"Load State", "", ID_LOAD_STATE, true});
        items.push_back({"Controls", "", ID_CONTROLS, true});
        items.push_back({"Cheats", "", ID_CHEATS, true});
    }

    items.push_back({"Settings", "", ID_SETTINGS, true});
    items.push_back({"Quit", "", ID_QUIT, false});

    m_main_menu->setItems(items);
    m_main_menu->setTitle(m_in_game ? "FCEUX Vita - In Game" : "FCEUX Vita");
}

void RguiMain::buildSettingsMenu() {
    std::vector<RguiMenuItem> items;

    if (!m_in_game) {
        items.push_back({"Controls", "", ID_SETTINGS_CONTROLS, true});
        items.push_back({"Hotkeys", "", ID_SETTINGS_HOTKEYS, true});
    }
    items.push_back({"Show FPS", getOptionValue(PEMUConfig::OptId::EMU_SHOW_FPS),
                      PEMUConfig::OptId::EMU_SHOW_FPS, false});
    items.push_back({"Scaling", getOptionValue(PEMUConfig::OptId::EMU_SCALING),
                      PEMUConfig::OptId::EMU_SCALING, false});
    items.push_back({"Scaling Mode", getOptionValue(PEMUConfig::OptId::EMU_SCALING_MODE),
                      PEMUConfig::OptId::EMU_SCALING_MODE, false});
    items.push_back({"Filtering", getOptionValue(PEMUConfig::OptId::EMU_FILTER),
                      PEMUConfig::OptId::EMU_FILTER, false});
    items.push_back({"Frameskip", getOptionValue(PEMUConfig::OptId::EMU_FRAMESKIP),
                      PEMUConfig::OptId::EMU_FRAMESKIP, false});
    items.push_back({"Rewind", getOptionValue(PEMUConfig::OptId::EMU_REWIND),
                      PEMUConfig::OptId::EMU_REWIND, false});
#ifdef __VITA__
    items.push_back({"Wait Rendering", getOptionValue(PEMUConfig::OptId::EMU_WAIT_RENDERING),
                      PEMUConfig::OptId::EMU_WAIT_RENDERING, false});
#endif
    items.push_back({"Audio Frequency", getOptionValue(PEMUConfig::OptId::EMU_AUDIO_FREQ),
                      PEMUConfig::OptId::EMU_AUDIO_FREQ, false});

    m_settings_menu->setItems(items);
}

std::string RguiMain::getOptionValue(int optId) {
    auto *opt = m_ui->getConfig()->get(optId, m_in_game);
    if (!opt) return "N/A";
    return opt->getString();
}

void RguiMain::cycleOption(int optId, int direction) {
    auto *opt = m_ui->getConfig()->get(optId, m_in_game);
    if (!opt) return;

    int idx = opt->getArrayIndex();
    int count = (int)opt->getArray().size();
    if (count <= 0) return;

    idx += direction;
    if (idx >= count) idx = 0;
    if (idx < 0) idx = count - 1;

    opt->setArrayIndex(idx);

    // apply immediate changes
    if (optId == PEMUConfig::OptId::EMU_FILTER) {
        auto *emu = m_ui->getUiEmu();
        if (emu && emu->getVideo()) {
            emu->getVideo()->setFilter(
                (Texture::Filter)opt->getArrayIndex());
        }
    } else if (optId == PEMUConfig::OptId::EMU_SCALING ||
               optId == PEMUConfig::OptId::EMU_SCALING_MODE) {
        auto *emu = m_ui->getUiEmu();
        if (emu && emu->getVideo()) {
            emu->getVideo()->updateScaling();
        }
    }
#ifdef __VITA__
    else if (optId == PEMUConfig::OptId::EMU_WAIT_RENDERING) {
        ((PSP2Renderer *)m_ui)->setWaitRendering(opt->getInteger());
    }
#endif

    // save
    if (m_in_game) {
        m_ui->getConfig()->saveGame();
    } else {
        m_ui->getConfig()->save();
    }
}

void RguiMain::show(bool in_game) {
    m_in_game = in_game;
    m_screen = SCREEN_MAIN;
    m_visible = true;
    buildMainMenu();
    RectangleShape::setVisibility(Visibility::Visible);
    m_ui->getInput()->setRepeatDelay(INPUT_DELAY);
    m_ui->getInput()->clear();
}

void RguiMain::hide() {
    m_visible = false;
    RectangleShape::setVisibility(Visibility::Hidden);
}

bool RguiMain::isVisible() const {
    return m_visible;
}

void RguiMain::resumeGame() {
    hide();
    // Consume the button used to exit the menu before gameplay mapping is restored.
    m_ui->getInput()->clear();
    m_ui->getUiEmu()->resume();
}

void RguiMain::handleMainAction(RguiMenu::Action action) {
    if (action == RguiMenu::CONFIRM) {
        int id = m_main_menu->getSelectedId();
        switch (id) {
            case ID_RESUME:
                resumeGame();
                break;
            case ID_LOAD_ROM:
                m_screen = SCREEN_FILEBROWSER;
                m_filebrowser->setPath(m_last_browse_path, m_last_rom_name);
                m_ui->getInput()->clear();
                break;
            case ID_SAVE_STATE:
                m_screen = SCREEN_SAVESTATE;
                m_state_menu->setMode(RguiStateMenu::SAVE);
                m_ui->getInput()->clear();
                break;
            case ID_LOAD_STATE:
                m_screen = SCREEN_LOADSTATE;
                m_state_menu->setMode(RguiStateMenu::LOAD);
                m_ui->getInput()->clear();
                break;
            case ID_CONTROLS:
                m_screen = SCREEN_CONTROLS;
                m_controls_menu->refresh(true);
                m_ui->getInput()->clear();
                break;
            case ID_CHEATS: {
                m_screen = SCREEN_CHEATS;
                std::string romName = getCurrentRomName();
                RguiCheats::loadState(romName.c_str());
                m_cheats_menu->refresh();
                m_ui->getInput()->clear();
                break;
            }
            case ID_SETTINGS:
                m_screen = SCREEN_SETTINGS;
                buildSettingsMenu();
                m_ui->getInput()->clear();
                break;
            case ID_QUIT:
                if (m_in_game) {
                    m_ui->getUiEmu()->stop();
                    m_in_game = false;
                    m_screen = SCREEN_MAIN;
                    buildMainMenu();
                    m_ui->getInput()->clear();
                } else {
                    hide();
                    m_ui->done = true;
                }
                break;
        }
    } else if (action == RguiMenu::CANCEL) {
        if (m_in_game) {
            resumeGame();
        }
    }
}

void RguiMain::handleSettingsAction(RguiMenu::Action action) {
    if (action == RguiMenu::CANCEL) {
        m_screen = SCREEN_MAIN;
        buildMainMenu();
        return;
    }

    if (action == RguiMenu::LEFT || action == RguiMenu::RIGHT || action == RguiMenu::CONFIRM) {
        int optId = m_settings_menu->getSelectedId();
        if (optId == ID_SETTINGS_CONTROLS) {
            if (action == RguiMenu::CONFIRM) {
                m_screen = SCREEN_CONTROLS;
                m_controls_menu->refresh(m_in_game);
                m_ui->getInput()->clear();
            }
            return;
        }
        if (optId == ID_SETTINGS_HOTKEYS) {
            if (action == RguiMenu::CONFIRM) {
                m_screen = SCREEN_HOTKEYS;
                m_hotkeys_menu->refresh(m_in_game);
                m_ui->getInput()->clear();
            }
            return;
        }
        int dir = (action == RguiMenu::LEFT) ? -1 : 1;
        int sel = m_settings_menu->getSelectedIndex();
        cycleOption(optId, dir);
        buildSettingsMenu();
        m_settings_menu->setSelectedIndex(sel);
    }
}

bool RguiMain::onInput(Input::Player *players) {
    if (!m_visible) return false;

    Input *input = m_ui->getInput();

    switch (m_screen) {
        case SCREEN_MAIN: {
            auto action = m_main_menu->handleInput(input);
            if (action != RguiMenu::NONE) {
                handleMainAction(action);
            }
            break;
        }
        case SCREEN_SETTINGS: {
            auto action = m_settings_menu->handleInput(input);
            if (action != RguiMenu::NONE) {
                handleSettingsAction(action);
            }
            break;
        }
        case SCREEN_FILEBROWSER: {
            int result = m_filebrowser->handleInput(input);
            if (result == 0) {
                // file selected - remember directory and ROM for next time
                m_last_browse_path = m_filebrowser->getCurrentPath();
                std::string path = m_filebrowser->getSelectedPath();
                m_last_rom_name = Utility::baseName(path);
                saveBrowsePath(m_last_browse_path, m_last_rom_name);
                if (!path.empty()) {
                    Game game;
                    game.path = Utility::baseName(path);
                    game.name = Utility::removeExt(game.path);
                    game.romsPath = Utility::remove(path, game.path);

                    // stop current game if running
                    if (m_in_game) {
                        m_ui->getConfig()->saveGame();
                        m_ui->getUiEmu()->stop();
                        m_in_game = false;
                    }

                    m_ui->getConfig()->loadGame(game);
                    if (m_ui->getUiEmu()->load(game) == 0) {
                        hide();
                        m_ui->getInput()->clear();
                    } else {
                        m_ui->getUiProgressBox()->setVisibility(Visibility::Hidden);
                        m_ui->getConfig()->clearGame();
                        m_ui->updateInputMapping(false);
                        m_ui->getInput()->setRepeatDelay(INPUT_DELAY);
                        fceux_vita::load_error::show(m_ui, &game);
                        m_screen = SCREEN_FILEBROWSER;
                        RectangleShape::setVisibility(Visibility::Visible);
                        m_visible = true;
                        m_ui->getInput()->clear();
                    }
                }
            } else if (result == 1) {
                // remember directory even on cancel
                m_last_browse_path = m_filebrowser->getCurrentPath();
                saveBrowsePath(m_last_browse_path);
                m_screen = SCREEN_MAIN;
                buildMainMenu();
            }
            break;
        }
        case SCREEN_SAVESTATE:
        case SCREEN_LOADSTATE: {
            int result = m_state_menu->handleInput(input);
            if (result == 0) {
                // action done
                resumeGame();
            } else if (result == 1) {
                m_screen = SCREEN_MAIN;
                buildMainMenu();
            }
            break;
        }
        case SCREEN_CHEATS: {
            int result = m_cheats_menu->handleInput(input);
            if (result == 1) {
                if (m_in_game) {
                    std::string romName = getCurrentRomName();
                    RguiCheats::saveState(romName.c_str());
                }
                m_screen = SCREEN_MAIN;
                buildMainMenu();
            }
            break;
        }
        case SCREEN_CONTROLS: {
            int result = m_controls_menu->handleInput(input);
            if (result == 1) {
                if (m_in_game) {
                    m_screen = SCREEN_MAIN;
                    buildMainMenu();
                } else {
                    m_screen = SCREEN_SETTINGS;
                    buildSettingsMenu();
                }
            }
            break;
        }
        case SCREEN_HOTKEYS: {
            int result = m_hotkeys_menu->handleInput(input);
            if (result == 1) {
                m_screen = SCREEN_SETTINGS;
                buildSettingsMenu();
            }
            break;
        }
    }

    return true;
}

void RguiMain::onDraw(Transform &t, bool draw) {
    if (!m_visible || !draw) return;

    switch (m_screen) {
        case SCREEN_MAIN:
            m_main_menu->onDraw(t, true);
            break;
        case SCREEN_SETTINGS:
            m_settings_menu->onDraw(t, true);
            break;
        case SCREEN_FILEBROWSER:
            m_filebrowser->draw(t);
            break;
        case SCREEN_SAVESTATE:
        case SCREEN_LOADSTATE:
            m_state_menu->draw(t);
            break;
        case SCREEN_CHEATS:
            m_cheats_menu->draw(t);
            break;
        case SCREEN_CONTROLS:
            m_controls_menu->draw(t);
            break;
        case SCREEN_HOTKEYS:
            m_hotkeys_menu->draw(t);
            break;
    }

    RectangleShape::onDraw(t, draw);
}
