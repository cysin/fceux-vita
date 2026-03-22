#ifndef FCEUX_VITA_IO_H
#define FCEUX_VITA_IO_H

#include "cross2d/c2d.h"

extern void FceuxPathsInit(c2d::C2DIo *io);

namespace c2d {
    class FceuxVitaIo : public c2d::C2DIo {
    public:
        FceuxVitaIo() : C2DIo() {
            C2DIo::create(FceuxVitaIo::getDataPath());
            C2DIo::create(FceuxVitaIo::getDataPath() + "configs");
            C2DIo::create(FceuxVitaIo::getDataPath() + "saves");
            C2DIo::create(FceuxVitaIo::getDataPath() + "roms");
            C2DIo::create(FceuxVitaIo::getDataPath() + "states");
            C2DIo::create(FceuxVitaIo::getDataPath() + "cheats");
            C2DIo::create(FceuxVitaIo::getDataPath() + "screenshots");
            C2DIo::create(FceuxVitaIo::getDataPath() + "tmp");
            FceuxPathsInit(this);
        }

        ~FceuxVitaIo() override {
            printf("~FceuxVitaIo()\n");
        }

        std::string getDataPath() override {
            return "ux0:/data/fceux-vita/";
        }
    };
}

#endif
