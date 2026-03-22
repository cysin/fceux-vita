//
// RGUI file browser for ROM selection
//

#include "rgui_filebrowser.h"
#include <algorithm>
#include <cctype>

using namespace c2d;
using namespace fceux_vita::rgui_theme;

namespace {
std::string normalizeDirectoryPath(const std::string &path) {
    if (path.empty()) {
        return {};
    }

    std::string normalized = path;
    if (normalized.back() != '/') {
        normalized += "/";
    }
    return normalized;
}

std::string getParentPath(const std::string &currentPath) {
    std::string path = currentPath;
    if (path.length() > 1 && path.back() == '/') {
        path.pop_back();
    }

    auto colon = path.rfind(':');
    if (colon != std::string::npos) {
        const size_t rootLength = colon + 2; // "ux0:/"
        if (path.length() <= rootLength) {
            return path.substr(0, rootLength);
        }

        auto pos = path.rfind('/');
        if (pos != std::string::npos) {
            if (pos == colon + 1) {
                return path.substr(0, rootLength);
            }
            return path.substr(0, pos + 1);
        }
    }

    auto pos = path.rfind('/');
    if (pos != std::string::npos) {
        return path.substr(0, pos + 1);
    }

    return currentPath;
}
}

RguiFileBrowser::RguiFileBrowser(Renderer *renderer, Font *font, const std::string &rootPath,
                                 const std::string &startPath,
                                 const std::vector<std::string> &extensions)
    : m_renderer(renderer),
      m_font(font),
      m_root_path(normalizeDirectoryPath(rootPath)),
      m_current_path(normalizeDirectoryPath(startPath)),
      m_extensions(extensions) {
    if (m_root_path.empty()) {
        m_root_path = "ux0:/";
    }
    if (m_current_path.empty()) {
        m_current_path = m_root_path;
    }

    m_menu = new RguiMenu(renderer, font, "Browse: " + m_current_path, {});

    m_jump_panel = new RectangleShape({520.0f, 160.0f});
    m_jump_panel->setFillColor(Background);
    m_jump_panel->setOutlineColor(Highlight);
    m_jump_panel->setOutlineThickness(2.0f);
    m_jump_panel->setOrigin(Origin::Center);
    m_jump_panel->setVisibility(Visibility::Hidden);

    m_jump_title = new Text("Quick Jump", 22, font);
    m_jump_title->setFillColor(TitleText);
    m_jump_title->setVisibility(Visibility::Hidden);

    m_jump_value = new Text("A", 34, font);
    m_jump_value->setFillColor(ItemText);
    m_jump_value->setVisibility(Visibility::Hidden);

    m_jump_help = new Text("Left/Right change   Cross jump   Circle cancel", 18, font);
    m_jump_help->setFillColor(ValueText);
    m_jump_help->setVisibility(Visibility::Hidden);

    m_menu->add(m_jump_panel);
    m_menu->add(m_jump_title);
    m_menu->add(m_jump_value);
    m_menu->add(m_jump_help);

    updateQuickJumpOverlay();
    refreshList();
}

RguiFileBrowser::~RguiFileBrowser() {
    delete m_menu;
}

void RguiFileBrowser::refreshList() {
    m_entries.clear();
    std::vector<RguiMenuItem> items;

    auto dirList = m_renderer->getIo()->getDirList(m_current_path, true);

    // add parent directory entry
    {
        RguiMenuItem item;
        item.label = "..";
        item.id = -1;
        item.is_submenu = true;
        items.push_back(item);
        m_entries.emplace_back("..", m_current_path + "..", Io::Type::Directory);
    }

    std::sort(dirList.begin(), dirList.end(), Io::compare);

    int id = 0;
    for (const auto &entry : dirList) {
        if (entry.name == "." || entry.name == "..") {
            continue;
        }

        if (entry.type == Io::Type::Directory) {
            items.push_back({"[" + entry.name + "]", "", id++, true});
            m_entries.push_back(entry);
            continue;
        }

        bool match = m_extensions.empty();
        if (!match) {
            std::string nameLower = Utility::toLower(entry.name);
            for (const auto &ext : m_extensions) {
                std::string extLower = Utility::toLower(ext);
                if (nameLower.length() >= extLower.length() &&
                    nameLower.compare(nameLower.length() - extLower.length(),
                                      extLower.length(), extLower) == 0) {
                    match = true;
                    break;
                }
            }
        }

        if (match) {
            items.push_back({entry.name, "", id++, false});
            m_entries.push_back(entry);
        }
    }

    m_menu->setItems(items);
    m_menu->setTitle("Browse: " + m_current_path);
    restoreCurrentState();
    updateJumpTargets();
}

