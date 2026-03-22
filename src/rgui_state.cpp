//
// RGUI save/load state menu - FCEUX version
//

#include "rgui_state.h"
#include "runtime/runtime.h"

using namespace c2d;

extern int fceux_state_load(const char *path);
extern int fceux_state_save(const char *path);

RguiStateMenu::RguiStateMenu(Renderer *renderer, Font *font, pemu::UiMain *ui)
    : m_renderer(renderer), m_ui(ui) {
    m_menu = new RguiMenu(renderer, font, "Save State", {});
}

RguiStateMenu::~RguiStateMenu() {
    delete m_menu;
}

void RguiStateMenu::setMode(Mode mode) {
    m_mode = mode;
    m_menu->setTitle(mode == SAVE ? "Save State" : "Load State");
    refresh();
}

void RguiStateMenu::refresh() {
    auto *emu = m_ui->getUiEmu();
    if (!emu) return;

    auto game = emu->getCurrentGame();
    std::string baseName = Utility::removeExt(game.path);
    std::string dataPath = m_renderer->getIo()->getDataPath();

    std::vector<RguiMenuItem> items;
    for (int i = 0; i < SLOT_COUNT; i++) {
        char pathBuf[1024];
        char shotBuf[1024];
        snprintf(pathBuf, sizeof(pathBuf), "%ssaves/%s-%i.state",
                 dataPath.c_str(), baseName.c_str(), i);
        snprintf(shotBuf, sizeof(shotBuf), "%ssaves/%s-%i.png",
                 dataPath.c_str(), baseName.c_str(), i);
        m_paths[i] = pathBuf;
        m_shots[i] = shotBuf;

        RguiMenuItem item;
        item.id = i;
        bool exists = m_renderer->getIo()->exist(pathBuf);
        char label[64];
        snprintf(label, sizeof(label), "Slot %d", i);
        item.label = label;
        item.value = exists ? "USED" : "EMPTY";
        items.push_back(item);
    }
    m_menu->setItems(items);
}

int RguiStateMenu::handleInput(Input *input) {
    auto action = m_menu->handleInput(input);

    if (action == RguiMenu::CONFIRM) {
        int slot = m_menu->getSelectedIndex();
        if (slot < 0 || slot >= SLOT_COUNT) return -1;

        if (m_mode == SAVE) {
            printf("RguiState: saving to %s\n", m_paths[slot].c_str());
            if (fceux_state_save(m_paths[slot].c_str()) == 0) {
                auto *emu = m_ui->getUiEmu();
                if (emu && emu->getVideo()) {
                    emu->getVideo()->save(m_shots[slot]);
                }
            }
            return 0;
        } else {
            bool exists = m_renderer->getIo()->exist(m_paths[slot]);
            if (exists) {
                printf("RguiState: loading from %s\n", m_paths[slot].c_str());
                fceux_state_load(m_paths[slot].c_str());
                return 0;
            }
        }
    } else if (action == RguiMenu::CANCEL) {
        return 1;
    }

    return -1;
}

void RguiStateMenu::draw(Transform &t) {
    m_menu->onDraw(t, true);
}
