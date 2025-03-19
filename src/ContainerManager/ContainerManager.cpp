#include "ContainerManager.h"

namespace ContainerManager
{
    bool ContainerManager::InitializeMembers() {
        const auto then = std::chrono::high_resolution_clock::now();
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            logger::error("Failed to get the Data Handler! You are likely going to crash later and it won't be my fault.");
            return false;
        }

        const auto* dobj = RE::BGSDefaultObjectManager::GetSingleton();
        if (!dobj) {
            logger::error("Failed to get the Default Object Manager! You are likely going to crash later and it won't be my fault.");
            return false;
        }

        const auto* eitherHandEquip = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kEitherHandEquip);
        const auto* leftHandEquip = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kLeftHandEquip);
        const auto* rightHandEquip = dobj->GetObject<RE::BGSEquipSlot>(RE::DEFAULT_OBJECT::kRightHandEquip);
        if (!eitherHandEquip || !leftHandEquip || !rightHandEquip) {
            logger::warn("Failed to get at least one default object (equip slots). You are likely going to crash later and it won't be my fault.");
            return true;
        }

        std::vector<StoredForm> tempContainer{};

        // Note - Repetition follows. I haven't decided if/how to add additional filtering.
        const auto& weaponArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();
        for (auto* form : weaponArray) {
            StoreFormInArray<RE::TESObjectWEAP>(form, tempContainer);
        }

        const auto& ammoArray = dataHandler->GetFormArray<RE::TESAmmo>();
        for (auto* form : ammoArray) {
            StoreFormInArray<RE::TESAmmo>(form, tempContainer);
        }

        const auto& armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
        for (auto* form : armorArray) {
            StoreFormInArray<RE::TESObjectARMO>(form, tempContainer);
        }

        const auto& bookArray = dataHandler->GetFormArray<RE::TESObjectBOOK>();
        for (auto* form : bookArray) {
            StoreFormInArray<RE::TESObjectBOOK>(form, tempContainer);
        }

        const auto& scrollArray = dataHandler->GetFormArray<RE::ScrollItem>();
        for (auto* form : scrollArray) {
            StoreFormInArray<RE::ScrollItem>(form, tempContainer);
        }

        const auto& ingredientArray = dataHandler->GetFormArray<RE::IngredientItem>();
        for (auto* form : ingredientArray) {
            StoreFormInArray<RE::IngredientItem>(form, tempContainer);
        }

        const auto& foodStuffArray = dataHandler->GetFormArray<RE::AlchemyItem>();
        for (auto* form : foodStuffArray) {
            StoreFormInArray<RE::AlchemyItem>(form, tempContainer);
        }

        const auto& soulGemArray = dataHandler->GetFormArray<RE::TESSoulGem>();
        for (auto* form : soulGemArray) {
            StoreFormInArray<RE::TESSoulGem>(form, tempContainer);
        }

        const auto& miscArray = dataHandler->GetFormArray<RE::TESObjectMISC>();
        for (auto* form : miscArray) {
            StoreFormInArray<RE::TESObjectMISC>(form, tempContainer);
        }

        std::sort(tempContainer.begin(), tempContainer.end(), [](const StoredForm& lhs, const StoredForm& rhs) {
            return lhs.formValue < rhs.formValue;
        });

        const auto now = std::chrono::high_resolution_clock::now();
        const auto dur = now - then;
        const auto result = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

        size_t contSize = tempContainer.size();
        logger::info("Stored {} forms in {}ms.", contSize, result);

        float memoryBytes = 0.0f;
        if (tempContainer.size() > 0) {
            float contItemSize = static_cast<float>(sizeof(tempContainer.at(0)));
            memoryBytes = contItemSize * contSize / BYTE_TO_MEGABYTE; //hello floating point error
        }

        logger::info("Memory used: {0:.3f}MBs", memoryBytes);
        logger::info("---------------------------------------------------------");

        return true;
    }

    int ContainerManager::AddRule(std::unique_ptr<Rule>&& a_rule) {
		auto storedSize = this->storedFilters.size();
        if (storedSize >= std::numeric_limits<int>::max()) {
            logger::warn("Rule limit reached. How did you manage that?");
            this->storedFilters.clear();
            storedSize = 0;
        }

		const auto index = static_cast<int>(storedSize);
		this->storedFilters.push_back(std::move(a_rule));
        return index - 1;
    }

    void ContainerManager::ResetPapyrusRules() {
        this->storedPapyrusFilters.clear();
    }

    std::vector<std::pair<int, std::string>> ContainerManager::GetPapyrusRulesDefinitions() {
        auto response = std::vector<std::pair<int, std::string>>();
		for (const auto& [index, rule] : storedPapyrusFilters) {
			response.push_back(std::make_pair(index, rule->ruleName));
		}
        return response;
    }
}