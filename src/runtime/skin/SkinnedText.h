//
// Created by cpasjuste on 13/12/2021.
//

#ifndef PEMU_SKINNEDTEXT_H
#define PEMU_SKINNEDTEXT_H

#include <string>
#include <vector>

#include "cross2d/c2d.h"

namespace pemu {
    class UiMain;
    class Skin;
}

class SkinnedText : public c2d::Text {
public:
    explicit SkinnedText(pemu::UiMain *main, const std::vector<std::string> &cfgTree);

    bool load();

private:
    std::vector<std::string> mTree;
    pemu::UiMain *pMain;
    pemu::Skin *pSkin;
    c2d::Io *pIo;
};

#endif //PEMU_SKINNEDTEXT_H
