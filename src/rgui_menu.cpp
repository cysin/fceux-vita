//
// RGUI-style menu component for fceux-vita
//

#include "rgui_menu.h"
#include "rgui_theme.h"

using namespace c2d;
using namespace fceux_vita::rgui_theme;

RguiMenu::RguiMenu(Renderer *renderer, Font *font, const std::string &title,
                     const std::vector<RguiMenuItem> &items)
    : RectangleShape(renderer->getSize()), m_renderer(renderer), m_font(font), m_title(title), m_items(items) {
    RectangleShape::setFillColor(Color::Transparent);
    memset(m_highlights, 0, sizeof(m_highlights));
    memset(m_labels, 0, sizeof(m_labels));
    memset(m_values, 0, sizeof(m_values));
    createChildren();
}

RguiMenu::~RguiMenu() = default;

void RguiMenu::setItems(const std::vector<RguiMenuItem> &items) {
    const int previous_index = m_selected;
    const int previous_id = getSelectedId();

    m_items = items;

    if (m_items.empty()) {
        m_selected = 0;
        m_scroll_offset = 0;
        m_dirty = true;
        return;
    }

    int next_selected = -1;
    for (int i = 0; i < (int)m_items.size(); i++) {
        if (m_items[i].id == previous_id) {
            next_selected = i;
            break;
        }
    }

    if (next_selected < 0) {
        next_selected = std::max(0, std::min(previous_index, (int)m_items.size() - 1));
    }

    m_selected = next_selected;
    m_scroll_offset = 0;
    int visible = getVisibleRows();
    if (m_selected >= visible) {
        m_scroll_offset = m_selected - visible + 1;
    }
    m_dirty = true;
}

void RguiMenu::setTitle(const std::string &title) {
    m_title = title;
    m_dirty = true;
}

int RguiMenu::getSelectedIndex() const {
    return m_selected;
}

int RguiMenu::getSelectedId() const {
    if (m_selected >= 0 && m_selected < (int)m_items.size()) {
        return m_items[m_selected].id;
    }
    return -1;
}

void RguiMenu::setSelectedIndex(int idx) {
    if (idx >= 0 && idx < (int)m_items.size()) {
        m_selected = idx;
        int visible = getVisibleRows();
        if (m_selected < m_scroll_offset) {
            m_scroll_offset = m_selected;
        } else if (m_selected >= m_scroll_offset + visible) {
            m_scroll_offset = m_selected - visible + 1;
        }
        m_dirty = true;
    }
}

void RguiMenu::setSelectionState(int idx, int scrollOffset) {
    if (m_items.empty()) {
        m_selected = 0;
        m_scroll_offset = 0;
        m_dirty = true;
        return;
    }

    m_selected = std::max(0, std::min(idx, (int)m_items.size() - 1));
    int visible = getVisibleRows();
    int maxScroll = std::max(0, (int)m_items.size() - visible);
    m_scroll_offset = std::max(0, std::min(scrollOffset, maxScroll));

    if (m_selected < m_scroll_offset) {
        m_scroll_offset = m_selected;
    } else if (m_selected >= m_scroll_offset + visible) {
        m_scroll_offset = std::max(0, m_selected - visible + 1);
    }

    m_dirty = true;
}

int RguiMenu::getScrollOffset() const {
    return m_scroll_offset;
}

int RguiMenu::getVisibleRowCount() const {
    return getVisibleRows();
}

int RguiMenu::getVisibleRows() const {
    float h = m_renderer->getSize().y;
    int rows = (int)((h - TITLE_HEIGHT - MARGIN * 2) / ROW_HEIGHT);
    if (rows > MAX_VISIBLE) rows = MAX_VISIBLE;
    return rows;
}

RguiMenu::Action RguiMenu::handleInput(Input *input) {
    unsigned int buttons = input->getButtons();

    if (buttons & Input::Button::Up) {
        m_selected--;
        if (m_selected < 0) m_selected = (int)m_items.size() - 1;
        int visible = getVisibleRows();
        if (m_selected < m_scroll_offset) {
            m_scroll_offset = m_selected;
        } else if (m_selected >= m_scroll_offset + visible) {
            m_scroll_offset = m_selected - visible + 1;
        }
        m_dirty = true;
        return NONE;
    }

    if (buttons & Input::Button::Down) {
        m_selected++;
        if (m_selected >= (int)m_items.size()) m_selected = 0;
        int visible = getVisibleRows();
        if (m_selected >= m_scroll_offset + visible) {
            m_scroll_offset = m_selected - visible + 1;
        } else if (m_selected < m_scroll_offset) {
            m_scroll_offset = m_selected;
        }
        m_dirty = true;
        return NONE;
    }

    if (buttons & Input::Button::A) {
        return CONFIRM;
    }

    if (buttons & Input::Button::B) {
        return CANCEL;
    }

    if (buttons & Input::Button::Left) {
        return LEFT;
    }

    if (buttons & Input::Button::Right) {
        return RIGHT;
    }

    return NONE;
}

