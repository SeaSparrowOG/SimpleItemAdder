#pragma once

namespace Papyrus {
#define BIND(a_method, ...) a_vm.RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
#define BIND_EVENT(a_method, ...) a_vm.RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
#define STATIC_ARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, RE::StaticFunctionTag *

	using VM = RE::BSScript::Internal::VirtualMachine;
	using StackID = RE::VMStackID;
	inline constexpr auto script = "SEA_SimpleItemAdder"sv;
	
	enum QueryType {
		kWeapon,
		kArmor,
		kBook,
		kMisc,
		kIngredient,
		kAll
	};

	class Papyrus : public clib_util::singleton::ISingleton<Papyrus> {
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
	};

	bool RegisterFunctions(VM* a_vm);
}