#include "containerManager.h"

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
		const char* keys[] = { "iContainerID", "iQuestID", "sModName", "iMaxDisplayItems" };
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
			_loggerInfo("Created INI file because it was missing. Values will be created shortly.");
		}

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(f.c_str());
		if (!createEntries) { createEntries = ShouldRebuildINI(&ini); }

		if (createEntries) {
			ini.Delete("General", NULL);
			ini.SetValue("General", "iContainerID", "0x802", ";The ID of the container to drop the items in.");
			ini.SetValue("General", "iQuestID", "0xD67", ";The ID of the quest that handles event dispatching.");
			ini.SetLongValue("General", "iMaxDisplayItems", 250, ";The maximum number of results to display. Must be between 50 and 1000.");
			ini.SetValue("General", "sModName", "SimpleItemAdder.esp", ";The name of the mod with the above forms.");
			ini.SaveFile(f.c_str());
			_loggerInfo("Set values for INI file because one or more were invalid or missing.");
		}

		const char* id = ini.GetValue("General", "iContainerID", "0x802");
		const char* questID = ini.GetValue("General", "iQuestID", "0xD67");
		auto maxResults = ini.GetLongValue("General", "iMaxDisplayItems", 250);
		const char* name = ini.GetValue("General", "sModName", "SimpleItemAdder.esp");

		Container::Manager::GetSingleton()->SetContainer(id, name);
		Container::Manager::GetSingleton()->SetQuest(questID, name);
		Container::Manager::GetSingleton()->SetMaxContainerItems(maxResults);
		return true;
	}

	auto VectorContainsName(std::string a_string, std::vector<std::pair<std::string, std::vector<RE::TESBoundObject*>>>* a_vec) {
		auto response = a_vec->end();
		for (auto it = a_vec->begin(); it != a_vec->end(); ++it) {
			if ((*it).first == a_string) return it;
		}
		return response;
	}

	template <typename T>
	void FillMap(std::vector<std::pair<std::string, std::vector<RE::TESBoundObject*>>>* currentMap) {
		auto& itemArray = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();

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
			auto it = VectorContainsName(formattedName, currentMap);
			if (it != currentMap->end()) {
				(*it).second.push_back(obj);
			}
			else {
				std::vector<RE::TESBoundObject*> boundArray{ boundObject };
				auto newPair = std::make_pair(formattedName, boundArray);
				currentMap->push_back(newPair);
			}
		}

		std::sort(currentMap->begin(), currentMap->end(), [&](std::pair<std::string, std::vector<RE::TESBoundObject*>>& a, std::pair<std::string, std::vector<RE::TESBoundObject*>>& b) {
		return a.first < b.first;
		});
	}
}

namespace Container {
	bool Manager::InitializeMaps() {
		FillMap<RE::TESAmmo>(&this->weaponMap);
		FillMap<RE::TESObjectWEAP>(&this->weaponMap);
		FillMap<RE::TESObjectARMO>(&this->armorMap);
		FillMap<RE::TESObjectBOOK>(&this->bookMap);
		FillMap<RE::IngredientItem>(&this->ingredientMap);
		FillMap<RE::TESObjectMISC>(&this->miscMap);
		FillMap<RE::AlchemyItem>(&this->consumableMap);
		_loggerInfo("Loaded forms.");
		InitializeINI();
		return true;
	}

	void Manager::ToggleSetting(std::string a_settingName) {
		a_settingName = clib_util::string::tolower(a_settingName);

		if (a_settingName == "unique") {
			this->onlyUniqueEnchantments = !this->onlyUniqueEnchantments;
		}
		else if (a_settingName == "enchanted") {
			this->showEnchants = !this->showEnchants;
		}
		else if (a_settingName == "onlyenchanted") {
			this->onlyEnchants = true;
		}
		else if (a_settingName == "onlyspelltomes") {
			this->onlySpellbooks = !this->onlySpellbooks;
		}
		else if (a_settingName == "onlypotions") {
			this->onlyPotions = !this->onlyPotions;
		}
		else if (a_settingName == "onlypoisons") {
			this->onlyPoisons = !this->onlyPoisons;
		}
		else if (a_settingName == "onlyfood") {
			this->onlyFood = !this->onlyFood;
		}
		else if (a_settingName == "reset") {
			this->onlyUniqueEnchantments = false;
			this->onlyEnchants = false;
			this->onlySpellbooks = false;
			this->showEnchants = true;
			this->onlyFood = false;
			this->onlyPoisons = false;
			this->onlyPotions = false;
		}
	}

