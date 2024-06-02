#pragma once

namespace Container {
	enum QueryType {
		kWeapon,
		kArmor,
		kBook,
		kMisc,
		kIngredient,
		kConsumable,
		kAll
	};

	class Manager : public clib_util::singleton::ISingleton<Manager> {
	public:
		void DisplayPage(uint64_t a_page);
		bool InitializeMaps();
		void ToggleSetting(std::string a_settingName);
		bool SearchItem(std::string a_name, QueryType a_type = kAll);
		void SetContainer(std::string a_id, std::string a_modName);
		void SetCount(int32_t a_newCount);
		void SetMaxContainerItems(size_t a_num);
		void SetQuest(std::string a_id, std::string a_modName);

	private:
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> weaponMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> armorMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> bookMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> miscMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> ingredientMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> consumableMap;
		RE::TESQuest* quest;
		RE::TESObjectREFR* container;

		//Filters
		bool showEnchants{ true };
		bool onlyUniqueEnchantments{ false };
		bool onlyEnchants{ false };
		bool onlySpellbooks{ false };
		bool onlyFood{ false };
		bool onlyPotions{ false };
		bool onlyPoisons{ false };

		//Search results
		std::vector<RE::TESBoundObject*> vectorResult;
		size_t displayPage{ 0 };
		size_t maxResults{ 250 };
		int32_t count{ 1 };
	};
}