#pragma once
#include <string>
#include <vector>

#include "../model/model.h"
class Controller {
 public:
 Controller()
 {
    Model m_model;
 }
  std::vector<std::pair<std::string, std::string>> SetGearToSearch(const std::string& item_gear);
  std::set<std::string> GetAllGearList();
  std::string& GetChosenClass() {
    return chosen_class;
  }
  std::vector<std::pair<std::string, std::string>>& GetAffixList() {
    return affix_list;
  }
  std::set<std::string>::iterator& GetSelectedIt() {
    return selected_it;
  }

  private:
  Model m_model;
  std::string chosen_class = {};
  std::vector<std::pair<std::string, std::string>> affix_list = {};
  std::set<std::string>::iterator selected_it = {};
};