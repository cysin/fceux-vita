#ifndef FCEUX_VITA_CORE_BRIDGE_H
#define FCEUX_VITA_CORE_BRIDGE_H

#include <memory>
#include <string>
#include <vector>

#include "cross2d/c2d.h"

struct FceuxCheat;
class FceuxVitaUiEmu;

class FceuxVitaCoreBridge {
public:
    explicit FceuxVitaCoreBridge(FceuxVitaUiEmu *emu);
    ~FceuxVitaCoreBridge();

    int load(const std::string &full_path);
    void unload();

    void execFrame(c2d::Input::Player *players);

    int saveState(const char *path);
    int loadState(const char *path);
    void applyCheats(const std::vector<FceuxCheat> &cheats);

    void reset(bool hard);
    void suspendHotkeysUntilRelease();

    float getTargetFps() const;
    bool isPal() const;
    const std::string &getLastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif
