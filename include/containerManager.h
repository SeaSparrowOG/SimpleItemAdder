#pragma once

namespace Container {
	enum QueryType {
		kWeapon,
		kArmor,
		kBook,
		kMisc,
		kIngredient,
		kAll
	};

	class Manager : public clib_util::singleton::ISingleton<Manager> {
	public:
		bool InitializeMaps();
		void ToggleSetting(std::string a_settingName);
		bool SearchItem(std::string a_name, QueryType a_type = kAll);
		void SetContainer(std::string a_id, std::string a_modName);

	private:
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> weaponMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> armorMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> bookMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> miscMap;
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> ingredientMap;
		RE::TESObjectREFR* container;

		//Filters
		bool showEnchants{ true };
		bool onlyUniqueEnchantments{ false };
		bool onlyEnchants{ false };
	};
}