void RguiFileBrowser::saveCurrentState() {
    DirectoryState &state = m_directory_states[m_current_path];
    state.selected_index = m_menu->getSelectedIndex();
    state.scroll_offset = m_menu->getScrollOffset();

    int idx = m_menu->getSelectedIndex();
    if (idx >= 0 && idx < (int) m_entries.size()) {
        state.selected_entry_name = m_entries[idx].name;
    } else {
        state.selected_entry_name.clear();
    }
}

void RguiFileBrowser::restoreCurrentState() {
    auto it = m_directory_states.find(m_current_path);
    if (it == m_directory_states.end()) {
        m_menu->setSelectionState(0, 0);
        return;
    }

    int selectedIndex = it->second.selected_index;
    int scrollOffset = it->second.scroll_offset;

    if (!it->second.selected_entry_name.empty()) {
        int nameIndex = getEntryIndexByName(it->second.selected_entry_name);
        if (nameIndex >= 0) {
            selectedIndex = nameIndex;
            // Center the selection in the visible area
            int visible = m_menu->getVisibleRowCount();
            scrollOffset = std::max(0, selectedIndex - visible / 2);
        }
    }

    m_menu->setSelectionState(selectedIndex, scrollOffset);
}

void RguiFileBrowser::updateJumpTargets() {
    m_jump_targets.clear();
    std::map<std::string, int> firstMatches;

    for (int i = 1; i < (int) m_entries.size(); i++) {
        std::string key = getJumpKey(m_entries[i].name);
        if (!key.empty() && !firstMatches.count(key)) {
            firstMatches[key] = i;
        }
    }

    if (firstMatches.count("0-9")) {
        m_jump_targets.push_back({"0-9", firstMatches["0-9"]});
    }
    for (char letter = 'A'; letter <= 'Z'; letter++) {
        std::string key(1, letter);
        auto it = firstMatches.find(key);
        if (it != firstMatches.end()) {
            m_jump_targets.push_back({key, it->second});
        }
    }

    if (m_jump_targets.empty()) {
        m_jump_index = 0;
    } else {
        m_jump_index = std::max(0, std::min(m_jump_index, (int) m_jump_targets.size() - 1));
    }
}

void RguiFileBrowser::updateQuickJumpOverlay() {
    float screenWidth = m_renderer->getSize().x;
    float screenHeight = m_renderer->getSize().y;
    float panelWidth = 520.0f;
    float panelHeight = 160.0f;
    float panelX = screenWidth / 2.0f;
    float panelY = screenHeight / 2.0f;

    m_jump_panel->setSize(panelWidth, panelHeight);
    m_jump_panel->setPosition(panelX, panelY);

    m_jump_title->setString("Quick Jump");
    m_jump_title->setPosition(panelX - panelWidth / 2.0f + 24.0f, panelY - panelHeight / 2.0f + 20.0f);

    std::string value = m_jump_targets.empty() ? "--" : m_jump_targets[m_jump_index].label;
    m_jump_value->setString(value);
    float valueWidth = m_jump_value->getLocalBounds().width;
    m_jump_value->setPosition(panelX - valueWidth / 2.0f, panelY - 12.0f);

    m_jump_help->setString("Left/Right change   Cross jump   Circle cancel");
    float helpWidth = m_jump_help->getLocalBounds().width;
    m_jump_help->setPosition(panelX - helpWidth / 2.0f, panelY + panelHeight / 2.0f - 38.0f);

    Visibility visibility = m_quick_jump_active ? Visibility::Visible : Visibility::Hidden;
    m_jump_panel->setVisibility(visibility);
    m_jump_title->setVisibility(visibility);
    m_jump_value->setVisibility(visibility);
    m_jump_help->setVisibility(visibility);
}

void RguiFileBrowser::beginQuickJump() {
    updateJumpTargets();
    if (m_jump_targets.empty()) {
        return;
    }

    m_quick_jump_active = true;
    int selectedIndex = m_menu->getSelectedIndex();
    if (selectedIndex >= 1 && selectedIndex < (int) m_entries.size()) {
        std::string currentKey = getJumpKey(m_entries[selectedIndex].name);
        for (int i = 0; i < (int) m_jump_targets.size(); i++) {
            if (m_jump_targets[i].label == currentKey) {
                m_jump_index = i;
                break;
            }
        }
    }
    updateQuickJumpOverlay();
}

