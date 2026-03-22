//
// RGUI hotkeys menu
//

#include "rgui_hotkeys.h"
#include "runtime/runtime.h"
#include <cstdio>

using namespace c2d;
using namespace pemu;

namespace {
    struct HotkeyItem {
        int opt_id;
        const char *label;
    };

    constexpr HotkeyItem HOTKEY_ITEMS[] = {
        {PEMUConfig::OptId::JOY_MENU1, "Menu 1"},
        {PEMUConfig::OptId::JOY_MENU2, "Menu 2"},
        {PEMUConfig::OptId::JOY_REWIND, "Rewind"},
    };
}

RguiHotkeys::RguiHotkeys(UiMain *ui, Renderer *renderer, Font *font)
    : m_ui(ui), m_renderer(renderer) {
    m_menu = new RguiMenu(renderer, font, "Hotkeys", {});
    m_prompt = new RguiBindingPrompt(renderer, font);
    m_menu->add(m_prompt);
}

RguiHotkeys::~RguiHotkeys() {
    delete m_menu;
}

void RguiHotkeys::refresh(bool in_game) {
    (void)in_game;
    m_in_game = false;

    std::vector<RguiMenuItem> items;
    items.reserve(sizeof(HOTKEY_ITEMS) / sizeof(HOTKEY_ITEMS[0]));
    for (const auto &item : HOTKEY_ITEMS) {
        items.push_back({item.label, getOptionValue(item.opt_id), item.opt_id, false});
    }

    m_menu->setItems(items);
    m_menu->setTitle("Hotkeys - Global");
}

void RguiHotkeys::captureBinding(int opt_id) {
    auto *opt = m_ui->getConfig()->get(opt_id, false);
    if (!opt) {
        return;
    }

    m_ui->getInput()->clear();

    std::string label = "Hotkey";
    for (const auto &item : HOTKEY_ITEMS) {
        if (item.opt_id == opt_id) {
            label = item.label;
            break;
        }
    }

    const int new_key = m_prompt->capture(
        m_ui->getInput(),
        "Bind " + label,
        "Choose a new input for " + label,
        9
    );
    if (new_key == RguiBindingPrompt::TIMEOUT) {
        return;
    }

    opt->setInteger(new_key);
    saveCurrentScope();
    m_ui->getInput()->clear();
}

void RguiHotkeys::saveCurrentScope() const {
    m_ui->getConfig()->save();
    m_ui->updateInputMapping(false);
}

std::string RguiHotkeys::getBindingLabel(int value) const {
    switch (value) {
        case SDL_CONTROLLER_BUTTON_INVALID:
            return "Unbound";
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return "Up";
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return "Down";
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return "Left";
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return "Right";
        case SDL_CONTROLLER_BUTTON_A:
            return "Cross";
        case SDL_CONTROLLER_BUTTON_B:
            return "Circle";
        case SDL_CONTROLLER_BUTTON_X:
            return "Square";
        case SDL_CONTROLLER_BUTTON_Y:
            return "Triangle";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return "L1";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "R1";
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return "L3";
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return "R3";
        case SDL_CONTROLLER_BUTTON_BACK:
            return "Select";
        case SDL_CONTROLLER_BUTTON_START:
            return "Start";
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return "PS";
        default:
            break;
    }

    if (value == SDL_CONTROLLER_AXIS_TRIGGERLEFT + 100) {
        return "L2";
    }
    if (value == SDL_CONTROLLER_AXIS_TRIGGERRIGHT + 100) {
        return "R2";
    }

    char label[32];
    snprintf(label, sizeof(label), "Button %d", value);
    return label;
}

std::string RguiHotkeys::getOptionValue(int opt_id) const {
    auto *opt = m_ui->getConfig()->get(opt_id, false);
    if (!opt) {
        return "N/A";
    }

    return getBindingLabel(opt->getInteger());
}

int RguiHotkeys::handleInput(Input *input) {
    const auto action = m_menu->handleInput(input);
    if (action == RguiMenu::CANCEL) {
        return 1;
    }

    if (action != RguiMenu::CONFIRM) {
        return -1;
    }

    const int sel = m_menu->getSelectedIndex();
    captureBinding(m_menu->getSelectedId());
    refresh(m_in_game);
    m_menu->setSelectedIndex(sel);
    return -1;
}

void RguiHotkeys::draw(Transform &t) {
    m_menu->onDraw(t, true);
}
