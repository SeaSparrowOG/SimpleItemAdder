#include "ContainerManager.h"

namespace ContainerManager
{
    bool ContainerManager::InitializeMembers()
    {
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

        LOG_DEBUG("Patching weapons...");
        const auto& weaponArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();
        for (auto* weap : weaponArray) {
            if (!weap || !weap->GetPlayable() || weap->IsBound()) {
                continue;
            }

            const auto* weapEquipSlot = weap->GetEquipSlot();
            if (weapEquipSlot != eitherHandEquip && weapEquipSlot != rightHandEquip && weapEquipSlot != leftHandEquip) {
                continue;
            }

            try {
                StoredForm createdForm = StoredForm(weap);
                validWeapons.push_back(std::move(createdForm));
            }
            catch (std::exception& e) {
                logger::warn("{}", e.what());
            }
        }

        for (auto& thing : this->validWeapons) {
            thing.PrettyPrint();
        }
        return true;
    }

    int ContainerManager::SaveFilter(std::unique_ptr<Rule>&& a_newFilter) {
        (void)a_newFilter;
        return -1;
    }

    bool ContainerManager::OpenContainer(std::vector<int>& a_filters) {
        (void)a_filters;
        return false;
    }
}