#include "papyrus.h"

namespace {
	bool IsHex(std::string const& s) {
		return s.compare(0, 2, "0x") == 0
			&& s.size() > 2
			&& s.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
	}

	RE::FormID StringToFormID(std::string a_str) {
		RE::FormID result{ 0 };
		if (!IsHex(a_str)) return result;
		std::istringstream ss{ a_str };
		ss >> std::hex >> result;
		return result;
	}

	RE::TESForm* GetFormFromMod(std::string a_id, std::string a_mod) {
		RE::TESForm* response = nullptr;
		if (!RE::TESDataHandler::GetSingleton()->LookupModByName(a_mod)) return response;
		if (!IsHex(a_id)) return response;

		RE::FormID formID = StringToFormID(a_id);
		response = RE::TESDataHandler::GetSingleton()->LookupForm(formID, a_mod);
		return response;
	}

	bool ShouldRebuildINI(CSimpleIniA* a_ini) {
		const char* section = "General";
		const char* keys[] = { "iContainerID", "sModName" };
		int sectionLength = sizeof(keys) / sizeof(keys[0]);
		std::list<CSimpleIniA::Entry> keyHolder;

		a_ini->GetAllKeys(section, keyHolder);
		if (std::size(keyHolder) != sectionLength) return true;
		for (auto* key : keys) {
			if (!a_ini->KeyExists(section, key)) return true;
		}
		return false;
	}

	bool InitializeINI() {
		std::filesystem::path f{ "./Data/SKSE/Plugins/SimpleItemAdder.ini" };
		bool createEntries = false;
		if (!std::filesystem::exists(f)) {
			std::fstream createdINI;
			createdINI.open(f, std::ios::out);
			createdINI.close();
			createEntries = true;
		}

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(f.c_str());
		if (!createEntries) { createEntries = ShouldRebuildINI(&ini); }

		if (createEntries) {
			ini.Delete("General", NULL);
			ini.SetValue("General", "iContainerID", "0x802", ";If a reference does not have a location specified, search this distance for a marker with a location to substitute.");
			ini.SetValue("General", "sModName", "SimpleItemAdder.esp", ";If a reference does not have a location specified, search this distance for a marker with a location to substitute.");
			ini.SaveFile(f.c_str());
		}

		const char* id = ini.GetValue("General", "iContainerID", "0x802");
		const char* name = ini.GetValue("General", "sModName", "SimpleItemAdder.esp");

		Papyrus::Papyrus::GetSingleton()->SetContainer(id, name);
		return true;
	}

	template <typename T>
	void FillMap(std::unordered_map<std::string, std::vector<RE::TESBoundObject*>>* currentMap) {
		auto& itemArray = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();
		uint32_t successes = 0;
		uint32_t failures = 0;

		for (T* obj : itemArray) {
			auto* boundObject = static_cast<RE::TESBoundObject*>(obj);
			if (!boundObject) {
				continue;
			}
			const char* objName = obj->GetName();
			if (objName[0] == '\0') {
				continue;
			}

			auto formattedName = clib_util::string::tolower(objName);
			if ((*currentMap).contains(formattedName)) {
				(*currentMap)[formattedName].push_back(boundObject);
			}
			else {
				std::vector<RE::TESBoundObject*> boundArray{ boundObject };
				(*currentMap).try_emplace(formattedName, boundArray);
			}
		}
	}
}

namespace Papyrus {
	bool Papyrus::Papyrus::InitializeMaps() {
		FillMap<RE::TESObjectWEAP>(&this->weaponMap);
		FillMap<RE::TESObjectARMO>(&this->armorMap);
		_loggerInfo("Loaded weapons, armors, books, and ingredients.");
		InitializeINI();
		return true;
	}

	bool Papyrus::SearchItem(std::string a_name, QueryType a_type) {
		std::unordered_map<std::string, std::vector<RE::TESBoundObject*>> target;

		switch (a_type) {
		case kWeapon:
			target = this->weaponMap;
			break;
		case kArmor:
			target = this->armorMap;
			break;
		default:
			break;
		}

		if (!this->container) {
			_loggerInfo("Missing container");
			return false;
		}

		std::vector<RE::TESBoundObject*> vectorResult{};
		this->container->ResetInventory(false);
		for (auto& item : target) {
			auto lowerName = clib_util::string::tolower(item.first);
			if (!lowerName.contains(a_name)) continue;
			for (auto* obj : item.second) {
				vectorResult.push_back(obj);
				this->container->AddObjectToContainer(obj, nullptr, 1, nullptr);
			}
		}
		if (vectorResult.empty()) {
			return false;
		}

		this->container->ActivateRef(RE::PlayerCharacter::GetSingleton()->AsReference(), 0, nullptr, 0, false);
		return true;
	}

	void Papyrus::SetContainer(std::string a_id, std::string a_modName) {
		auto* form = GetFormFromMod(a_id, a_modName);
		auto* reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
		auto* containerBaseForm = reference ? reference->GetBaseObject() ? reference->GetBaseObject()->As<RE::TESObjectCONT>()  : nullptr : nullptr;
		if (!containerBaseForm) return;
		_loggerInfo("Successfully validated container {}~{}", a_id, a_modName);
		this->container = reference;
	}

	bool SearchItem(STATIC_ARGS, std::string a_name, std::string a_type) {
		if (a_type.empty() || a_name.empty()) return false;

		a_name = clib_util::string::tolower(a_name);
		a_type = clib_util::string::tolower(a_type);

		QueryType query = kAll;
		if (a_type == "armo") {
			query = kArmor;
		}
		else if (a_type == "weap") {
			query = kWeapon;
		}
		if (query == kAll) return false;

		return Papyrus::GetSingleton()->SearchItem(a_name, query);
	}

	void Bind(VM& a_vm) {
		BIND(SearchItem);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}