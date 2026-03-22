//
// FceuxVitaUiEmu - FCEUX integration with RGUI menu
//

#include "fceux_vita_ui_emu.h"

#include "fceux_vita_core_bridge.h"
#include "rgui_cheats.h"
#include "rgui_main.h"
#include "runtime/runtime.h"

using namespace c2d;
using namespace pemu;

FceuxVitaUiEmu *uiEmu;

FceuxVitaUiEmu::FceuxVitaUiEmu(UiMain *ui) : UiEmu(ui) {
    printf("FceuxVitaUiEmu()\n");
    uiEmu = this;
    m_core = std::make_unique<FceuxVitaCoreBridge>(this);
}

FceuxVitaUiEmu::~FceuxVitaUiEmu() = default;

namespace {
std::string build_load_message(const std::string &core_error) {
    return core_error.empty() ? "Could not load ROM." : core_error;
}
}

int FceuxVitaUiEmu::load(const Game &game) {
    fceux_vita::load_error::clear();
    currentGame = game;

    getUi()->getUiProgressBox()->setTitle(game.name);
    getUi()->getUiProgressBox()->setMessage("Please wait...");
    getUi()->getUiProgressBox()->setProgress(0);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Visible);
    getUi()->getUiProgressBox()->setLayer(1000);
    getUi()->flip();

    std::string fullPath = game.romsPath + game.path;
    if (m_core->load(fullPath) != 0) {
        getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);
        if (!fceux_vita::load_error::has()) {
            fceux_vita::load_error::set(
                    "Load Failed",
                    build_load_message(m_core->getLastError()),
                    game.path);
        }
        stop();
        return -1;
    }

    targetFps = m_core->getTargetFps();

    getUi()->getUiProgressBox()->setProgress(1);
    getUi()->flip();
    getUi()->delay(500);
    getUi()->getUiProgressBox()->setVisibility(Visibility::Hidden);

    return UiEmu::load(game);
}

void FceuxVitaUiEmu::stop() {
    printf("FceuxVitaUiEmu::stop()\n");
    if (m_core) {
        m_core->unload();
    }
    UiEmu::stop();
}

void FceuxVitaUiEmu::pause() {
    if (m_core) {
        m_core->suspendHotkeysUntilRelease();
    }
    UiEmu::pause();
}

void FceuxVitaUiEmu::resume() {
    if (m_core) {
        m_core->suspendHotkeysUntilRelease();
    }
    UiEmu::resume();
}

extern RguiMain *g_rgui;

bool FceuxVitaUiEmu::onInput(c2d::Input::Player *players) {
    // if RGUI is visible, let it handle input
    if (g_rgui && g_rgui->isVisible()) {
        return g_rgui->onInput(players);
    }

    // intercept menu combo to show RGUI instead of old menu
    if (((players[0].buttons & c2d::Input::Button::Menu1) && (players[0].buttons & c2d::Input::Button::Menu2))) {
        if (g_rgui) {
            pause();
            g_rgui->show(true);
            pMain->getInput()->clear();
            return true;
        }
    }

    // skip base UiEmu::onInput to avoid triggering old menu,
    // go directly to C2DObject::onInput
    return C2DObject::onInput(players);
}

void FceuxVitaUiEmu::onUpdate() {
    if (isPaused()) {
        return;
    }

    auto *players = getUi()->getInput()->getPlayers();
    if (m_core) {
        m_core->execFrame(players);
    }

    UiEmu::onUpdate();
}

int fceux_state_load(const char *path) {
    return (uiEmu && uiEmu->getCore()) ? uiEmu->getCore()->loadState(path) : -1;
}

int fceux_state_save(const char *path) {
    return (uiEmu && uiEmu->getCore()) ? uiEmu->getCore()->saveState(path) : -1;
}

void fceux_apply_cheats() {
    if (uiEmu && uiEmu->getCore()) {
        uiEmu->getCore()->applyCheats(g_fceux_cheats);
    }
}
