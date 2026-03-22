#ifndef FCEUX_VITA_UI_EMU_H
#define FCEUX_VITA_UI_EMU_H

#include <memory>
#include <string>

#include "runtime/ui_emu.h"

class FceuxVitaCoreBridge;

class FceuxVitaUiEmu : public pemu::UiEmu {
public:
    explicit FceuxVitaUiEmu(pemu::UiMain *ui);
    ~FceuxVitaUiEmu() override;

    int load(const pemu::Game &game) override;

    void stop() override;
    void pause() override;
    void resume() override;

    FceuxVitaCoreBridge *getCore() const { return m_core.get(); }

private:
    bool onInput(c2d::Input::Player *players) override;

    void onUpdate() override;

    std::unique_ptr<FceuxVitaCoreBridge> m_core;
};

int fceux_state_load(const char *path);
int fceux_state_save(const char *path);
void fceux_apply_cheats();

#endif // FCEUX_VITA_UI_EMU_H
