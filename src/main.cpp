#include "main.h"
#include "rgui_main.h"

#ifdef __VITA__
#include <psp2/kernel/processmgr.h>
#endif

using namespace c2d;
using namespace pemu;

PEMUUiMain *pemu_ui;
RguiMain *g_rgui = nullptr;

int main(int argc, char **argv) {
    Game game;

    const auto io = new PEMUIo();

    pemu_ui = new PEMUUiMain(Vector2f{0, 0});
    pemu_ui->setIo(io);

    constexpr int version = (__PEMU_VERSION_MAJOR__ * 100) + __PEMU_VERSION_MINOR__;
    const auto cfg = new PEMUConfig(pemu_ui, version);
    pemu_ui->setConfig(cfg);

    const auto skin = new PEMUSkin(pemu_ui);
    pemu_ui->setSkin(skin);

    if (argc > 1) {
        if (io->exist(argv[1])) {
            game.path = Utility::baseName(argv[1]);
            game.name = Utility::removeExt(game.path);
            game.romsPath = Utility::remove(argv[1], game.path);
        } else {
            printf("main: file provided as console argument does not exist (%s)\n", argv[1]);
            delete skin;
            delete cfg;
            delete pemu_ui;
            return 1;
        }
    }

    const auto uiEmu = new PEMUUiEmu(pemu_ui);
    pemu_ui->init(uiEmu);

    g_rgui = new RguiMain(pemu_ui);
    g_rgui->setLayer(100);
    pemu_ui->add(g_rgui);

    if (!game.path.empty()) {
        cfg->loadGame(game);
        if (uiEmu->load(game) == 0) {
            uiEmu->setExitOnStop(true);
        } else {
            cfg->clearGame();
            pemu_ui->updateInputMapping(false);
            g_rgui->show(false);
            fceux_vita::load_error::show(pemu_ui, &game);
        }
    } else {
        g_rgui->show(false);
    }

    while (!pemu_ui->done) {
        pemu_ui->flip();
    }

    pemu_ui->remove(g_rgui);
    delete g_rgui;
    g_rgui = nullptr;

    delete pemu_ui;
    delete skin;
    delete cfg;

    return 0;
}
