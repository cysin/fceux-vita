//
// Created by cpasjuste on 08/12/18.
//

#include "runtime.h"
#include "../rgui_theme.h"

using namespace fceux_vita::rgui_theme;

UiStatusBox::UiStatusBox(UiMain *m)
        : SkinnedRectangle(m, {"SKIN_CONFIG", "STATUSBOX"}) {
    main = m;
    clock = new C2DClock();

    SkinnedRectangle::setCornersRadius(6);
    SkinnedRectangle::setCornerPointCount(6);
    applyStyle();

    text = new Text("TIPS:", (int) (getSize().y * 0.65f), main->getSkin()->getFont());
    text->setOutlineThickness(0.0f);
    text->setOrigin(Origin::Left);
    text->setPosition(6 * main->getScaling().x, getSize().y / 2);
    text->setFillColor(ItemText);
    add(text);

    tween = new TweenAlpha(0, SkinnedRectangle::getAlpha(), 1.0f);
    add(tween);

    setVisibility(Visibility::Hidden);
}

void UiStatusBox::applyStyle() {
    setFillColor(Background);
    setOutlineColor(Highlight);
    setOutlineThickness(2.0f);
}

void UiStatusBox::show(const std::string &t) {
    text->setString(t);
    text->setPosition(6 * main->getScaling().x, getSize().y / 2);
    setSize(text->getLocalBounds().width + (12 * main->getScaling().x), getSize().y);

    clock->restart();
    setVisibility(Visibility::Visible, true);
}

void UiStatusBox::show(const char *fmt, ...) {
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 512, fmt, args);
    va_end(args);
    show(std::string(msg));
}

void UiStatusBox::hide() {
    clock->restart();
}

void UiStatusBox::onDraw(c2d::Transform &transform, bool draw) {
    if (isVisible() && clock->getElapsedTime().asSeconds() > 5) {
        setVisibility(Visibility::Hidden, true);
    }
    SkinnedRectangle::onDraw(transform, draw);
}

UiStatusBox::~UiStatusBox() {
    delete (clock);
}
