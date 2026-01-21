#pragma once
#include <string>
#include <vector>

class Controller {
public:
    // Получение списка баз (очищенного от условий)
    static std::vector<std::string> GetItemBases();
    
    // Управление состоянием выбора
    static void SetItemBase(const std::string& type);
    static void ToggleStat(const std::string& stat, bool active);
    
    // Проверка необходимости рисовать чекбоксы статов
    static bool NeedsAttributeSelector(const std::string& item_base);

    // Финальный поиск аффиксов по выбранным критериям
    static std::vector<std::string> RefreshAffixes();
};