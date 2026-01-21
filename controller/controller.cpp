#include "controller.h"
#include "../model/model.h"
#include <set>
#include <algorithm>

static Model g_model;
static std::string g_selected_base;
static std::vector<std::string> g_selected_stats;

std::vector<std::string> Controller::GetItemBases() {
    return g_model.GetParsedItemData().base_types;
}

void Controller::SetItemBase(const std::string& type) {
    g_selected_base = type;
}

void Controller::ToggleStat(const std::string& stat, bool active) {
    auto it = std::find(g_selected_stats.begin(), g_selected_stats.end(), stat);
    if (active) {
        if (it == g_selected_stats.end()) g_selected_stats.push_back(stat);
    } else {
        if (it != g_selected_stats.end()) g_selected_stats.erase(it);
    }
}

bool Controller::NeedsAttributeSelector(const std::string& item_base) {
    return g_model.IsArmourBase(item_base);
}

std::vector<std::string> Controller::RefreshAffixes() {
    if (g_selected_base.empty()) return {};

    std::set<std::string> query;
    query.insert(g_selected_base); 

    if (g_model.IsArmourBase(g_selected_base)) {
        query.insert("armour");

        if (!g_selected_stats.empty()) {
            std::string condition = g_model.BuildConditionTag(g_selected_stats);
            query.insert(condition);
        }
    }

    return g_model.GetAffixesByTags(query);
}