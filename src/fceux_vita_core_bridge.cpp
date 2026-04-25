#include "fceux_vita_core_bridge.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <zlib.h>
#include <SDL.h>
#include <archive.h>
#include <archive_entry.h>

// FCEUX headers must come before runtime headers due to JOY_* macro conflicts
#include "fceu.h"
#include "driver.h"
#include "state.h"
#include "emufile.h"
#include "cheat.h"
#include "input.h"
#include "git.h"
#include "video.h"

// Save FCEUX JOY_* values then undef macros to avoid conflict with pemu_config.h enum
static constexpr uint32_t FCEUX_JOY_A      = JOY_A;
static constexpr uint32_t FCEUX_JOY_B      = JOY_B;
static constexpr uint32_t FCEUX_JOY_SELECT = JOY_SELECT;
static constexpr uint32_t FCEUX_JOY_START  = JOY_START;
static constexpr uint32_t FCEUX_JOY_UP     = JOY_UP;
static constexpr uint32_t FCEUX_JOY_DOWN   = JOY_DOWN;
static constexpr uint32_t FCEUX_JOY_LEFT   = JOY_LEFT;
static constexpr uint32_t FCEUX_JOY_RIGHT  = JOY_RIGHT;
#undef JOY_A
#undef JOY_B
#undef JOY_SELECT
#undef JOY_START
#undef JOY_UP
#undef JOY_DOWN
#undef JOY_LEFT
#undef JOY_RIGHT

#include "fceux_vita_ui_emu.h"
#include "rgui_cheats.h"
#include "runtime/runtime.h"

using namespace c2d;
using namespace pemu;

// RGB565 palette LUT defined in driver stubs, updated by FCEUD_SetPalette
extern uint16_t g_palette_rgb565[256];
extern uint32_t g_palette_rgb565_pair[256 * 256];
extern int dendy;

