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

void Controller::SetSelectedAffix(
    const std::pair<std::string, std::string> affix) {
  selected_affix = affix;
}
void Controller::SetSelectedAffix() {
  { selected_affix = {}; }
}
std::pair<std::string, std::string> Controller::GetSelectedAffix() {
  return selected_affix;
}

void Controller::NewPreset(std::string preset_name) {
  chosen_class.clear();
  selected_affix = {};
  current_preset.preset_name.clear();
  current_preset.gears.clear();
  current_preset.preset_name = preset_name;
}

bool Controller::IsCreatingPreset() { return menu_preset_creation; }

void Controller::StartCreatingPreset() { menu_preset_creation = true; }

void Controller::FinishCreatingPreset() { menu_preset_creation = false; }

std::string Controller::GetPresetName() { return current_preset.preset_name; }

void Controller::AddAffixToPreset(
    const std::pair<std::string, std::string> affix, int weight) {
  auto it = std::find_if(
      current_preset.gears.begin(), current_preset.gears.end(),
      [&](const Preset_gear& gear) { return gear.gear_name == chosen_class; });

  if (it == current_preset.gears.end()) {
    current_preset.gears.push_back({chosen_class});
    if (affix.second == "prefix") {
      current_preset.gears.back().prefixes.push_back(affix.first);
    } else {
      current_preset.gears.back().suffixes.push_back(affix.first);
    }
  } else

  {
    if (affix.second == "prefix") {
      it->prefixes.push_back(affix.first);
    } else {
      it->suffixes.push_back(affix.first);
    }
  }
}

Preset& Controller::GetCurrentPreset() { return current_preset; }