void RguiMenu::createChildren() {
    float w = m_renderer->getSize().x;
    float h = m_renderer->getSize().y;
    Font *font = m_font;

    // background
    m_bg = new RectangleShape({w, h});
    m_bg->setFillColor(Background);
    m_bg->setPosition(0, 0);
    add(m_bg);

    // title bar
    m_title_bar = new RectangleShape({w, (float)TITLE_HEIGHT});
    m_title_bar->setFillColor(TitleBar);
    m_title_bar->setPosition(0, 0);
    add(m_title_bar);

    // title text
    m_title_text = new Text(m_title, FONT_SIZE + 2, font);
    m_title_text->setFillColor(TitleText);
    m_title_text->setPosition(MARGIN, (TITLE_HEIGHT - (FONT_SIZE + 2)) / 2.0f);
    add(m_title_text);

    // scroll indicators
    m_scroll_up = new Text("^", FONT_SIZE, font);
    m_scroll_up->setFillColor(Accent);
    m_scroll_up->setPosition(w - 20, TITLE_HEIGHT + 2);
    m_scroll_up->setVisibility(Visibility::Hidden);
    add(m_scroll_up);

    m_scroll_down = new Text("v", FONT_SIZE, font);
    m_scroll_down->setFillColor(Accent);
    m_scroll_down->setPosition(w - 20, h - FONT_SIZE - 4);
    m_scroll_down->setVisibility(Visibility::Hidden);
    add(m_scroll_down);

    // pre-create row elements
    int maxRows = getVisibleRows();
    for (int i = 0; i < maxRows && i < MAX_VISIBLE; i++) {
        float y = (float)TITLE_HEIGHT + MARGIN + (float)(i * ROW_HEIGHT);

        m_highlights[i] = new RectangleShape({w - MARGIN * 2, (float)ROW_HEIGHT});
        m_highlights[i]->setFillColor(Highlight);
        m_highlights[i]->setPosition(MARGIN, y);
        m_highlights[i]->setVisibility(Visibility::Hidden);
        add(m_highlights[i]);

        m_labels[i] = new Text("", FONT_SIZE, font);
        m_labels[i]->setFillColor(ItemText);
        m_labels[i]->setPosition(MARGIN * 2, y + (ROW_HEIGHT - FONT_SIZE) / 2.0f);
        m_labels[i]->setVisibility(Visibility::Hidden);
        add(m_labels[i]);

        m_values[i] = new Text("", FONT_SIZE, font);
        m_values[i]->setFillColor(ValueText);
        m_values[i]->setPosition(0, y + (ROW_HEIGHT - FONT_SIZE) / 2.0f);
        m_values[i]->setVisibility(Visibility::Hidden);
        add(m_values[i]);
    }
}

void RguiMenu::updateChildren() {
    float w = m_renderer->getSize().x;
    int visible = getVisibleRows();

    m_title_text->setString(m_title);

    // scroll indicators
    m_scroll_up->setVisibility(m_scroll_offset > 0 ? Visibility::Visible : Visibility::Hidden);
    m_scroll_down->setVisibility(
        m_scroll_offset + visible < (int)m_items.size() ? Visibility::Visible : Visibility::Hidden);

    int end = std::min(m_scroll_offset + visible, (int)m_items.size());
    int row = 0;

    for (int i = m_scroll_offset; i < end && row < MAX_VISIBLE; i++, row++) {
        const auto &item = m_items[i];
        bool selected = (i == m_selected);

        // highlight
        if (m_highlights[row]) {
            m_highlights[row]->setVisibility(selected ? Visibility::Visible : Visibility::Hidden);
        }

        // label
        if (m_labels[row]) {
            m_labels[row]->setString(item.label);
            m_labels[row]->setFillColor(selected ? TitleText : ItemText);
            m_labels[row]->setVisibility(Visibility::Visible);
        }

        // value
        if (m_values[row]) {
            std::string right_text;
            if (item.is_submenu) {
                right_text = ">";
            } else if (!item.value.empty()) {
                right_text = "< " + item.value + " >";
            }

            if (!right_text.empty()) {
                m_values[row]->setString(right_text);
                m_values[row]->setFillColor(selected ? TitleText : ValueText);
                float tw = m_values[row]->getLocalBounds().width;
                float y = m_values[row]->getPosition().y;
                m_values[row]->setPosition(w - MARGIN * 2 - tw, y);
                m_values[row]->setVisibility(Visibility::Visible);
            } else {
                m_values[row]->setVisibility(Visibility::Hidden);
            }
        }
    }

    // hide unused rows
    for (; row < MAX_VISIBLE; row++) {
        if (m_highlights[row]) m_highlights[row]->setVisibility(Visibility::Hidden);
        if (m_labels[row]) m_labels[row]->setVisibility(Visibility::Hidden);
        if (m_values[row]) m_values[row]->setVisibility(Visibility::Hidden);
    }

    m_dirty = false;
}

void RguiMenu::rebuild() {
    m_dirty = true;
}

void RguiMenu::onDraw(Transform &t, bool draw) {
    if (m_dirty) {
        updateChildren();
    }
    // update children (needed because RguiMenu is not in the scene graph,
    // so the renderer's onUpdate() traversal doesn't reach Text children
    // which need onUpdate() to build their vertex geometry)
    C2DObject::onUpdate();
    RectangleShape::onDraw(t, draw);
}
