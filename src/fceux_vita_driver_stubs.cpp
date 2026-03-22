//
// FCEUD_* driver interface stubs for Vita
//
// Implements all functions the FCEUX core expects from the platform driver.
//

#include "types.h"
#include "driver.h"
#include "state.h"
#include "emufile.h"
#include "file.h"
#include "input.h"
#include "drivers/common/configSys.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

// ---------------------------------------------------------------------------
// Global variables expected by FCEUX core (extern'd in fceu.h / sound.h)
// ---------------------------------------------------------------------------
bool swapDuty = false;
int dendy = 0;
int KillFCEUXonFrame = 0;
int32 fps_scale = 256; // 1x speed (256 = normal in FCEUX's scale)
int32 fps_scale_unpaused = 256;
int32 fps_scale_frameadvance = 0;

// ---------------------------------------------------------------------------
// Palette storage + RGB565 LUT shared with the core bridge
// ---------------------------------------------------------------------------
static uint8 s_palette[256][3]; // RGB888
uint16_t g_palette_rgb565[256]; // RGB565 lookup table

void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b) {
    s_palette[index][0] = r;
    s_palette[index][1] = g;
    s_palette[index][2] = b;
    g_palette_rgb565[index] = (uint16_t)(((r & 0xF8) << 8) |
                                          ((g & 0xFC) << 3) |
                                          (b >> 3));
}

