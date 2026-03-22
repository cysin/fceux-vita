//
// Shared RGUI theme colors for fceux-vita
//

#ifndef FCEUX_VITA_RGUI_THEME_H
#define FCEUX_VITA_RGUI_THEME_H

#include "cross2d/c2d.h"

namespace fceux_vita::rgui_theme {
    static const c2d::Color Overlay(0, 0, 0, 200);
    static const c2d::Color Background(18, 18, 18, 255);
    static const c2d::Color TitleBar(34, 34, 34, 255);
    static const c2d::Color TitleText(245, 245, 245, 255);
    static const c2d::Color ItemText(228, 228, 228, 255);
    static const c2d::Color ValueText(196, 196, 196, 255);
    static const c2d::Color Highlight(92, 92, 92, 255);
    static const c2d::Color Accent(216, 216, 216, 255);
    static const c2d::Color Shadow(0, 0, 0, 160);
}

#endif // FCEUX_VITA_RGUI_THEME_H
