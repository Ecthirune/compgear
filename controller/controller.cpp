#include "controller.h"

#include <algorithm>
#include <set>

static Model g_model;
static std::string g_selected_base;
static std::vector<std::string> g_selected_stats;

std::vector<std::string> Controller::GetItemBases() {
  return g_model.GetParsedItemData().base_types;
}

void Controller::SetItemBase(const std::string &type) {
  g_selected_base = type;
}

void Controller::ToggleStat(const std::string &stat, bool active) {
  auto it = std::find(g_selected_stats.begin(), g_selected_stats.end(), stat);
  if (active) {
    if (it == g_selected_stats.end()) g_selected_stats.push_back(stat);
  } else {
    if (it != g_selected_stats.end()) g_selected_stats.erase(it);
  }
}

bool Controller::NeedsAttributeSelector(const std::string &item_base) {
  return g_model.IsArmourBase(item_base);
}

std::vector<std::string> Controller::RefreshAffixes() {
  if (g_selected_base.empty()) return {};

  std::set<std::string> query =
      g_model.GetQueryTagsForBase(g_selected_base, g_selected_stats);

  return g_model.GetAffixesByTags(query);
}

void Controller::AddSelectedAffixToPreset(const std::string &display_text) {
  if (!g_selected_base.empty()) {
    g_model.AddAffixToPreset(g_selected_base, display_text);
  }
}

const std::vector<SelectedItem> &Controller::GetCurrentPresetContent() {
  return g_model.GetCurrentPresetItems();
}

std::vector<std::string> Controller::GetPresetList() {
  return g_model.GetAvailablePresets();
}
void Controller::LoadPreset(const std::string &name) {
  g_model.LoadPresetFromFile(name);
}
void Controller::NewPreset() { g_model.ClearCurrentPreset(); }