	bool Manager::SearchItem(std::string a_name, QueryType a_type) {
		if (!this->quest || !this->container) return false;

		this->vectorResult.clear();
		std::vector<std::pair<std::string, std::vector<RE::TESBoundObject*>>>* target = nullptr;
		switch (a_type) {
		case kWeapon:
			target = &this->weaponMap;
			break;
		case kArmor:
			target = &this->armorMap;
			break;
		case kBook:
			target = &this->bookMap;
			break;
		case kIngredient:
			target = &this->ingredientMap;
			break;
		case kMisc:
			target = &this->miscMap;
			break;
		case kConsumable:
			target = &this->consumableMap;
			break;
		default:
			return false;
		}
		
		//VM Check. If we can't find the manager script, abort.
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* handlePolicy = vm->GetObjectHandlePolicy();
		if (!handlePolicy) return false;
		RE::VMHandle handle = handlePolicy->GetHandleForObject(this->quest->GetFormType(), quest);
		if (!handle) return false;

		//Add the name, type, and container result to the script.
		bool succeeded = false;
		for (auto& foundScript : vm->attachedScripts.find(handle)->second) {
			if (!foundScript) continue;
			if (!foundScript->GetTypeInfo()) continue;
			if (foundScript->GetTypeInfo()->GetName() != "SAM_ManagerQuestScript"sv) continue;

			auto* searchNameVar = foundScript->GetProperty("Name");
			auto* searchTypeVar = foundScript->GetProperty("Type");
			auto* searchContVar = foundScript->GetProperty("FrameworkContainer");
			if (!searchNameVar || !searchTypeVar || !searchContVar) continue;

			RE::BSScript::PackValue(searchContVar, this->container);
			RE::BSScript::PackValue(searchNameVar, a_name);
			RE::BSScript::PackValue(searchTypeVar, a_type);

			succeeded = true;
			break;
		}
		if (!succeeded) return false;

		//Note: Sometimes, result is too large. Putthing THOUSANDS of objects in a container is very slow.
		std::vector<RE::EnchantmentItem*> foundEnchantments{};
		this->container->ResetInventory(false);

		for (auto& item : *target) {
			auto lowerName = clib_util::string::tolower(item.first);
			if (!lowerName.contains(a_name) && !a_name.empty()) continue;
			for (auto* obj : item.second) {
				
				if (a_type == kWeapon || a_type == kArmor) {
					auto* weapForm = obj->As<RE::TESObjectWEAP>();
					auto* armorForm = obj->As<RE::TESObjectARMO>();

					bool playable = weapForm ? weapForm->GetPlayable() : true;
					playable = playable && armorForm ? armorForm->GetPlayable() : true;
					if (!playable) continue;

					auto* enchantment = armorForm ? armorForm->formEnchanting : weapForm ? weapForm->formEnchanting : nullptr;
					if (enchantment) {
						auto* baseEnchant = enchantment->data.baseEnchantment;
						if (baseEnchant) {
							enchantment = baseEnchant;
						}
					}
					if (!enchantment && this->onlyEnchants) continue;
					if (enchantment && !this->showEnchants) continue;
					if (enchantment && this->onlyUniqueEnchantments) {
						if (std::find(foundEnchantments.begin(), foundEnchantments.end(), enchantment) != foundEnchantments.end()) {
							continue;
						}
						foundEnchantments.push_back(enchantment);
					}
				}
				
				if (a_type == kBook) {
					if (this->onlySpellbooks && !obj->As<RE::TESObjectBOOK>()->TeachesSpell()) {
						continue;
					}
				}

				if (a_type == kConsumable) {
					auto* consumable = obj->As<RE::AlchemyItem>();
					bool isPoison = consumable->IsPoison();
					bool isPotion = !consumable->IsFood() && !isPoison;
					bool isFood = !(isPotion && isPoison);

					if (this->onlyFood && !isFood) continue;
					if (this->onlyPoisons && !isPoison) continue;
					if (this->onlyPotions && !isPotion) continue;
				}

				this->vectorResult.push_back(obj);
			}
		}
		
		if (this->vectorResult.empty()) {
			return false;
		}

		size_t vectorStart = 0;
		size_t vectorEnd = vectorResult.size() - 1;
		if (this->vectorResult.size() > this->maxResults) {
			vectorEnd = vectorStart + this->maxResults - 1;
			if (vectorEnd > vectorResult.size()) vectorEnd = vectorResult.size() - 1;
		}

		for (;vectorStart <= vectorEnd; ++vectorStart) {
			auto* obj = vectorResult.at(vectorStart);
			if (!obj) continue;
			this->container->AddObjectToContainer(obj, nullptr, count, nullptr);
		}

		for (auto& foundScript : vm->attachedScripts.find(handle)->second) {
			if (!foundScript) continue;
			if (!foundScript->GetTypeInfo()) continue;
			if (foundScript->GetTypeInfo()->GetName() != "SAM_ManagerQuestScript"sv) continue;

			auto callback = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>();
			auto args = RE::MakeFunctionArguments();
			const RE::BSFixedString functionName = "BeginCountdown"sv;
			auto scriptObject = foundScript.get();
			auto object = RE::BSTSmartPointer<RE::BSScript::Object>(scriptObject);
			vm->DispatchMethodCall(object, functionName, args, callback);
			break;
		}
		return true;
	}

