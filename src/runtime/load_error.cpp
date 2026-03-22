#include "load_error.h"

#include <utility>

#include "runtime.h"

namespace {

std::string g_title;
std::string g_summary;
std::string g_detail;

std::string trimTrailingWhitespace(std::string text) {
    while (!text.empty()) {
        const char c = text.back();
        if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
            text.pop_back();
            continue;
        }
        break;
    }
    return text;
}

}

namespace fceux_vita::load_error {

void clear() {
    g_title.clear();
    g_summary.clear();
    g_detail.clear();
}

void set(std::string title, std::string summary, std::string detail) {
    g_title = std::move(title);
    g_summary = trimTrailingWhitespace(std::move(summary));
    g_detail = trimTrailingWhitespace(std::move(detail));
}

bool has() {
    return !g_title.empty() || !g_summary.empty() || !g_detail.empty();
}

std::string getTitle() {
    return g_title.empty() ? "ERROR" : g_title;
}

std::string getMessage() {
    std::string message = g_summary;
    if (!g_detail.empty()) {
        if (!message.empty()) {
            message += "\n\n";
        }
        message += g_detail;
    }
    if (message.empty()) {
        message = "DRIVER INIT FAILED";
    }
    return message;
}

void show(pemu::UiMain *ui, const pemu::Game *game) {
    if (!ui) {
        clear();
        return;
    }

    std::string message = getMessage();
    if (game && !game->path.empty()) {
        if (!message.empty()) {
            message += "\n\n";
        }
        message += "ROM: ";
        message += game->romsPath + game->path;
    }

    ui->getUiProgressBox()->setVisibility(c2d::Visibility::Hidden);
    ui->getUiMessageBox()->show(getTitle(), message, "OK");
    clear();
}

}
