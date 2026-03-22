//
// RGUI controls menu
//

#include "rgui_controls.h"
#include "runtime/runtime.h"
#include <cstdio>

using namespace c2d;
using namespace pemu;

namespace {
    struct ControlItem {
        int opt_id;
        const char *label;
    };

    constexpr ControlItem CONTROL_ITEMS[] = {
        {PEMUConfig::OptId::JOY_UP, "Up"},
        {PEMUConfig::OptId::JOY_DOWN, "Down"},
        {PEMUConfig::OptId::JOY_LEFT, "Left"},
        {PEMUConfig::OptId::JOY_RIGHT, "Right"},
        {PEMUConfig::OptId::JOY_A, "B"},
        {PEMUConfig::OptId::JOY_B, "A"},
        {PEMUConfig::OptId::JOY_X, "Turbo B"},
        {PEMUConfig::OptId::JOY_Y, "Turbo A"},
        {PEMUConfig::OptId::JOY_SELECT, "Select"},
        {PEMUConfig::OptId::JOY_START, "Start"},
        {PEMUConfig::OptId::JOY_DEADZONE, "Dead Zone"},
    };

    bool isTriggerAxis(int value) {
        return value == (SDL_CONTROLLER_AXIS_TRIGGERLEFT + 100) ||
               value == (SDL_CONTROLLER_AXIS_TRIGGERRIGHT + 100);
    }
}

RguiControls::RguiControls(UiMain *ui, Renderer *renderer, Font *font)
    : m_ui(ui), m_renderer(renderer) {
    m_menu = new RguiMenu(renderer, font, "Controls", {});
    m_prompt = new RguiBindingPrompt(renderer, font);
    m_menu->add(m_prompt);
}

RguiControls::~RguiControls() {
    delete m_menu;
}

void RguiControls::refresh(bool in_game) {
    m_in_game = in_game;

    std::vector<RguiMenuItem> items;
    items.reserve(sizeof(CONTROL_ITEMS) / sizeof(CONTROL_ITEMS[0]));
    for (const auto &item : CONTROL_ITEMS) {
        items.push_back({item.label, getOptionValue(item.opt_id), item.opt_id, false});
    }

    m_menu->setItems(items);
    m_menu->setTitle(in_game ? "Controls - This Game" : "Controls - Global");
}

void RguiControls::cycleDeadZone(int direction) {
    auto *opt = m_ui->getConfig()->get(PEMUConfig::OptId::JOY_DEADZONE, m_in_game);
    if (!opt) {
        return;
    }

    int idx = opt->getArrayIndex() + direction;
    int count = (int)opt->getArray().size();
    if (count <= 0) {
        return;
    }
    if (idx >= count) {
        idx = 0;
    } else if (idx < 0) {
        idx = count - 1;
    }

    opt->setArrayIndex(idx);
    saveCurrentScope();
}

void RguiControls::captureBinding(int opt_id) {
    auto *opt = m_ui->getConfig()->get(opt_id, m_in_game);
    if (!opt) {
        return;
    }

    m_ui->getInput()->clear();

    std::string label = "Control";
    for (const auto &item : CONTROL_ITEMS) {
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

void RguiControls::saveCurrentScope() const {
    if (m_in_game) {
        m_ui->getConfig()->saveGame();
    } else {
        m_ui->getConfig()->save();
    }
    // RGUI navigation should stay on the fixed menu mapping while the in-game
    // menu is open. Per-game bindings are restored when emulation resumes.
    m_ui->updateInputMapping(false);
}

std::string RguiControls::getBindingLabel(int value) const {
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
    if (isTriggerAxis(value)) {
        snprintf(label, sizeof(label), "Axis %d", value - 100);
    } else {
        snprintf(label, sizeof(label), "Button %d", value);
    }
    return label;
}

std::string RguiControls::getOptionValue(int opt_id) const {
    auto *opt = m_ui->getConfig()->get(opt_id, m_in_game);
    if (!opt) {
        return "N/A";
    }

    if (opt_id == PEMUConfig::OptId::JOY_DEADZONE) {
        return opt->getString();
    }

    return getBindingLabel(opt->getInteger());
}

int RguiControls::handleInput(Input *input) {
    const auto action = m_menu->handleInput(input);
    if (action == RguiMenu::CANCEL) {
        return 1;
    }

    if (action != RguiMenu::LEFT && action != RguiMenu::RIGHT && action != RguiMenu::CONFIRM) {
        return -1;
    }

    const int sel = m_menu->getSelectedIndex();
    const int opt_id = m_menu->getSelectedId();

    if (opt_id == PEMUConfig::OptId::JOY_DEADZONE) {
        const int direction = action == RguiMenu::LEFT ? -1 : 1;
        cycleDeadZone(direction);
    } else if (action == RguiMenu::CONFIRM) {
        captureBinding(opt_id);
    }

    refresh(m_in_game);
    m_menu->setSelectedIndex(sel);
    return -1;
}

void RguiControls::draw(Transform &t) {
    m_menu->onDraw(t, true);
}
