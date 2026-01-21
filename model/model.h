#pragma once
#include <vector>
#include <string>
#include <set>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ItemData {
    std::vector<std::string> base_types;
    std::vector<std::string> condition_tags; 
};

class Model {
public:
    Model() { LoadData(); }
    void LoadData();

    // Прямой парсинг JSON для получения всех доступных баз и условий
    ItemData GetParsedItemData();
    
    // Поиск аффиксов по набору тегов (OR логика: если есть хоть один тег из сета)
    std::vector<std::string> GetAffixesByTags(const std::set<std::string>& search_tags);

    // Вспомогательные методы логики POE
    std::string BuildConditionTag(const std::vector<std::string>& stats);
    bool IsArmourBase(const std::string& base);

private:
    json cached_data;
};