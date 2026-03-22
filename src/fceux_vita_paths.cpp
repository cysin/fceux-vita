#include "cross2d/c2d.h"

#define MAX_PATH 512

char szFceuxHomePath[MAX_PATH];
char szFceuxRomPath[MAX_PATH];
char szFceuxSavePath[MAX_PATH];
char szFceuxConfigPath[MAX_PATH];
char szFceuxStatePath[MAX_PATH];
char szFceuxCheatPath[MAX_PATH];

// Alias for RGUI files that reference szAppRomPath / szAppConfigPath
char szAppRomPath[MAX_PATH];
char szAppConfigPath[MAX_PATH];

void FceuxPathsInit(c2d::C2DIo *io) {
    printf("FceuxPathsInit: dataPath = %s\n", io->getDataPath().c_str());

    snprintf(szFceuxHomePath, MAX_PATH - 1, "%s", io->getDataPath().c_str());
    io->create(szFceuxHomePath);

    snprintf(szFceuxRomPath, MAX_PATH - 1, "%sroms/", szFceuxHomePath);
    io->create(szFceuxRomPath);
    snprintf(szAppRomPath, MAX_PATH - 1, "%s", szFceuxRomPath);

    snprintf(szFceuxSavePath, MAX_PATH - 1, "%ssaves/", szFceuxHomePath);
    io->create(szFceuxSavePath);

    snprintf(szFceuxConfigPath, MAX_PATH - 1, "%sconfigs/", szFceuxHomePath);
    io->create(szFceuxConfigPath);
    snprintf(szAppConfigPath, MAX_PATH - 1, "%s", szFceuxConfigPath);

    snprintf(szFceuxStatePath, MAX_PATH - 1, "%sstates/", szFceuxHomePath);
    io->create(szFceuxStatePath);

    snprintf(szFceuxCheatPath, MAX_PATH - 1, "%scheats/", szFceuxHomePath);
    io->create(szFceuxCheatPath);
}
