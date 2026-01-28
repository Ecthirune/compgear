#pragma once
#include <string>
#include <vector>

#include "../model/model.h"

struct Preset_gear {
  std::string gear_name;
  std::vector<std::string> prefixes;
  std::vector<std::string> suffixes;
};

struct Preset {
  std::string preset_name;
  std::vector<Preset_gear> gears;
};

class Controller {
 public:
  // contst
  Controller() { Model m_model; }

  // setters
  std::vector<std::pair<std::string, std::string>> SetGearToSearch(
      const std::string& item_gear);
  void SetSelectedAffix(const std::pair<std::string, std::string> affix);
  void SetSelectedAffix();

  // getters
  std::set<std::string> GetAllGearList();
  std::string& GetChosenClass() { return chosen_class; }
  std::vector<std::pair<std::string, std::string>>& GetAffixList() {
    return affix_list;
  }
  std::set<std::string>::iterator& GetSelectedIt() { return selected_it; }
  std::pair<std::string, std::string> GetSelectedAffix();

  // preset controller
  bool affis_is_selected;

  bool IsCreatingPreset();
  void StartCreatingPreset();
  void FinishCreatingPreset();
  void NewPreset(std::string preset_name);
  void AddAffixToPreset(std::pair<std::string, std::string> affix, int weight);
  std::string GetPresetName();
  Preset& GetCurrentPreset();

 private:
  Model m_model;
  std::string chosen_class = {};
  std::vector<std::pair<std::string, std::string>> affix_list = {};
  std::set<std::string>::iterator selected_it = {};
  std::pair<std::string, std::string> selected_affix = {};
  Preset current_preset = {};
  bool menu_preset_creation = false;
  // std::vector<Preset> presets = {}; // todo: add save-load presets
};