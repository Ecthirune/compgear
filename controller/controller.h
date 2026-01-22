#pragma once
#include <string>
#include <vector>

#include "../model/model.h"
class Controller {
 public:
  // Get clear bases
  static std::vector<std::string> GetItemBases();

  // Choice controls
  static void SetItemBase(const std::string &type);
  static void ToggleStat(const std::string &stat, bool active);

  // Does checkboxes required
  static bool NeedsAttributeSelector(const std::string &item_base);

  static std::vector<std::string> RefreshAffixes();

  // Preset controls
  static void AddSelectedAffixToPreset(const std::string &display_text);
  static const std::vector<SelectedItem> &GetCurrentPresetContent();
  static std::vector<std::string> GetPresetList();
  static void LoadPreset(const std::string &name);
  static void NewPreset();
};