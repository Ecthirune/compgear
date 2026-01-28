#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

struct Gear {
  std::string base_name;
  std::string class_name;
  std::set<std::string> tags;
};

struct Tag {
  std::string tag_name;
  int weight;
  Tag(std::string n, int w) : tag_name(std::move(n)), weight(w) {}
};

struct Affix {
  std::string affix_type;
  std::string affix_text;
  std::string generation_type;
  std::vector<Tag> tags;
};
class Model {
 public:
  Model();
  void LoadFiles();
  void GetWeapons();
  void GetAffixes();
  std::vector<std::pair<std::string, std::string>> SearchRequestedGear(
      const std::string& item_gear);
  std::set<std::string> GetClassNames();

 private:
  nlohmann::json json_affixes;
  nlohmann::json json_weapons;
  std::vector<Gear> parsed_gear;
  std::vector<Affix> parsed_affixes;

  std::string curr_gear;
  std::set<std::string> curr_gear_tags;
  std::vector<Affix> curr_gear_affixes;
  std::vector<std::pair<std::string, std::string>> cached_affix_names;
};