#include "model.h"

#include <algorithm>
#include <fstream>
#include <list>
#include <filesystem>


void Model::LoadData() {
  std::ifstream f("input.json");
  if (f.is_open()) {
    cached_data = json::parse(f);
  }
}

bool FoundProhibitedTag(const std::string& tag) {
  std::list<std::string> prohibited_tags = {
      "default",      "armour",         "amulet_elder",    "amulet_shaper",
      "axe",          "dagger",         "fishing_rod",     "flail",
      "gloves_elder", "gloves_shaper",  "one_hand_weapon", "two_hand_weapon",
      "ranged",       "str_dex_shield", "str_int_shield",  "str_shield",
      "sword",        "trap",           "warstaff",        "weapon"};
  if (std::find(prohibited_tags.begin(), prohibited_tags.end(), tag) !=
      prohibited_tags.end())
    return true;
  else
    return false;
}

ItemData Model::GetParsedItemData() {
  ItemData data;
  std::set<std::string> bases;
  std::set<std::string> conditions;

  for (auto& [id, affix] : cached_data.items()) {
    if (affix.value("domain", "") != "item") continue;
    std::string gen_type = affix.value("generation_type", "");
    if (gen_type != "prefix" && gen_type != "suffix") continue;

    if (affix.contains("spawn_weights") && affix["spawn_weights"].is_array()) {
      for (auto& sw : affix["spawn_weights"]) {
        std::string tag = sw.value("tag", "");
        if (sw.value("weight", 0) <= 0 || FoundProhibitedTag(tag)) continue;

        // if "_armour" found -> it's condition
        if ((tag.find("_armour") != std::string::npos) &&
            !(tag.find("body_armour") != std::string::npos)) {
          conditions.insert(tag);
        } else {
          bases.insert(tag);
        }
      }
    }
  }
  data.base_types.assign(bases.begin(), bases.end());
  data.condition_tags.assign(conditions.begin(), conditions.end());
  return data;
}

std::vector<std::string> Model::GetAffixesByTags(
    const std::set<std::string>& search_tags) {
  std::map<std::string, std::string> unique_results;

  for (auto& [id, affix] : cached_data.items()) {
    if (affix.value("domain", "") != "item" && affix.value("domain", "") != "desecrated" && affix.value("domain", "") != "misc") continue;

    // checking suffix/prefix
    std::string gen_type = affix.value("generation_type", "");
    if (gen_type != "prefix" && gen_type != "suffix") continue;

    if (affix.contains("spawn_weights") && affix["spawn_weights"].is_array()) {
      for (auto& sw : affix["spawn_weights"]) {
        if (sw.value("weight", 0) > 0 &&
            search_tags.count(sw.value("tag", ""))) {
          std::string group_key = affix.value(
              "type",
              "");  // Logical group (strength, dexterity, resistance etc)
          std::string display_text = affix.value("text", id);  // Entire text

          if (!group_key.empty()) {
            // Add if not already exists
            if (unique_results.find(group_key) == unique_results.end()) {
              unique_results[group_key] = display_text;
            }
          }
          break;
        }
      }
    }
  }

  // Collecting texts
  std::vector<std::string> final_list;
  for (auto const& [key, text] : unique_results) {
    final_list.push_back(text);
  }

  std::sort(final_list.begin(), final_list.end());

  return final_list;
}

std::string Model::BuildConditionTag(const std::vector<std::string>& stats) {
  if (stats.empty()) return "";

  std::string res = "";
  if (std::find(stats.begin(), stats.end(), "str") != stats.end())
    res += "str_";
  if (std::find(stats.begin(), stats.end(), "dex") != stats.end())
    res += "dex_";
  if (std::find(stats.begin(), stats.end(), "int") != stats.end())
    res += "int_";

  return res + "armour";
}

bool Model::IsArmourBase(const std::string& base) {
  static const std::set<std::string> armour_bases = {
      "helmet", "body_armour", "boots", "gloves", "shield"};
  return armour_bases.count(base) > 0;
}

void Model::AddAffixToPreset(const std::string& tag,
                             const std::string& affix_text) {
  // If preset exists
  auto it = std::find_if(
      current_preset_items.begin(), current_preset_items.end(),
      [&](const SelectedItem& item) { return item.tag_name == tag; });

  if (it != current_preset_items.end()) {
    // Check for duplicates inside tag's affixes
    if (std::find(it->affixes.begin(), it->affixes.end(), affix_text) ==
        it->affixes.end()) {
      it->affixes.push_back(affix_text);
    }
  } else {
    // New tag entrance
    SelectedItem newItem;
    newItem.tag_name = tag;
    newItem.affixes.push_back(affix_text);
    current_preset_items.push_back(newItem);
  }
}

std::vector<std::string> Model::GetAvailablePresets() {
    std::vector<std::string> presets;
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().extension() == ".preset") { // Используем свое расширение
            presets.push_back(entry.path().filename().string());
        }
    }
    return presets;
}

void Model::LoadPresetFromFile(const std::string& filename) {
    std::ifstream f(filename);
    if (f.is_open()) {
        json j = json::parse(f);
        current_preset_items.clear();
        for (auto& item : j) {
            SelectedItem si;
            si.tag_name = item["tag"];
            si.affixes = item["affixes"].get<std::vector<std::string>>();
            current_preset_items.push_back(si);
        }
    }
}

void Model::CreateNewPreset() {
    current_preset_items.clear();
}

void Model::ClearCurrentPreset() {
    current_preset_items.clear();
}

std::set<std::string> Model::GetQueryTagsForBase(const std::string& base, const std::vector<std::string>& manual_stats) {
    std::set<std::string> tags;
    tags.insert(base);

    // Логика для фокусов (скрытая настройка модели)
    if (base == "focus") {
        tags.insert("armour");
        tags.insert("int_armour");
    }
    // Логика для брони (явная настройка через статы)
    else if (IsArmourBase(base)) {
        tags.insert("armour");
        std::string condition = BuildConditionTag(manual_stats);
        if (!condition.empty()) {
            tags.insert(condition);
        }
    }
    
    return tags;
}