#pragma once
#include <string>
#include <vector>

#include "../model/model.h"
class Controller {
 public:
  static std::vector<std::string> SetGearToSearch(const std::string& item_gear);
  static std::set<std::string> GetAllGearList();
};