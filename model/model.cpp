#include "model.h"

#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <list>
#include <regex>

std::string NormalizeAffixText(std::string text) {
  // 1. diapasons
  static const std::regex range_regex(
      R"(\+?\s*\(\s*[0-9.]+\s*[-–]\s*[0-9.]+\s*\)\s*(%?))");
  text = std::regex_replace(text, range_regex, "#$1");

  // 2. tags
  static const std::regex tag_regex(R"(\[(?:[^|\]]+\|)?([^\]]+)\])");
  text = std::regex_replace(text, tag_regex, "$1");

  // 3. numbers and percents
  static const std::regex percent_regex(R"(\+?\d+%)");
  text = std::regex_replace(text, percent_regex, "#%");
  // 4. integers
  static const std::regex number_regex(R"(\+?\d+(?!\.)\b)");
  text = std::regex_replace(text, number_regex, "#");

  // 5. spaces
  static const std::regex multi_spaces(R"(\s{2,})");
  text = std::regex_replace(text, multi_spaces, " ");

  return text;
}

void Model::LoadFiles() {
  std::ifstream f_affixes("affixes.json");
  if (f_affixes.is_open()) {
    try {
      json_affixes = nlohmann::json::parse(f_affixes);
    } catch (const nlohmann::json::parse_error& e) {
      std::string error_msg = "Error in parsing affixes.json:\n";
      error_msg += "What: " + std::string(e.what()) + "\n";
      error_msg += "Where: " + std::to_string(e.byte) + "\n";

      MessageBoxA(NULL, error_msg.c_str(), "Error JSON", MB_OK | MB_ICONERROR);
      exit(1);
    }
  } else {
    MessageBoxA(NULL, "affixes.json not found\n", "error file loading\n",
                MB_OK | MB_ICONERROR);
    exit(1);
  }
  f_affixes.close();
  std::ifstream f_weapons("weapons.json");
  if (f_weapons.is_open()) {
    try {
      json_weapons = nlohmann::json::parse(f_weapons);
    } catch (const nlohmann::json::parse_error& e) {
      std::string error_msg = "Error in parsing weapons.json:\n";
      error_msg += "What: " + std::string(e.what()) + "\n";
      error_msg += "Where: " + std::to_string(e.byte) + "\n";

      MessageBoxA(NULL, error_msg.c_str(), "Error JSON", MB_OK | MB_ICONERROR);
      exit(1);
    }
  } else {
    MessageBoxA(NULL, "weapons.json not found\n", "error file loading\n",
                MB_OK | MB_ICONERROR);
    exit(1);
  }
  f_weapons.close();
}

void Model::GetWeapons() {
  for (auto& [id, item] : json_weapons.items()) {
    if (!item.is_object()) continue;

    Gear object = {};
    std::string domain = item.value("domain", "");
    std::string release_state = item.value("release_state", "");
    std::string item_class = item.value("item_class", "");

    if (domain == "item" && release_state == "released" &&
        !item_class.empty()) {
      // small json protection from not existing data
      object.base_name = item.value("name", "Unnamed");
      std::transform(item_class.begin(), item_class.end(), item_class.begin(),
                     ::tolower);
      object.class_name = item_class;
      std::set<std::string> tags;
      if (!item.contains("tags")) {
        continue;
      }

      if (!item["tags"].is_array()) {
        continue;
      }

      bool skip_tag_required = false;

      for (auto& tag : item["tags"]) {
        if (!tag.is_string()) {
          continue;
        }

        if (tag.get<std::string>() == "not_for_sale") {
          skip_tag_required = true;
          break;
        }

        std::string tag_data = tag.get<std::string>();
        std::transform(tag_data.begin(), tag_data.end(), tag_data.begin(),
                       ::tolower);
        tags.insert(tag_data);
      }

      if (skip_tag_required) {
        continue;
      }
      std::string item_name = object.base_name;
      std::string item_class_name = object.class_name;

      object.tags = std::move(tags);
      size_t tags_count = object.tags.size();
      parsed_gear.push_back(std::move(object));
    }
  }
  json_weapons.clear();
}

void Model::GetAffixes() {
  for (auto& [id, item] : json_affixes.items()) {
    if (!item.is_object()) continue;
    if (item.value("domain", "") != "item") continue;
    if (item.value("is_essence_only", false)) continue;
    Affix object{};
    if (item.contains("generation_type") &&
        !item["generation_type"].is_null()) {
      object.generation_type = item["generation_type"].get<std::string>();
    }
    if (object.generation_type != "suffix" &&
        object.generation_type != "prefix") {
      continue;
    }
    if (item.contains("released") && !item["released"].is_null()) {
      std::string released = item["released"].get<std::string>();
      if (released != "released") continue;
    }

    if (item.contains("type") && !item["type"].is_null()) {
      object.affix_type = item["type"].get<std::string>();
    }
    if (item.contains("text") && !item["text"].is_null()) {
      object.affix_text = item["text"].get<std::string>();
    }

    if (item.contains("spawn_weights") && item["spawn_weights"].is_array()) {
      for (auto& w : item["spawn_weights"]) {
        if (!w.is_object()) continue;
        std::string tag = w.value("tag", "");
        int weight = w.value("weight", 0);
        if (!tag.empty()) {
          object.tags.emplace_back(tag, weight);
        }
      }
    }

    parsed_affixes.push_back(std::move(object));
  }
  json_affixes.clear();
}

std::vector<std::pair<std::string, std::string>> Model::SearchRequestedGear(
    const std::string& item_gear) {
  std::string item_gear_copy = item_gear;
  std::transform(item_gear_copy.begin(), item_gear_copy.end(),
                 item_gear_copy.begin(), ::tolower);

  if (item_gear_copy == curr_gear) {
    return cached_affix_names;
  }
  curr_gear_tags.clear();
  curr_gear_affixes.clear();
  cached_affix_names.clear();

  for (auto& gear : parsed_gear) {
    if (gear.class_name == item_gear_copy) {
      for (auto& tag : gear.tags) {
        curr_gear_tags.insert(tag);
      }
    }
  }

  for (const Affix& affix : parsed_affixes) {
    int final_weight = 0;
    bool match_found = false;

    for (const Tag& w : affix.tags) {
      if (curr_gear_tags.count(w.tag_name)) {
        final_weight = w.weight;
        match_found = true;
        break;
      }
    }
    if (match_found && final_weight > 0) {
      curr_gear_affixes.push_back(affix);
    }
  }

  for (auto& affix : curr_gear_affixes) {
    std::string text = NormalizeAffixText(affix.affix_text);

    bool already_exists = false;
    for (const auto& p : cached_affix_names) {
      if (p.first == text) {
        already_exists = true;
        break;
      }
    }
    if (!already_exists) {
      cached_affix_names.emplace_back(text, affix.generation_type);
    }
  }
  return cached_affix_names;
}

std::set<std::string> Model::GetClassNames() {
  std::set<std::string> gear_classes = {};
  for (auto& gear : parsed_gear) {
    gear_classes.insert(gear.class_name);
  }
  return gear_classes;
}

Model::Model() {
  LoadFiles();
  GetWeapons();
  GetAffixes();
}