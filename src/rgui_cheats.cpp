//
// RGUI cheats menu - FCEUX version (Game Genie codes)
//

#include "rgui_cheats.h"
#include "fceux_vita_ui_emu.h"
#include <cstdio>
#include <cstring>

using namespace c2d;

std::vector<FceuxCheat> g_fceux_cheats;

namespace {
    void applyAllCheats() {
        fceux_apply_cheats();
    }
}

RguiCheats::RguiCheats(Renderer *renderer, Font *font)
    : m_renderer(renderer) {
    m_menu = new RguiMenu(renderer, font, "Cheats (Game Genie)", {});
}

RguiCheats::~RguiCheats() {
    delete m_menu;
}

void RguiCheats::refresh() {
    std::vector<RguiMenuItem> items;

    if (g_fceux_cheats.empty()) {
        RguiMenuItem item;
        item.label = "No cheats loaded";
        item.id = -1;
        items.push_back(item);
    } else {
        for (int i = 0; i < (int)g_fceux_cheats.size(); i++) {
            RguiMenuItem item;
            if (g_fceux_cheats[i].name.empty()) {
                item.label = g_fceux_cheats[i].gg_code;
            } else {
                item.label = g_fceux_cheats[i].name;
            }
            item.value = g_fceux_cheats[i].enabled ? "ON" : "OFF";
            item.id = i;
            items.push_back(item);
        }
    }

    m_menu->setItems(items);
}

int RguiCheats::handleInput(Input *input) {
    auto action = m_menu->handleInput(input);

    if (action == RguiMenu::CANCEL) {
        return 1;
    }

    if (g_fceux_cheats.empty()) return -1;

    if (action == RguiMenu::LEFT || action == RguiMenu::RIGHT ||
        action == RguiMenu::CONFIRM) {
        int sel = m_menu->getSelectedIndex();
        if (sel < 0 || sel >= (int)g_fceux_cheats.size()) return -1;

        // Toggle enabled state
        g_fceux_cheats[sel].enabled = !g_fceux_cheats[sel].enabled;
        applyAllCheats();

        refresh();
        m_menu->setSelectedIndex(sel);
    }

    return -1;
}

void RguiCheats::saveState(const char *romName) {
    if (!romName || !romName[0]) return;
    if (g_fceux_cheats.empty()) return;

    extern char szAppConfigPath[];
    char path[1024];
    snprintf(path, sizeof(path), "%s%s_cheats.cfg", szAppConfigPath, romName);

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (const auto &cheat : g_fceux_cheats) {
        // Format: GG_CODE=ON/OFF;name
        fprintf(f, "%s=%s;%s\n",
                cheat.gg_code.c_str(),
                cheat.enabled ? "ON" : "OFF",
                cheat.name.c_str());
    }

    fclose(f);
}

void RguiCheats::loadState(const char *romName) {
    if (!romName || !romName[0]) return;

    extern char szAppConfigPath[];
    char path[1024];
    snprintf(path, sizeof(path), "%s%s_cheats.cfg", szAppConfigPath, romName);

    g_fceux_cheats.clear();

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;

        // Parse: GG_CODE=ON/OFF;name
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        std::string gg_code = line;
        char *rest = eq + 1;

        char *semi = strchr(rest, ';');
        bool enabled = false;
        std::string name;

        if (semi) {
            *semi = '\0';
            enabled = (strcmp(rest, "ON") == 0);
            name = semi + 1;
        } else {
            enabled = (strcmp(rest, "ON") == 0);
        }

        // Validate Game Genie code length (6 or 8 chars)
        if (gg_code.length() == 6 || gg_code.length() == 8) {
            g_fceux_cheats.push_back({name, gg_code, enabled});
        }
    }

    fclose(f);

    applyAllCheats();
}

void RguiCheats::draw(Transform &t) {
    m_menu->onDraw(t, true);
}
