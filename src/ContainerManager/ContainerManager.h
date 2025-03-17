#pragma once

#include "Filter.h"

namespace ContainerManager
{
    class ContainerManager : public Utilities::Singleton::ISingleton<ContainerManager>
    {
    public:
        bool InitializeMembers();

        int  SaveFilter(std::unique_ptr<Rule>&& a_newFilter);
        bool OpenContainer(std::vector<int>& a_filters);

    private:
        std::vector<StoredForm> validWeapons{};     // Weapons, Ammo, Staves
        std::vector<StoredForm> validArmor{};       // Light, Heavy, Clothes, Shields
        std::vector<StoredForm> validBooks{};       // Spell Tomes, Skill Books, Books, Notes, Scrolls
        std::vector<StoredForm> validIngestibles{}; // Potions, Poisons, Food, Ingredients
        std::vector<StoredForm> validMisc{};        // Miscellaneous and Soul Gems

        // Stored Filters (wiped after each use)
        std::vector<std::unique_ptr<Rule>> storedFilters{};

        // Pre-initialized Members
        // Survival Mode: Warmth
        RE::Setting* bootsWarm{ nullptr };
        RE::Setting* bootsCold{ nullptr };
        RE::Setting* bootsFallback{ nullptr };
        RE::Setting* armorWarm{ nullptr };
        RE::Setting* armorCold{ nullptr };
        RE::Setting* armorFallback{ nullptr };
        RE::Setting* glovesWarm{ nullptr };
        RE::Setting* glovesCold{ nullptr };
        RE::Setting* glovesFallback{ nullptr };
        RE::Setting* headWarm{ nullptr };
        RE::Setting* headCold{ nullptr };
        RE::Setting* headFallback{ nullptr };

        RE::BGSKeyword* warmGearKeyword{ nullptr };
        RE::BGSKeyword* coldGearKeyword{ nullptr };
    };
}