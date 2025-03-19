#pragma once

#include "Filter.h"

namespace ContainerManager
{
constexpr auto BYTE_TO_MEGABYTE = 1048576.0f;;

    class ContainerManager : public Utilities::Singleton::ISingleton<ContainerManager>
    {
    public:
        bool InitializeMembers(); 
        
        template <typename T>
        void StoreFormInArray(T* a_form, std::vector<StoredForm>& a_storage) {
            if (!a_form || !a_form->GetPlayable()) {
                return;
            }

            try {
                StoredForm createdForm = StoredForm(a_form);
                a_storage.push_back(std::move(createdForm));
            }
            catch (std::invalid_argument& e) {
                logger::warn("{}", e.what());
            }
            catch (std::exception& e) {
                logger::warn("{}", e.what());
            }
        }

        int AddRule(std::unique_ptr<Rule>&& a_rule);

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