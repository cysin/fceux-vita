//
// RGUI-style menu component for fceux-vita
//

#ifndef FCEUX_VITA_RGUI_MENU_H
#define FCEUX_VITA_RGUI_MENU_H

#include "cross2d/c2d.h"
#include <string>
#include <vector>

struct RguiMenuItem {
    std::string label;
    std::string value;   // right-side value text (empty = none)
    int id = 0;
    bool is_submenu = false;
};

class RguiMenu : public c2d::RectangleShape {
public:
    // action codes returned by handleInput
    enum Action { NONE = -1, CONFIRM = 0, CANCEL = 1, LEFT = 2, RIGHT = 3 };

    RguiMenu(c2d::Renderer *renderer, c2d::Font *font, const std::string &title,
             const std::vector<RguiMenuItem> &items);
    ~RguiMenu() override;

    void setItems(const std::vector<RguiMenuItem> &items);
    void setTitle(const std::string &title);
    int getSelectedIndex() const;
    int getSelectedId() const;
    void setSelectedIndex(int idx);
    void setSelectionState(int idx, int scrollOffset);
    int getScrollOffset() const;
    int getVisibleRowCount() const;

    Action handleInput(c2d::Input *input);

    void rebuild();

    void onDraw(c2d::Transform &t, bool draw) override;

private:

    c2d::Renderer *m_renderer;
    c2d::Font *m_font;
    std::string m_title;
    std::vector<RguiMenuItem> m_items;
    int m_selected = 0;
    int m_scroll_offset = 0;
    bool m_dirty = true;

    static constexpr int TITLE_HEIGHT = 38;
    static constexpr int ROW_HEIGHT = 30;
    static constexpr int FONT_SIZE = 22;
    static constexpr int MARGIN = 10;
    static constexpr int MAX_VISIBLE = 24;

    // persistent child objects for rendering
    c2d::RectangleShape *m_bg = nullptr;
    c2d::RectangleShape *m_title_bar = nullptr;
    c2d::Text *m_title_text = nullptr;
    c2d::RectangleShape *m_highlights[MAX_VISIBLE]{};
    c2d::Text *m_labels[MAX_VISIBLE]{};
    c2d::Text *m_values[MAX_VISIBLE]{};
    c2d::Text *m_scroll_up = nullptr;
    c2d::Text *m_scroll_down = nullptr;

    int getVisibleRows() const;
    void createChildren();
    void updateChildren();
};

#endif // FCEUX_VITA_RGUI_MENU_H
