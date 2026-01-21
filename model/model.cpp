#include "model.h"
#include <fstream>
#include <algorithm>
#include <list>

void Model::LoadData() {
    std::ifstream f("input.json");
    if (f.is_open()) {
        cached_data = json::parse(f);
    }
}

bool FoundProhibitedTag(const std::string& tag) {
    std::list<std::string> prohibited_tags = {"default", "armour", "amulet_elder", 
        "amulet_shaper", "axe", "dagger", "fishing_rod", "flail", "gloves_elder", 
        "gloves_shaper", "one_hand_weapon", "two_hand_weapon", "ranged", 
        "str_dex_shield", "str_int_shield", "str_shield", "sword", "trap", "warstaff",
    "weapon" };
   if (std::find(prohibited_tags.begin(), prohibited_tags.end(), tag) != prohibited_tags.end()) 
    return true;
    else
    return false;
   }


ItemData Model::GetParsedItemData() {
    ItemData data;
    std::set<std::string> bases;
    std::set<std::string> conditions;

    for (auto& [id, affix] : cached_data.items()) {
        // Условие: domain "item", тип "prefix" или "suffix"
        if (affix.value("domain", "") != "item") continue;
        std::string gen_type = affix.value("generation_type", "");
        if (gen_type != "prefix" && gen_type != "suffix") continue;

        if (affix.contains("spawn_weights") && affix["spawn_weights"].is_array()) {
            for (auto& sw : affix["spawn_weights"]) {
                std::string tag = sw.value("tag", "");
                if (sw.value("weight", 0) <= 0 || FoundProhibitedTag(tag)) continue;

                // Классификация: если есть _armour - это условие, иначе - база
                if ((tag.find("_armour") != std::string::npos) && !(tag.find("body_armour") != std::string::npos)) {
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

std::vector<std::string> Model::GetAffixesByTags(const std::set<std::string>& search_tags) {
    std::map<std::string, std::string> unique_results;

    for (auto& [id, affix] : cached_data.items()) {
        if (affix.value("domain", "") != "item") continue;

        // Проверяем только префиксы и суффиксы (как договаривались ранее)
        std::string gen_type = affix.value("generation_type", "");
        if (gen_type != "prefix" && gen_type != "suffix") continue;

        if (affix.contains("spawn_weights") && affix["spawn_weights"].is_array()) {
            for (auto& sw : affix["spawn_weights"]) {
                if (sw.value("weight", 0) > 0 && search_tags.count(sw.value("tag", ""))) {
                    
                    std::string group_key = affix.value("type", ""); // Логическая группа (напр. "Strength")
                    std::string display_text = affix.value("text", id); // Описание (напр. "+# to Strength")

                    if (!group_key.empty()) {
                        // Если такая группа еще не добавлена, добавляем её описание
                        if (unique_results.find(group_key) == unique_results.end()) {
                            unique_results[group_key] = display_text;
                        }
                    }
                    break; 
                }
            }
        }
    }

    // Собираем только описания (text) в финальный вектор
    std::vector<std::string> final_list;
    for (auto const& [key, text] : unique_results) {
        final_list.push_back(text);
    }

    // Сортируем алфавитно для удобства в GUI
    std::sort(final_list.begin(), final_list.end());
    
    return final_list;
}

std::string Model::BuildConditionTag(const std::vector<std::string>& stats) {
    if (stats.empty()) return "";
    
    std::string res = "";
    // Строгий алфавитный порядок POE для статов в тегах
    if (std::find(stats.begin(), stats.end(), "str") != stats.end()) res += "str_";
    if (std::find(stats.begin(), stats.end(), "dex") != stats.end()) res += "dex_";
    if (std::find(stats.begin(), stats.end(), "int") != stats.end()) res += "int_";
    
    return res + "armour";
}

bool Model::IsArmourBase(const std::string& base) {
    static const std::set<std::string> armour_bases = {
        "helmet", "body_armour", "boots", "gloves", "shield"
    };
    return armour_bases.count(base) > 0;
}