int RguiFileBrowser::handleQuickJump(Input *input) {
    unsigned int buttons = input->getButtons();

    if (buttons & Input::Button::Left) {
        m_jump_index--;
        if (m_jump_index < 0) {
            m_jump_index = (int) m_jump_targets.size() - 1;
        }
        updateQuickJumpOverlay();
        return -1;
    }

    if (buttons & Input::Button::Right) {
        m_jump_index++;
        if (m_jump_index >= (int) m_jump_targets.size()) {
            m_jump_index = 0;
        }
        updateQuickJumpOverlay();
        return -1;
    }

    if (buttons & Input::Button::A) {
        if (!m_jump_targets.empty()) {
            m_menu->setSelectedIndex(m_jump_targets[m_jump_index].entry_index);
        }
        m_quick_jump_active = false;
        updateQuickJumpOverlay();
        input->clear();
        return -1;
    }

    if ((buttons & Input::Button::B) || (buttons & Input::Button::X)) {
        m_quick_jump_active = false;
        updateQuickJumpOverlay();
        input->clear();
        return -1;
    }

    return -1;
}

int RguiFileBrowser::getEntryIndexByName(const std::string &name) const {
    for (int i = 0; i < (int) m_entries.size(); i++) {
        if (m_entries[i].name == name) {
            return i;
        }
    }
    return -1;
}

std::string RguiFileBrowser::getJumpKey(const std::string &name) {
    if (name.empty()) {
        return {};
    }

    unsigned char first = static_cast<unsigned char>(name[0]);
    if (std::isdigit(first)) {
        return "0-9";
    }
    if (std::isalpha(first)) {
        return std::string(1, (char) std::toupper(first));
    }
    return {};
}

int RguiFileBrowser::handleInput(Input *input) {
    if (m_quick_jump_active) {
        return handleQuickJump(input);
    }

    unsigned int buttons = input->getButtons();

    if (buttons & Input::Button::B) {
        saveCurrentState();
        return 1;
    }

    if (buttons & Input::Button::Y) {
        m_menu->setSelectedIndex(0);
        return -1;
    }

    if (buttons & Input::Button::X) {
        beginQuickJump();
        input->clear();
        return -1;
    }

    if (buttons & Input::Button::Select) {
        saveCurrentState();
        m_current_path = m_root_path;
        m_quick_jump_active = false;
        refreshList();
        return -1;
    }

    int pageSize = std::max(1, m_menu->getVisibleRowCount() - 1);
    if (buttons & Input::Button::LT) {
        m_menu->setSelectedIndex(std::max(0, m_menu->getSelectedIndex() - pageSize));
        return -1;
    }

    if (buttons & Input::Button::RT) {
        m_menu->setSelectedIndex(std::min((int) m_entries.size() - 1,
                                          m_menu->getSelectedIndex() + pageSize));
        return -1;
    }

    auto action = m_menu->handleInput(input);

    if (action == RguiMenu::CONFIRM) {
        int idx = m_menu->getSelectedIndex();
        if (idx >= 0 && idx < (int) m_entries.size()) {
            const auto &entry = m_entries[idx];
            if (entry.type == Io::Type::Directory) {
                saveCurrentState();
                if (entry.name == "..") {
                    m_current_path = getParentPath(m_current_path);
                } else {
                    m_current_path = normalizeDirectoryPath(entry.path);
                }
                refreshList();
                return -1;
            }

            m_selected_file = entry.path;
            saveCurrentState();
            return 0;
        }
    } else if (action == RguiMenu::CANCEL) {
        saveCurrentState();
        return 1;
    }

    return -1;
}

void RguiFileBrowser::draw(Transform &t) {
    m_menu->onDraw(t, true);
}

std::string RguiFileBrowser::getSelectedPath() const {
    return m_selected_file;
}

std::string RguiFileBrowser::getCurrentPath() const {
    return m_current_path;
}

void RguiFileBrowser::setPath(const std::string &path, const std::string &selectName) {
    std::string normalized = normalizeDirectoryPath(path);
    m_current_path = normalized.empty() ? m_root_path : normalized;
    m_quick_jump_active = false;

    // Seed directory state so the cursor lands on selectName after refresh
    if (!selectName.empty() && m_directory_states.find(m_current_path) == m_directory_states.end()) {
        DirectoryState &state = m_directory_states[m_current_path];
        state.selected_entry_name = selectName;
    }

    refreshList();
}
