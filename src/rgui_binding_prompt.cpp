//
// RGUI-style binding prompt used by controls/hotkeys
//

#include "rgui_binding_prompt.h"
#include "rgui_theme.h"
#include <algorithm>
#include <cstdio>

using namespace c2d;
using namespace fceux_vita::rgui_theme;

namespace {
    void drainPendingButtonEvents(Input *input) {
        while (input->waitButton() > -1) {
        }
    }

    bool hasActiveButtons(Input *input) {
        input->update();
        unsigned int buttons = input->getButtons();
        buttons &= ~Input::Button::Delay;
        return buttons != 0;
    }
}

RguiBindingPrompt::RguiBindingPrompt(Renderer *renderer, Font *font)
    : RectangleShape(renderer->getSize()), m_renderer(renderer), m_font(font) {
    RectangleShape::setFillColor(Color::Transparent);
    setVisibility(Visibility::Hidden);
    setLayer(10000);
    createChildren();
    updateLayout();
}

int RguiBindingPrompt::capture(Input *input, const std::string &title, const std::string &message, int timeout_seconds) {
    C2DClock clock;
    int captured = TIMEOUT;

    m_title_text->setString(title);
    m_message_text->setString(message);
    m_hint_text->setString("Press a button or trigger");
    updateLayout();
    setVisibility(Visibility::Visible);

    drainPendingButtonEvents(input);
    c2d_renderer->flip(true, false);

    while (hasActiveButtons(input)) {
        c2d_renderer->flip(true, false);
    }

    drainPendingButtonEvents(input);
    clock.restart();

    while (true) {
        const int elapsed = (int)clock.getElapsedTime().asSeconds();
        if (elapsed >= timeout_seconds) {
            captured = TIMEOUT;
            break;
        }

        char timer[32];
        snprintf(timer, sizeof(timer), "Waiting %d", timeout_seconds - elapsed);
        m_timer_text->setString(timer);
        float timer_width = m_timer_text->getLocalBounds().width;
        m_timer_text->setPosition(
            m_panel->getPosition().x + m_panel->getSize().x - 24.0f - timer_width,
            m_panel->getPosition().y + m_panel->getSize().y - 42.0f
        );

        const int button = input->waitButton();
        if (button > -1) {
            captured = button;
            break;
        }

        c2d_renderer->flip(true, false);
    }

    setVisibility(Visibility::Hidden);
    input->clear();
    return captured;
}

void RguiBindingPrompt::createChildren() {
    m_overlay = new RectangleShape(m_renderer->getSize());
    m_overlay->setFillColor(Overlay);
    add(m_overlay);

    m_panel = new RectangleShape({760.0f, 240.0f});
    m_panel->setFillColor(Background);
    m_panel->setOutlineColor(Highlight);
    m_panel->setOutlineThickness(2.0f);
    add(m_panel);

    m_title_bar = new RectangleShape({760.0f, 44.0f});
    m_title_bar->setFillColor(TitleBar);
    add(m_title_bar);

    m_title_text = new Text("Bind Control", 24, m_font);
    m_title_text->setFillColor(TitleText);
    add(m_title_text);

    m_message_text = new Text("Press a button or trigger", 24, m_font);
    m_message_text->setFillColor(ItemText);
    m_message_text->setOverflow(Text::Overflow::NewLine);
    m_message_text->setLineSpacingModifier(6);
    add(m_message_text);

    m_hint_text = new Text("Press a button or trigger", 20, m_font);
    m_hint_text->setFillColor(ValueText);
    m_hint_text->setOrigin(Origin::Center);
    add(m_hint_text);

    m_timer_text = new Text("Waiting 9", 20, m_font);
    m_timer_text->setFillColor(Accent);
    add(m_timer_text);
}

void RguiBindingPrompt::updateLayout() {
    const Vector2f screen = m_renderer->getSize();
    const float panel_width = std::min(760.0f, screen.x - 80.0f);
    const float panel_height = std::min(240.0f, screen.y - 80.0f);
    const float panel_x = (screen.x - panel_width) * 0.5f;
    const float panel_y = (screen.y - panel_height) * 0.5f;

    m_overlay->setSize(screen);
    m_overlay->setPosition(0.0f, 0.0f);

    m_panel->setSize(panel_width, panel_height);
    m_panel->setPosition(panel_x, panel_y);

    m_title_bar->setSize(panel_width, 44.0f);
    m_title_bar->setPosition(panel_x, panel_y);

    m_title_text->setPosition(panel_x + 20.0f, panel_y + 10.0f);

    m_message_text->setPosition(panel_x + 24.0f, panel_y + 74.0f);
    m_message_text->setSizeMax(panel_width - 48.0f, panel_height - 132.0f);

    m_hint_text->setPosition(panel_x + panel_width * 0.5f, panel_y + panel_height - 38.0f);

    float timer_width = m_timer_text->getLocalBounds().width;
    m_timer_text->setPosition(panel_x + panel_width - 24.0f - timer_width, panel_y + panel_height - 42.0f);
}
