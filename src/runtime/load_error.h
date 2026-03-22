#ifndef FCEUX_VITA_LOAD_ERROR_H
#define FCEUX_VITA_LOAD_ERROR_H

#include <string>

namespace pemu {
class UiMain;
struct Game;
}

namespace fceux_vita::load_error {

void clear();
void set(std::string title, std::string summary, std::string detail = {});
bool has();
std::string getTitle();
std::string getMessage();
void show(pemu::UiMain *ui, const pemu::Game *game = nullptr);

}

#endif
