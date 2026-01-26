#include "controller.h"

#include <windows.h>

#include <algorithm>
#include <set>


std::vector<std::pair<std::string, std::string>> Controller::SetGearToSearch(
    const std::string& item_gear) {
  return m_model.SearchRequestedGear(item_gear);
}

std::set<std::string> Controller::GetAllGearList() {
  return m_model.GetClassNames();
}
