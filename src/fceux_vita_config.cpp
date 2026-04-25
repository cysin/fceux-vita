#include <string>
#include "runtime/runtime.h"
#include "fceux_vita_config.h"

using namespace c2d;
using namespace pemu;

#define C2D_CONFIG_RESTART_EMU_NEEDED "YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION"

FceuxVitaConfig::FceuxVitaConfig(Renderer *renderer, const int version)
    : PEMUConfig(renderer, "FCEUX_VITA", version) {
    printf("FceuxVitaConfig(%s, v%i)\n", getPath().c_str(), version);

    c2d::Io *io = renderer->getIo();

    /// MAIN OPTIONS
    get(UI_SHOW_ZIP_NAMES)->setArrayIndex(0);

    // no need for auto-scaling mode
    getOption(PEMUConfig::OptId::EMU_SCALING_MODE)->setArray({"ASPECT", "INTEGER"}, 0);

    /// EMULATION OPTIONS
    const auto group = getGroup(EMULATION);
    if (!group) {
        printf("FceuxVitaConfig: error, group not found (EMULATION)\n");
        return;
    }

    group->addOption({"AUDIO_FREQUENCY", {"22050", "44100", "48000"},
                      2, EMU_AUDIO_FREQ, C2D_CONFIG_RESTART_EMU_NEEDED});
    group->addOption({"FRAMESKIP", {"0", "1", "2", "3", "4", "5"},
                      0, EMU_FRAMESKIP});
    group->addOption({"REWIND", {"OFF", "ON"},
                      0, EMU_REWIND});

    // Reload config so new rom paths are immediately visible
    PEMUConfig::load();

    // Add ROM path
    addRomPath("NES", io->getDataPath() + "roms/", {0, 0, "NES"});

    // Save newly added rom paths
    PEMUConfig::save();

    // Create rom paths if needed
    const auto paths = getRomPaths();
    for (const auto &path: paths) {
        io->create(path.path);
    }
}
