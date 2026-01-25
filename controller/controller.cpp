#include "controller.h"

#include <windows.h>

#include <algorithm>
#include <set>

static Model g_model;

std::vector<std::string> Controller::SetGearToSearch(
    const std::string& item_gear) {
  return g_model.SearchRequestedGear(item_gear);
}

std::set<std::string> Controller::GetAllGearList() {
  return g_model.GetClassNames();
}
