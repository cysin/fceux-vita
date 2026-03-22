//
// Created by cpasjuste on 04/02/18.
//

#include <algorithm>

#include "runtime.h"
#include "../rgui_theme.h"

using namespace fceux_vita::rgui_theme;

UIProgressBox::UIProgressBox(UiMain *ui)
        : SkinnedRectangle(ui, {"SKIN_CONFIG", "MESSAGEBOX"}) {
    float w = ui->getSize().x;
    float h = ui->getSize().y;
    float content_width = w * 0.84f;
    float title_font_size = std::max(28.0f, h / 15.0f);
    float message_font_size = std::max(22.0f, h / 24.0f);
    float progress_width = w * 0.56f;
    float progress_height = std::max(12.0f, h / 48.0f);
    float progress_x = (w - progress_width) / 2.0f;
    float progress_y = h * 0.78f;

    setOrigin(Origin::TopLeft);
    setPosition(0, 0);
    setSize(w, h);
    setFillColor(Background);
    setOutlineThickness(0);

    m_title = new Text("TITLE", (unsigned int) title_font_size, ui->getSkin()->getFont());
    m_title->setPosition(w * 0.5f, h * 0.35f);
    m_title->setOrigin(Origin::Center);
    m_title->setSizeMax(content_width, h * 0.28f);
    m_title->setOverflow(Text::NewLine);
    m_title->setLineSpacingModifier((int) std::max(4.0f, title_font_size * 0.15f));
    m_title->setFillColor(TitleText);
    m_title->setOutlineThickness(0);
    add(m_title);

    m_message = new Text("MESSAGE", (unsigned int) message_font_size, ui->getSkin()->getFont());
    m_message->setPosition(w * 0.5f, h * 0.56f);
    m_message->setOrigin(Origin::Center);
    m_message->setSizeMax(content_width, h * 0.14f);
    m_message->setOverflow(Text::NewLine);
    m_message->setLineSpacingModifier((int) std::max(2.0f, message_font_size * 0.12f));
    m_message->setFillColor(ItemText);
    m_message->setOutlineThickness(0);
    add(m_message);

    m_progress_bg = new RectangleShape({progress_x, progress_y, progress_width, progress_height});
    m_progress_bg->setFillColor(TitleBar);
    m_progress_bg->setOutlineThickness(0);
    add(m_progress_bg);

    m_progress_fg = new RectangleShape(
            FloatRect(m_progress_bg->getPosition().x, m_progress_bg->getPosition().y,
                      0, m_progress_bg->getSize().y));
    m_progress_fg->setFillColor(Accent);
    add(m_progress_fg);

    setVisibility(Visibility::Hidden);
}

void UIProgressBox::setTitle(const std::string &title) {
    m_title->setString(title);
    m_title->setOrigin(c2d::Origin::Center);
}

void UIProgressBox::setProgress(float progress) {
    progress = std::max(0.0f, std::min(progress, 1.0f));
    float width = progress * m_progress_bg->getSize().x;
    m_progress_fg->setSize(
            std::min(width, m_progress_bg->getSize().x),
            m_progress_fg->getSize().y);
}

void UIProgressBox::setMessage(const std::string &message) {
    m_message->setString(message);
    m_message->setOrigin(c2d::Origin::Center);
}

c2d::Text *UIProgressBox::getTitleText() {
    return m_title;
}

c2d::Text *UIProgressBox::getMessageText() {
    return m_message;
}