namespace {

constexpr size_t kArchiveReadChunk = 16384;

std::string basename_from_path(const std::string &path) {
    const size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string stem_from_path(const std::string &path) {
    std::string base = basename_from_path(path);
    const size_t pos = base.find_last_of('.');
    return pos == std::string::npos ? base : base.substr(0, pos);
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string extension_from_path(const std::string &path) {
    const std::string base = basename_from_path(path);
    const size_t pos = base.find_last_of('.');
    return pos == std::string::npos ? std::string{} : lowercase(base.substr(pos));
}

bool is_supported_rom_ext(const std::string &path) {
    const std::string ext = extension_from_path(path);
    return ext == ".nes" || ext == ".nez" || ext == ".unf" || ext == ".unif" ||
           ext == ".fds" || ext == ".nsf" || ext == ".bin";
}

bool is_archive_ext(const std::string &path) {
    const std::string ext = extension_from_path(path);
    return ext == ".zip" || ext == ".7z" || ext == ".gz" || ext == ".bz2" ||
           ext == ".xz" || ext == ".rar";
}

bool extract_archive_first_rom(const std::string &archive_path, const std::string &tmp_dir,
                                std::string &out_path) {
    archive *arc = archive_read_new();
    archive_read_support_filter_all(arc);
    archive_read_support_format_all(arc);
    archive_read_support_format_raw(arc);

    if (archive_read_open_filename(arc, archive_path.c_str(), kArchiveReadChunk) != ARCHIVE_OK) {
        archive_read_free(arc);
        return false;
    }

    archive_entry *entry = nullptr;
    bool extracted = false;

    while (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
        const char *entry_name = archive_entry_pathname(entry);
        const std::string current_name = entry_name ? entry_name : "";
        const bool direct_stream = current_name == "data" && !archive_entry_size_is_set(entry);

        if (!direct_stream && !is_supported_rom_ext(current_name)) {
            archive_read_data_skip(arc);
            continue;
        }

        // Determine output filename
        std::string out_name = direct_stream ?
            stem_from_path(archive_path) + ".nes" :
            basename_from_path(current_name);
        out_path = tmp_dir + out_name;

        FILE *f = fopen(out_path.c_str(), "wb");
        if (!f) {
            archive_read_data_skip(arc);
            continue;
        }

        std::vector<uint8_t> chunk(kArchiveReadChunk);
        la_ssize_t read_size;
        while ((read_size = archive_read_data(arc, chunk.data(), chunk.size())) > 0) {
            fwrite(chunk.data(), 1, static_cast<size_t>(read_size), f);
        }

        fclose(f);
        extracted = (read_size >= 0);
        break;
    }

    archive_read_free(arc);
    return extracted;
}

bool is_trigger_axis_binding(int value) {
    return value == (SDL_CONTROLLER_AXIS_TRIGGERLEFT + 100) ||
           value == (SDL_CONTROLLER_AXIS_TRIGGERRIGHT + 100);
}

bool is_raw_binding_pressed(const Input::Player &player, int binding) {
    if (!player.data || binding == SDL_CONTROLLER_BUTTON_INVALID) {
        return false;
    }

    auto *pad = static_cast<SDL_GameController *>(player.data);
    if (is_trigger_axis_binding(binding)) {
        return SDL_GameControllerGetAxis(pad, static_cast<SDL_GameControllerAxis>(binding - 100)) > player.dz;
    }

    return SDL_GameControllerGetButton(pad, static_cast<SDL_GameControllerButton>(binding)) > 0;
}

// ---------------------------------------------------------------------------
// Lightweight rewind ring buffer
// Saves compressed savestates every frame into a fixed-size ring buffer.
// During rewind: loads previous states, emulates one frame forward for audio.
// ---------------------------------------------------------------------------
constexpr int REWIND_SECONDS = 20;
constexpr int REWIND_FPS = 60;
constexpr int REWIND_BUF_SIZE = REWIND_SECONDS * REWIND_FPS; // 1200 slots

struct RewindBuffer {
    struct Slot {
        std::vector<uint8_t> data;
        bool valid = false;
    };

    Slot slots[REWIND_BUF_SIZE];
    int head = 0;   // next write position
    int count = 0;  // number of valid entries

    void reset() {
        head = 0;
        count = 0;
        for (auto &s : slots) {
            s.valid = false;
            s.data.clear();
        }
    }

    void save_state() {
        EMUFILE_MEMORY em(0x4000);
        if (!FCEUSS_SaveMS(&em, Z_BEST_SPEED)) {
            return;
        }

        Slot &slot = slots[head];
        const size_t sz = em.size();
        slot.data.resize(sz);
        memcpy(slot.data.data(), em.buf(), sz);
        slot.valid = true;

        head = (head + 1) % REWIND_BUF_SIZE;
        if (count < REWIND_BUF_SIZE) count++;
    }

    // Load the most recent state and pop it. Returns false if empty.
    bool load_prev() {
        if (count <= 1) return false; // keep at least 1 so we can resume

        // Move head back to the last written slot
        head = (head - 1 + REWIND_BUF_SIZE) % REWIND_BUF_SIZE;
        count--;

        // The slot to load is one before current head (the previous frame)
        int idx = (head - 1 + REWIND_BUF_SIZE) % REWIND_BUF_SIZE;
        Slot &slot = slots[idx];
        if (!slot.valid || slot.data.empty()) return false;

        EMUFILE_MEMORY em(slot.data.data(), slot.data.size());
        return FCEUSS_LoadFP(&em, SSLOADPARAM_NOBACKUP);
    }
};

} // namespace

struct FceuxVitaCoreBridge::Impl {
    explicit Impl(FceuxVitaUiEmu *owner)
        : emu(owner) {}

    FceuxVitaUiEmu *emu = nullptr;
    std::string full_path;
    std::string game_name;
    std::string data_path;
    std::string save_path;
    std::string states_path;
    std::string cheats_path;
    std::string tmp_path;
    std::string last_error;
    std::string tmp_rom_path;  // extracted archive ROM
    uint32 paddata[2] = {0, 0};
    int audio_rate = 48000;
    int frame_skip_counter = 0;
    float target_fps = 60.0f;
    bool initialized = false;
    bool loaded = false;
    bool rewind_enabled = false;
    bool rewind_active = false;
    bool rewind_blocked_until_release = true;
    RewindBuffer rewind_buf;

    void set_last_error(const std::string &message) {
        last_error = message;
        while (!last_error.empty() &&
               (last_error.back() == '\n' || last_error.back() == '\r')) {
            last_error.pop_back();
        }
        if (last_error.empty()) {
            last_error = "Could not load ROM.";
        }
    }

    void ensure_directories() {
        auto *io = emu->getUi()->getIo();
        data_path = io->getDataPath();
        save_path = data_path + "saves/";
        states_path = data_path + "states/";
        cheats_path = data_path + "cheats/";
        tmp_path = data_path + "tmp/";

        io->create(data_path);
        io->create(data_path + "configs");
        io->create(save_path);
        io->create(states_path);
        io->create(cheats_path);
        io->create(data_path + "roms");
        io->create(tmp_path);
    }

    bool init_core() {
        if (initialized) {
            return true;
        }

        if (!FCEUI_Initialize()) {
            set_last_error("Failed to initialize FCEUX core.");
            return false;
        }

        FCEUI_SetBaseDirectory(data_path);

        // Override directories for saves, states, cheats
        // FCEUI_SetDirOverride takes char* (non-const)
        static char save_dir_buf[512];
        static char states_dir_buf[512];
        static char cheats_dir_buf[512];
        snprintf(save_dir_buf, sizeof(save_dir_buf), "%s", save_path.c_str());
        snprintf(states_dir_buf, sizeof(states_dir_buf), "%s", states_path.c_str());
        snprintf(cheats_dir_buf, sizeof(cheats_dir_buf), "%s", cheats_path.c_str());
        FCEUI_SetDirOverride(FCEUIOD_NV, save_dir_buf);
        FCEUI_SetDirOverride(FCEUIOD_STATES, states_dir_buf);
        FCEUI_SetDirOverride(FCEUIOD_CHEATS, cheats_dir_buf);

        initialized = true;
        return true;
    }

    void configure_from_settings() {
        auto *config = emu->getUi()->getConfig();

        // Audio rate
        audio_rate = config->get(PEMUConfig::OptId::EMU_AUDIO_FREQ, true)->getInteger();
        if (audio_rate <= 0) audio_rate = 48000;
        FCEUI_Sound(audio_rate);

        // Rendered scanlines (show full 240 lines)
        FCEUI_SetRenderedLines(0, 239, 0, 239);
    }

    bool load_rom(const std::string &path) {
        std::string rom_path = path;

        // Handle archives: extract to tmp file
        if (is_archive_ext(path)) {
            tmp_rom_path.clear();
            if (!extract_archive_first_rom(path, tmp_path, tmp_rom_path)) {
                set_last_error("Archive did not contain a supported NES image.");
                return false;
            }
            rom_path = tmp_rom_path;
        }

        FCEUGI *gi = FCEUI_LoadGame(rom_path.c_str(), 1);
        if (!gi) {
            set_last_error("FCEUX could not load the ROM file.");
            return false;
        }

        // Determine if PAL from game info
        target_fps = gi->vidsys ? 50.0f : 60.0f;

        return true;
    }

    void setup_input() {
        paddata[0] = 0;
        paddata[1] = 0;
        FCEUI_SetInput(0, SI_GAMEPAD, &paddata[0], 0);
        FCEUI_SetInput(1, SI_GAMEPAD, &paddata[1], 0);
        FCEUI_SetInputFC(SIFC_NONE, nullptr, 0);
        FCEUI_SetInputFourscore(false);
    }

    void build_video_surface() {
        // FCEUX NES resolution: 256 wide, scanlines as configured
        const int width = 256;
        const int first_scanline = FSettings.FirstSLine;
        const int last_scanline = FSettings.LastSLine;
        const int height = last_scanline - first_scanline + 1;

        // NES pixel aspect ratio ~8:7 for NTSC
        const int aspect_w = 8000;
        const int aspect_h = 7000;

        const Vector2i size(width, height);
        emu->addVideo(nullptr, nullptr, size, {aspect_w, aspect_h},
                      Texture::Format::RGB565);
    }

    void configure_audio() {
        const int spf = audio_rate / (target_fps > 55.0f ? 60 : 50);
        emu->addAudio(audio_rate, spf);
    }

    void start_rewind() {
        rewind_enabled = true;
        rewind_buf.reset();
        rewind_active = false;
        rewind_blocked_until_release = true;
        printf("fceux: rewind enabled (%d seconds, %d slots)\n",
               REWIND_SECONDS, REWIND_BUF_SIZE);
    }

    void stop_rewind() {
        rewind_enabled = false;
        rewind_active = false;
        rewind_buf.reset();
    }

    int get_frameskip() const {
        auto *opt = emu->getUi()->getConfig()->get(PEMUConfig::OptId::EMU_FRAMESKIP, true);
        return opt ? std::clamp(opt->getInteger(), 0, 5) : 0;
    }

    int next_frame_skip() {
        const int frameskip = get_frameskip();
        if (frameskip <= 0) {
            frame_skip_counter = 0;
            return 0;
        }

        if (frame_skip_counter > frameskip) {
            frame_skip_counter = 0;
        }

        const int skip = frame_skip_counter > 0 ? 1 : 0;
        frame_skip_counter = (frame_skip_counter + 1) % (frameskip + 1);
        return skip;
    }

    bool is_rewind_config_enabled() const {
        auto *opt = emu->getUi()->getConfig()->get(PEMUConfig::OptId::EMU_REWIND, true);
        return opt && opt->getInteger();
    }

    void update_rewind_enabled() {
        const bool enabled = is_rewind_config_enabled();
        if (enabled && !rewind_enabled) {
            start_rewind();
        } else if (!enabled && rewind_enabled) {
            stop_rewind();
        }
    }

    int get_rewind_binding() const {
        auto *opt = emu->getUi()->getConfig()->get(PEMUConfig::OptId::JOY_REWIND, false);
        return opt ? opt->getInteger() : KEY_JOY_LT_DEFAULT;
    }

    void suspend_hotkeys_until_release() {
        rewind_active = false;
        rewind_blocked_until_release = true;
    }

    // Returns true if rewinding this frame
    bool check_rewind(const Input::Player &player) {
        bool pressed = is_raw_binding_pressed(player, get_rewind_binding());

        // Block until first release after menu/pause
        if (rewind_blocked_until_release) {
            if (!pressed) {
                rewind_blocked_until_release = false;
            }
            pressed = false;
        }

        if (pressed && !rewind_active) {
            rewind_active = true;
        } else if (!pressed && rewind_active) {
            rewind_active = false;
        }

        return rewind_active;
    }

    void upload_video_frame(uint8 *xbuf) {
        if (!emu->getVideo() || !xbuf) {
            return;
        }

        const int first_scanline = FSettings.FirstSLine;
        const int last_scanline = FSettings.LastSLine;
        const int height = last_scanline - first_scanline + 1;
        const int width = 256;

        uint8_t *dst_pixels = nullptr;
        int pitch = 0;
        emu->getVideo()->lock(&dst_pixels, &pitch, emu->getVideo()->getTextureRect());
        if (!dst_pixels) return;

        for (int y = 0; y < height; ++y) {
            auto *dst_row = reinterpret_cast<uint32_t *>(dst_pixels + y * pitch);
            const uint8 *src_row = xbuf + (first_scanline + y) * 256;
            for (int x = 0; x < width; x += 2) {
                dst_row[x >> 1] = g_palette_rgb565_pair[src_row[x] | (src_row[x + 1] << 8)];
            }
        }

        emu->getVideo()->unlock();
    }

    void push_audio(int32 *soundbuf, int32 ssize) {
        if (!emu->getAudio() || !soundbuf || ssize <= 0) {
            return;
        }

        // FCEUX outputs mono int32 samples, convert to stereo int16
        // Use a stack buffer for small frame sizes (typical: ~800 samples)
        int16_t stereo_buf[2048 * 2];
        const int count = std::min(ssize, (int32)2048);

        for (int i = 0; i < count; ++i) {
            int32 sample = soundbuf[i];
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            stereo_buf[i * 2] = static_cast<int16_t>(sample);
            stereo_buf[i * 2 + 1] = static_cast<int16_t>(sample);
        }

        const Audio::SyncMode sync_mode =
            (target_fps < 55.0f) ? Audio::SyncMode::LowLatency : Audio::SyncMode::None;
        emu->getAudio()->play(stereo_buf, count, sync_mode);
    }

    uint32 build_pad_state(const Input::Player &player) {
        uint32 pad = 0;

        // NES A = Vita Circle (c2d B), NES B = Vita Cross (c2d A)
        if (player.buttons & Input::Button::B)      pad |= FCEUX_JOY_A;
        if (player.buttons & Input::Button::A)      pad |= FCEUX_JOY_B;
        if (player.buttons & Input::Button::Select)  pad |= FCEUX_JOY_SELECT;
        if (player.buttons & Input::Button::Start)   pad |= FCEUX_JOY_START;
        if (player.buttons & Input::Button::Up)      pad |= FCEUX_JOY_UP;
        if (player.buttons & Input::Button::Down)    pad |= FCEUX_JOY_DOWN;
        if (player.buttons & Input::Button::Left)    pad |= FCEUX_JOY_LEFT;
        if (player.buttons & Input::Button::Right)   pad |= FCEUX_JOY_RIGHT;

        // Turbo buttons: Y = Turbo A, X = Turbo B
        // Uses FCEUX built-in AutoFire (rapidAlternator toggled each frame by AutoFire())
        if ((player.buttons & Input::Button::Y) && GetAutoFireState(0)) pad |= FCEUX_JOY_A;
        if ((player.buttons & Input::Button::X) && GetAutoFireState(1)) pad |= FCEUX_JOY_B;

        return pad;
    }

    void exec_frame(Input::Player *players) {
        if (!loaded) return;

        update_rewind_enabled();

        bool rewinding = rewind_enabled && players && check_rewind(players[0]);

        if (rewinding) {
            // Rewind: load previous state, then emulate one frame forward for audio
            if (rewind_buf.load_prev()) {
                // Show the restored frame's video
                upload_video_frame(XBuf);

                // Emulate one frame forward to produce smooth audio
                // (zero input so the game doesn't advance meaningfully)
                paddata[0] = 0;
                paddata[1] = 0;
                uint8 *xbuf = nullptr;
                int32 *soundbuf = nullptr;
                int32 soundbufsize = 0;
                FCEUI_Emulate(&xbuf, &soundbuf, &soundbufsize, 0);
                push_audio(soundbuf, soundbufsize);

                // Don't save this forward-emulated frame to rewind buffer
            } else {
                // Nothing left to rewind, just show current frame
                upload_video_frame(XBuf);
            }
            return;
        }

        // Normal play: update input
        if (players) {
            paddata[0] = build_pad_state(players[0]);
            paddata[1] = build_pad_state(players[1]);
        } else {
            paddata[0] = 0;
            paddata[1] = 0;
        }

        if (rewind_enabled) {
            rewind_buf.save_state();
        }

        // Run one frame
        uint8 *xbuf = nullptr;
        int32 *soundbuf = nullptr;
        int32 soundbufsize = 0;
        const int skip = next_frame_skip();
        FCEUI_Emulate(&xbuf, &soundbuf, &soundbufsize, skip);

        // Output video and audio
        if (!skip) {
            upload_video_frame(xbuf);
        }
        push_audio(soundbuf, soundbufsize);
    }

    void cleanup_tmp() {
        if (!tmp_rom_path.empty()) {
            remove(tmp_rom_path.c_str());
            tmp_rom_path.clear();
        }
    }
};

FceuxVitaCoreBridge::FceuxVitaCoreBridge(FceuxVitaUiEmu *emu)
    : m_impl(std::make_unique<Impl>(emu)) {
}

FceuxVitaCoreBridge::~FceuxVitaCoreBridge() {
    unload();
    if (m_impl->initialized) {
        FCEUI_Kill();
        m_impl->initialized = false;
    }
}

int FceuxVitaCoreBridge::load(const std::string &full_path) {
    m_impl->full_path = full_path;
    m_impl->game_name = stem_from_path(full_path);
    m_impl->last_error.clear();

    m_impl->ensure_directories();

    if (!m_impl->init_core()) {
        return -1;
    }

    m_impl->configure_from_settings();

    if (!m_impl->load_rom(full_path)) {
        return -1;
    }

    m_impl->loaded = true;
    m_impl->frame_skip_counter = 0;
    m_impl->setup_input();
    m_impl->build_video_surface();
    m_impl->configure_audio();
    m_impl->update_rewind_enabled();

    return 0;
}

void FceuxVitaCoreBridge::unload() {
    if (!m_impl->loaded) {
        return;
    }

    m_impl->stop_rewind();
    FCEUI_CloseGame();
    m_impl->loaded = false;
    m_impl->cleanup_tmp();
}

void FceuxVitaCoreBridge::execFrame(c2d::Input::Player *players) {
    m_impl->exec_frame(players);
}

int FceuxVitaCoreBridge::saveState(const char *path) {
    if (!m_impl->loaded) return -1;
    FCEUI_SaveState(path, false);
    return 0;
}

int FceuxVitaCoreBridge::loadState(const char *path) {
    if (!m_impl->loaded) return -1;
    FCEUI_LoadState(path, false);
    return 0;
}

void FceuxVitaCoreBridge::applyCheats(const std::vector<FceuxCheat> &cheats) {
    if (!m_impl->loaded) return;

    // Clear all existing cheats
    int num_cheats = 0;
    FCEUI_ListCheats([](const char *, uint32, uint8, int, int, int, void *data) -> int {
        (*static_cast<int *>(data))++;
        return 1;
    }, &num_cheats);

    // Delete from the end to avoid index shifting issues
    for (int i = num_cheats - 1; i >= 0; --i) {
        FCEUI_DelCheat(static_cast<uint32>(i));
    }

    // Add enabled cheats
    for (const auto &cheat : cheats) {
        if (!cheat.enabled || cheat.gg_code.empty()) continue;

        int addr = 0, val = 0, compare = -1;
        if (FCEUI_DecodeGG(cheat.gg_code.c_str(), &addr, &val, &compare) == 0) {
            // Decode failed
            continue;
        }
        FCEUI_AddCheat(cheat.gg_code.c_str(),
                       static_cast<uint32>(addr),
                       static_cast<uint8>(val),
                       compare, 1); // type=1 (substitute)
    }
}

void FceuxVitaCoreBridge::reset(bool hard) {
    if (!m_impl->loaded) return;
    if (hard) {
        FCEUI_PowerNES();
    } else {
        FCEUI_ResetNES();
    }
}

void FceuxVitaCoreBridge::suspendHotkeysUntilRelease() {
    m_impl->suspend_hotkeys_until_release();
}

float FceuxVitaCoreBridge::getTargetFps() const {
    return m_impl->target_fps;
}

bool FceuxVitaCoreBridge::isPal() const {
    return m_impl->target_fps < 55.0f;
}

const std::string &FceuxVitaCoreBridge::getLastError() const {
    return m_impl->last_error;
}