void FCEUD_GetPalette(uint8 i, uint8 *r, uint8 *g, uint8 *b) {
    *r = s_palette[i][0];
    *g = s_palette[i][1];
    *b = s_palette[i][2];
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------
FILE *FCEUD_UTF8fopen(const char *fn, const char *mode) {
    return fopen(fn, mode);
}

EMUFILE_FILE *FCEUD_UTF8_fstream(const char *n, const char *m) {
    return new EMUFILE_FILE(n, m);
}

// ---------------------------------------------------------------------------
// Archive stubs (we handle archives in the bridge layer)
// ---------------------------------------------------------------------------
FCEUFILE *FCEUD_OpenArchiveIndex(ArchiveScanRecord &asr, std::string &fname, int innerIndex) {
    return nullptr;
}

FCEUFILE *FCEUD_OpenArchiveIndex(ArchiveScanRecord &asr, std::string &fname, int innerIndex, int *userCancel) {
    return nullptr;
}

FCEUFILE *FCEUD_OpenArchive(ArchiveScanRecord &asr, std::string &fname, std::string *innerFilename) {
    return nullptr;
}

FCEUFILE *FCEUD_OpenArchive(ArchiveScanRecord &asr, std::string &fname, std::string *innerFilename, int *userCancel) {
    return nullptr;
}

ArchiveScanRecord FCEUD_ScanArchive(std::string fname) {
    return ArchiveScanRecord();
}

// ---------------------------------------------------------------------------
// Logging / messages
// ---------------------------------------------------------------------------
void FCEUD_PrintError(const char *s) {
    fprintf(stderr, "FCEUX Error: %s\n", s ? s : "(null)");
}

void FCEUD_Message(const char *s) {
    printf("%s", s ? s : "");
}

const char *FCEUD_GetCompilerString() {
    return "GCC (Vita ARM)";
}

// ---------------------------------------------------------------------------
// Network stubs (no netplay on Vita)
// ---------------------------------------------------------------------------
int FCEUD_SendData(void *data, uint32 len) { return 0; }
int FCEUD_RecvData(void *data, uint32 len) { return 0; }
void FCEUD_NetplayText(uint8 *text) {}
void FCEUD_NetworkClose(void) {}

// ---------------------------------------------------------------------------
// Sound stubs
// ---------------------------------------------------------------------------
void FCEUD_SoundToggle(void) {}
void FCEUD_SoundVolumeAdjust(int n) {}

// ---------------------------------------------------------------------------
// Save state dialog stubs
// ---------------------------------------------------------------------------
void FCEUD_SaveStateAs(void) {}
void FCEUD_LoadStateFrom(void) {}

// ---------------------------------------------------------------------------
// Input configuration stub
// ---------------------------------------------------------------------------
void FCEUD_SetInput(bool fourscore, bool microphone, ESI port0, ESI port1, ESIFC fcexp) {}

// ---------------------------------------------------------------------------
// Movie / AVI / Lua stubs (not supported on Vita)
// ---------------------------------------------------------------------------
void FCEUD_MovieRecordTo(void) {}
void FCEUD_MovieReplayFrom(void) {}
void FCEUD_LuaRunFrom(void) {}
void FCEUD_AviRecordTo(void) {}
void FCEUD_AviStop(void) {}

// ---------------------------------------------------------------------------
// UI / display stubs
// ---------------------------------------------------------------------------
bool FCEUD_ShouldDrawInputAids() { return false; }
void FCEUD_OnCloseGame(void) {}
void FCEUD_SetEmulationSpeed(int cmd) {}
void FCEUD_TurboOn(void) {}
void FCEUD_TurboOff(void) {}
void FCEUD_TurboToggle(void) {}
int FCEUD_ShowStatusIcon(void) { return 0; }
void FCEUD_ToggleStatusIcon(void) {}
void FCEUD_HideMenuToggle(void) {}
void FCEUD_CmdOpen(void) {}
bool FCEUD_PauseAfterPlayback() { return false; }
void FCEUD_VideoChanged() {}

// ---------------------------------------------------------------------------
// Debug stubs (no debugger on Vita)
// ---------------------------------------------------------------------------
void FCEUD_DebugBreakpoint(int bp_num) {}
void FCEUD_TraceInstruction(uint8 *opcode, int size) {}
void FCEUD_FlushTrace() {}
void FCEUD_UpdateNTView(int scanline, bool drawall) {}
void FCEUD_UpdatePPUView(int scanline, int drawall) {}

// ---------------------------------------------------------------------------
// Time functions (used by video.cpp for FPS display)
// ---------------------------------------------------------------------------
uint64 FCEUD_GetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

uint64 FCEUD_GetTimeFreq(void) {
    return 1000; // milliseconds
}

// ---------------------------------------------------------------------------
// Throttle (called by fceu.cpp when region changes)
// ---------------------------------------------------------------------------
void RefreshThrottleFPS(void) {
    // No-op on Vita — frame timing handled by cross2d
}

// ---------------------------------------------------------------------------
// Additional globals expected by FCEUX core (defined in driver files normally)
// ---------------------------------------------------------------------------
int pal_emulation = 0;
bool turbo = false;
int eoptions = 0;
int closeFinishedMovie = 0;
bool paldeemphswap = false;

// ModernDeemphColorMap (normally in vidblit.cpp, used by ppu.cpp)
uint32 ModernDeemphColorMap(const uint8 *src, const uint8 *srcbuf, int scale) {
    // Simplified: just return the base color without deemph processing
    return src ? *src : 0;
}

// ---------------------------------------------------------------------------
// AVI stubs (declared in driver.h, referenced by core video/wave code)
// ---------------------------------------------------------------------------
bool FCEUI_AviIsRecording(void) { return false; }
bool FCEUI_AviEnableHUDrecording(void) { return false; }
void FCEUI_SetAviEnableHUDrecording(bool enable) {}
bool FCEUI_AviDisableMovieMessages(void) { return false; }
void FCEUI_SetAviDisableMovieMessages(bool disable) {}
void FCEUI_AviVideoUpdate(const unsigned char *buffer) {}
int FCEUI_AviBegin(const char *fname) { return 0; }
void FCEUI_AviEnd(void) {}
void FCEUI_AviSoundUpdate(void *soundData, int soundLen) {}

// ---------------------------------------------------------------------------
// Input preset stub (called from input.cpp)
// ---------------------------------------------------------------------------
void FCEUI_UseInputPreset(int preset) {}

// ---------------------------------------------------------------------------
// Keyboard/mouse stubs (called by some input device drivers)
// ---------------------------------------------------------------------------
static unsigned int s_keyboard_state[256] = {0};
unsigned int *GetKeyboard(void) {
    return s_keyboard_state;
}

void GetMouseData(uint32 (&d)[3]) {
    d[0] = 0;
    d[1] = 0;
    d[2] = 0;
}
