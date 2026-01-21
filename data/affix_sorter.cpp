#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <windows.h>
#include "affix_sorter.h"

using json = nlohmann::json;

struct AffixInfo {
    std::string generation_type;
    std::set<std::string> spawn_tags;
};

// Исправленная логика расширения: добавляем только общие категории
void expand_armour_tags(const std::string& tag, std::set<std::string>& out_tags) {
    out_tags.insert(tag);

    // 1. Оружие
    if (tag == "one_hand_weapon" || tag == "two_hand_weapon") {
        out_tags.insert("weapon");
    }

    // 2. Броня и экипировка
    // Добавляем тэг "armour", если это конкретный вид брони ИЛИ если в тэге есть подстрока "armour"
    if (tag == "body_armour" || tag == "boots" || tag == "gloves" || tag == "helmet" || tag == "belt" || 
        tag.find("armour") != std::string::npos) {
        out_tags.insert("armour");
    }
}

std::string normalize_tag(const std::string& tag) {
    if (tag == "shields") return "shield";
    return tag;
}

bool create_data_files(std::string input_path) {
    std::ifstream input_file(input_path);
    if (!input_file.is_open()) return false;

    json data;
    try {
        input_file >> data;
    } catch (json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return false;
    }

    std::map<std::string, AffixInfo> unique_affixes;
    std::set<std::string> final_all_tags; // Общий список для tags.txt

    for (auto& [key, affix] : data.items()) {
        std::string domain = affix.value("domain", "");
        if (domain != "item" && domain != "misc") continue;

        std::string gen_type = affix.value("generation_type", "");
        if (gen_type != "prefix" && gen_type != "suffix") continue;

        std::string affix_type = affix.value("type", "");
        if (affix_type.empty()) continue;

        std::set<std::string> current_spawn_tags;
        if (affix.contains("spawn_weights") && affix["spawn_weights"].is_array()) {
            for (auto& sw : affix["spawn_weights"]) {
                std::string tag = sw.value("tag", "");
                int weight = sw.value("weight", 0);

                if (!tag.empty() && tag != "default" && weight > 0) {
                    current_spawn_tags.insert(normalize_tag(tag));
                }
            }
        }

        // Сохраняем/объединяем только ОРИГИНАЛЬНЫЕ теги из JSON
        if (unique_affixes.find(affix_type) == unique_affixes.end()) {
            unique_affixes[affix_type] = {gen_type, current_spawn_tags};
        } else {
            unique_affixes[affix_type].spawn_tags.insert(
                current_spawn_tags.begin(), 
                current_spawn_tags.end()
            );
        }
    }

    // === Запись 1: affixes.txt ===
    std::ofstream out_affixes("affixes.txt");
    for (auto const& [type, info] : unique_affixes) {
        out_affixes << type << " | " << info.generation_type << " | ";
        
        std::set<std::string> expanded_row_tags;
        for (const auto& original_tag : info.spawn_tags) {
            expand_armour_tags(original_tag, expanded_row_tags);
        }

        bool first = true;
        for (const auto& tag : expanded_row_tags) {
            if (!first) out_affixes << ",";
            out_affixes << tag;
            first = false;
            final_all_tags.insert(tag); // Собираем все теги для tags.txt
        }
        out_affixes << "\n";
    }

    // === Запись 2: tags.txt ===
    std::ofstream out_tags("tags.txt");
    for (const auto& tag : final_all_tags) {
        out_tags << tag << "\n";
    }

    return true;
}