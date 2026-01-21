#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

using json = nlohmann::json;

struct ItemData {
  std::vector<std::string> base_types;
  std::vector<std::string> condition_tags;
};
struct SelectedItem {
  std::string tag_name;
  std::vector<std::string> affixes;
};

struct Preset {
  std::string name;
  std::vector<SelectedItem> items;
};

class Model {
 public:
  Model() { LoadData(); }
  void LoadData();

  // Straight json parsing and sorting
  ItemData GetParsedItemData();
  std::vector<std::string> GetAffixesByTags(
      const std::set<std::string>& search_tags);

  // Small convertations for armour/shield tag
  std::string BuildConditionTag(const std::vector<std::string>& stats);
  bool IsArmourBase(const std::string& base);

  // preset controls
  void AddAffixToPreset(const std::string& tag, const std::string& affix_text);
  const std::vector<SelectedItem>& GetCurrentPresetItems() const {
    return current_preset_items;
  }
  void SavePreset(const std::string& name);
  std::vector<std::string> GetAvailablePresets();
  void LoadPresetFromFile(const std::string& filename);
  void CreateNewPreset();
  void ClearCurrentPreset();
  std::set<std::string> GetQueryTagsForBase(const std::string& base, const std::vector<std::string>& manual_stats);

  
 private:
  json cached_data;
  std::vector<SelectedItem> current_preset_items;
  std::vector<Preset> saved_presets;
};