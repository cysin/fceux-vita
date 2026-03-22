//
// RGUI file browser for ROM selection
//

#ifndef FCEUX_VITA_RGUI_FILEBROWSER_H
#define FCEUX_VITA_RGUI_FILEBROWSER_H

#include "rgui_menu.h"
#include "rgui_theme.h"
#include "cross2d/c2d.h"
#include <map>
#include <string>
#include <vector>

class RguiFileBrowser {
public:
    RguiFileBrowser(c2d::Renderer *renderer, c2d::Font *font, const std::string &rootPath,
                    const std::string &startPath,
                    const std::vector<std::string> &extensions);
    ~RguiFileBrowser();

    // returns: 0=file selected, 1=cancelled, -1=navigating
    int handleInput(c2d::Input *input);

    void draw(c2d::Transform &t);

    std::string getSelectedPath() const;
    std::string getCurrentPath() const;

    void setPath(const std::string &path, const std::string &selectName = {});

private:
    struct DirectoryState {
        int selected_index = 0;
        int scroll_offset = 0;
        std::string selected_entry_name;
    };

    struct JumpTarget {
        std::string label;
        int entry_index = -1;
    };

    void refreshList();
    void saveCurrentState();
    void restoreCurrentState();
    void updateJumpTargets();
    void updateQuickJumpOverlay();
    void beginQuickJump();
    int handleQuickJump(c2d::Input *input);
    int getEntryIndexByName(const std::string &name) const;
    static std::string getJumpKey(const std::string &name);

    c2d::Renderer *m_renderer;
    c2d::Font *m_font;
    RguiMenu *m_menu;
    std::string m_root_path;
    std::string m_current_path;
    std::vector<std::string> m_extensions;
    std::vector<c2d::Io::File> m_entries;
    std::string m_selected_file;
    std::map<std::string, DirectoryState> m_directory_states;
    std::vector<JumpTarget> m_jump_targets;
    int m_jump_index = 0;
    bool m_quick_jump_active = false;
    c2d::RectangleShape *m_jump_panel = nullptr;
    c2d::Text *m_jump_title = nullptr;
    c2d::Text *m_jump_value = nullptr;
    c2d::Text *m_jump_help = nullptr;
};

#endif // FCEUX_VITA_RGUI_FILEBROWSER_H