	void Manager::DisplayPage(uint64_t a_pageNum) {
		if (!quest || !container) return;
		this->container->ResetInventory(false);
		auto maxPage = this->vectorResult.size() / this->maxResults;
		if (a_pageNum > maxPage) a_pageNum = maxPage;

		auto vectorSize = this->vectorResult.size();
		size_t vectorStart = 0;
		size_t vectorEnd = vectorResult.size();
		if (vectorEnd > this->maxResults - 1) {
			vectorStart = a_pageNum * this->maxResults;
			vectorEnd = vectorStart + this->maxResults - 1;
			if (vectorEnd > vectorSize) vectorEnd = vectorSize - 1;
		}

		for (;vectorStart <= vectorEnd; ++vectorStart) {
			auto* obj = vectorResult.at(vectorStart);
			if (!obj) continue;
			this->container->AddObjectToContainer(obj, nullptr, count, nullptr);
		}

		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto* handlePolicy = vm->GetObjectHandlePolicy();
		if (!handlePolicy) return;
		RE::VMHandle handle = handlePolicy->GetHandleForObject(this->quest->GetFormType(), quest);
		if (!handle) return;

		for (auto& foundScript : vm->attachedScripts.find(handle)->second) {
			if (!foundScript) continue;
			if (!foundScript->GetTypeInfo()) continue;
			if (foundScript->GetTypeInfo()->GetName() != "SAM_ManagerQuestScript"sv) continue;

			auto callback = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>();
			auto args = RE::MakeFunctionArguments();
			const RE::BSFixedString functionName = "BeginCountdown"sv;
			auto scriptObject = foundScript.get();
			auto object = RE::BSTSmartPointer<RE::BSScript::Object>(scriptObject);
			vm->DispatchMethodCall(object, functionName, args, callback);
			break;
		}
	}

	void Manager::SetContainer(std::string a_id, std::string a_modName) {
		auto* form = GetFormFromMod(a_id, a_modName);
		auto* reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
		auto* containerBaseForm = reference ? reference->GetBaseObject() ? reference->GetBaseObject()->As<RE::TESObjectCONT>()  : nullptr : nullptr;
		if (!containerBaseForm) {
			SKSE::log::error("Failed to find a suitable container with this signature: {}~{}", a_id, a_modName);
			return;
		}
		_loggerInfo("Successfully validated container {}~{}", a_id, a_modName);
		this->container = reference;
	}

	void Container::Manager::SetCount(int32_t a_newCount) {
		this->count = a_newCount;
	}

	void Container::Manager::SetQuest(std::string a_id, std::string a_modName) {
		auto* form = GetFormFromMod(a_id, a_modName);
		auto* foundQuest = form ? form->As<RE::TESQuest>() : nullptr;
		if (!foundQuest) {
			SKSE::log::error("Failed to find a suitable quest with this signature: {}~{}", a_id, a_modName);
			return;
		}
		_loggerInfo("Successfully validated quest {}~{}", a_id, a_modName);
		this->quest = foundQuest;
	}

	void Container::Manager::SetMaxContainerItems(size_t a_num) {
		if (a_num < 50) a_num = 50;
		if (a_num > 1000) a_num = 1000;
		this->maxResults = a_num;
		_loggerInfo("Set max results to {}.", a_num);
	}
}