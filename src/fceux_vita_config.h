#ifndef FCEUX_VITA_CONFIG_H
#define FCEUX_VITA_CONFIG_H

#include "runtime/pemu_config.h"

class FceuxVitaConfig final : public pemu::PEMUConfig {
public:
    FceuxVitaConfig(c2d::Renderer *renderer, int version);

    std::string getCoreVersion() override {
        return "FCEUX 2.7.0";
    }

    std::vector<std::string> getCoreSupportedExt() override {
        return {".zip", ".nes", ".nez", ".unf", ".unif", ".fds", ".nsf"};
    }
};

#endif // FCEUX_VITA_CONFIG_H
