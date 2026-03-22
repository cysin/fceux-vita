#ifndef FCEUX_VITA_MAIN_H
#define FCEUX_VITA_MAIN_H

#include "runtime/runtime.h"
#include "fceux_vita_ui_emu.h"
#include "fceux_vita_config.h"
#include "fceux_vita_io.h"

#define PEMUIo FceuxVitaIo
#define PEMUConfig FceuxVitaConfig
#define PEMUSkin pemu::Skin
#define PEMUUiMain pemu::UiMain
#define PEMUUiEmu FceuxVitaUiEmu

#endif
