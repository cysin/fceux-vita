#ifndef FCEUX_VITA_GAME_TYPES_H
#define FCEUX_VITA_GAME_TYPES_H

#include <string>

namespace pemu {
    struct System {
        System() = default;

        System(int id, int parentId, const std::string &name) {
            this->id = id;
            this->parentId = parentId;
            this->name = name;
        }

        std::string name = "UNKNOWN";
        int id = 0;
        int parentId = 0;
    };

    struct Game {
        std::string name;
        std::string path;
        std::string romsPath;
        System system;
    };
}

#endif
