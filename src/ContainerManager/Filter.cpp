#include "Filter.h"

namespace ContainerManager
{
	Rule::Rule() {
		this->acceptedFormTypes = FormType::kAll;
		this->acceptedFilters   = SimpleFilter::kNoFilters;
		this->ruleName          = "UNDEFINED";
	}

	bool Rule::Matches(StoredForm* a_form) {
		const auto currentType = a_form->formType;
		if (currentType != kAll && !IsFormType(currentType, acceptedFormTypes)) {
			return false;
		}

		const auto currentFilter = a_form->formFilter;
		if (!IsSimpleFilter(currentFilter, acceptedFilters)) {
			return false;
		}

		return true;
	}

	StoredForm::StoredForm(RE::TESBoundObject* a_form) {
		if (!a_form) {
			throw std::invalid_argument("Nullptr passed in StoredForm constructor: " + Utilities::EDID::GetEditorID(a_form));
		}

		if (!a_form->GetPlayable()) {
			throw std::invalid_argument("Unplayable form passed in StoredForm constructor: " + Utilities::EDID::GetEditorID(a_form));
		}

		std::vector<const RE::BGSKeyword*> foundFormKeywords = std::vector<const RE::BGSKeyword*>();
		std::vector<const RE::BGSKeyword*> foundFormEffectKeywords = std::vector<const RE::BGSKeyword*>();
		RE::BSTArray<RE::Effect*> effects{};
		
		// There is some casting that should never fail. Sometimes, it fails.
		if (a_form->IsWeapon()) {
			const auto* weapon = a_form->As<RE::TESObjectWEAP>();
			if (!weapon) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Weapon, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}

			if (weapon->IsOneHandedSword()) {
				this->formType = FormType::kSword;
			}
			else if (weapon->IsOneHandedDagger()) {
				this->formType = FormType::kDagger;
			}
			else if (weapon->IsOneHandedAxe()) {
				this->formType = FormType::kAxe;
			}
			else if (weapon->IsOneHandedMace()) {
				this->formType = FormType::kMace;
			}
			else if (weapon->IsTwoHandedSword()) {
				this->formType = FormType::kGreatsword;
			}
			else if (weapon->IsTwoHandedAxe()) {
				this->formType = FormType::kWarAxe;
			}
			else if (weapon->IsStaff()) {
				this->formType = FormType::kStaff;
			}
			// Future me, this is for you. I am a dumbass.
			else if (weapon->IsBow() || weapon->IsCrossbow()) {
				this->formType = FormType::kBow;
			}
			else {
				this->formType = FormType::kWarhammer;
			}

			const auto formKeywordCount = weapon->numKeywords;
			if (formKeywordCount > 0) {
				for (const auto* keyword : std::span(weapon->keywords, formKeywordCount)) {
					if (std::find(foundFormKeywords.begin(), foundFormKeywords.end(), keyword) == foundFormKeywords.end()) {
						foundFormKeywords.push_back(keyword);
					}
				}
			}

			if (const auto* enchantment = weapon->formEnchanting;  enchantment) {
				effects = enchantment->effects;
			}
		}
		else if (a_form->IsAmmo()) {
			auto* ammo = a_form->As<RE::TESAmmo>();
			if (!ammo) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Ammo, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}

			if (ammo->IsBolt()) {
				this->formType = FormType::kBolt;
			}
			else {
				this->formType = FormType::kArrow;
			}
			
			const auto ammoKeywordCount = ammo->numKeywords;
			if (ammoKeywordCount > 0) {
				for (const auto* keyword : std::span(ammo->keywords, ammoKeywordCount)) {
					if (std::find(foundFormKeywords.begin(), foundFormKeywords.end(), keyword) == foundFormKeywords.end()) {
						foundFormKeywords.push_back(keyword);
					}
				}
			}

			const auto* projectile = ammo->data.projectile;
			const auto* explosion = projectile ? projectile->data.explosionType : nullptr;
			const auto* enchantment = explosion ? explosion->formEnchanting : nullptr;
			if (enchantment) {
				effects = enchantment->effects;
			}
		}
		else if (a_form->IsArmor()) {
			const auto* armor = a_form->As<RE::TESObjectARMO>();
			if (!armor) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Armor, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}

			const auto heavyArmorKeyword = RE::TESForm::LookupByEditorID("ArmorHeavy"sv);
			const auto lightArmorKeyword = RE::TESForm::LookupByEditorID("ArmorLight"sv);
			const auto clothingKeyword = RE::TESForm::LookupByEditorID("ArmorClothing"sv);
			if (!heavyArmorKeyword || !lightArmorKeyword || !clothingKeyword) {
				logger::error("Failed to look up armor keywords in StoredForm. You are (probably) going to crash later.");
				throw new std::exception("Failed to look up armor keywords in StoredForm. You are (probably) going to crash later.");
			}

			// Not immediately obvious - armor can be classified as heavy or light in the armor record, but the game uses the Armor 
			// keyword in perks, abilities and dialogue - so be extra cautious. Heavy takes precedence over light. Keyword over tag.
			bool isHeavy = armor->HasKeyword(heavyArmorKeyword->formID);
			bool isLight = armor->HasKeyword(lightArmorKeyword->formID);
			bool isClothing = armor->HasKeyword(clothingKeyword->formID);

			// Last check for Heavy or Light. Otherwise, default to clothing.
			if (!isHeavy && !isLight && !isClothing) {
				isHeavy = armor->IsHeavyArmor();
				isLight = armor->IsLightArmor();
			}

			if (isHeavy) {
				this->formType = FormType::kHeavy;
			}
			else if (isLight) {
				this->formType = FormType::kLight;
			}
			else {
				this->formType = FormType::kClothing;
			}

			if (const auto* enchantment = armor->formEnchanting; enchantment) {
				if (!enchantment->effects.empty()) {
					effects = enchantment->effects;
				}
			}

			const auto armorKeywordCount = armor->numKeywords;
			if (armorKeywordCount > 0) {
				for (const auto* keyword : std::span(armor->keywords, armorKeywordCount)) {
					if (std::find(foundFormKeywords.begin(), foundFormKeywords.end(), keyword) == foundFormKeywords.end()) {
						foundFormKeywords.push_back(keyword);
					}
				}
			}
		}
		else if (a_form->IsBook()) {
			auto* bookForm = a_form->As<RE::TESObjectBOOK>();
			if (!bookForm) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Book, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}

			if (bookForm->TeachesSkill()) {
				this->formType = FormType::kSkillBook;
			}
			else if (bookForm->TeachesSpell()) {
				this->formType = FormType::kSpellTome;
				const auto* spell = bookForm->GetSpell();
				if (spell) {
					effects = spell->effects;
				}
			}
			else if (bookForm->IsNote()) {
				this->formType = FormType::kNote;
			}
			else {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as a Book, but is not a note, skill book, or spell tome (or book): " + Utilities::EDID::GetEditorID(a_form));
			}
		}
		// I hate the 4 letter abbreviation more than you, it's the game record type.
		else if (const auto* scrl = a_form->As<RE::ScrollItem>(); scrl) {
			this->formType = FormType::kScroll;
			effects = scrl->effects;
		}
		else if (const auto* alci = a_form->As<RE::AlchemyItem>(); alci) {
			if (alci->IsFood()) {
				this->formType = FormType::kFood;
			}
			else if (alci->IsPoison()) {
				this->formType = FormType::kPoison;
			}
			else {
				this->formType = FormType::kPotion;
			}
			effects = alci->effects;
		}
		else if (const auto* ingr = a_form->As<RE::IngredientItem>(); ingr) {
			this->formType = FormType::kIngredient;
			effects = ingr->effects;
		}
		else {
			throw new std::invalid_argument("Invalid form passed in StoredForm constructor: " + Utilities::EDID::GetEditorID(a_form));
		}

		for (const auto* effect : effects) {
			if (!effect || !effect->baseEffect) {
				continue;
			}

			const auto effectkeywordCount = effect->baseEffect->numKeywords;
			if (effectkeywordCount < 1) {
				continue;
			}

			auto** effectKeywords = effect->baseEffect->keywords;
			for (const auto* keyword : std::span(effectKeywords, effectkeywordCount)) {
				if (std::find(foundFormEffectKeywords.begin(), foundFormEffectKeywords.end(), keyword) == foundFormEffectKeywords.end()) {
					foundFormEffectKeywords.push_back(keyword);
				}
			}
		}

		this->form = a_form;
		this->formKeywords = foundFormKeywords;
		this->formEffectKeywords = foundFormEffectKeywords;
	}

	void StoredForm::PrettyPrint() {
		LOG_DEBUG("{}", Utilities::EDID::GetEditorID(this->form));
		switch (this->formType) {
		case kDagger:
			LOG_DEBUG("  >Dagger");
			break;
		case kSword:
			LOG_DEBUG("  >Sword");
			break;
		case kAxe:
			LOG_DEBUG("  >Axe");
			break;
		case kMace:
			LOG_DEBUG("  >Mace");
			break;
		case kGreatsword:
			LOG_DEBUG("  >Greatsword");
			break;
		case kWarAxe:
			LOG_DEBUG("  >War Axe");
			break;
		case kWarhammer:
			LOG_DEBUG("  >Warhammer");
			break;
		case kBow:
			LOG_DEBUG("  >Bow (or crossbow fuck me)");
			break;
		case kStaff:
			LOG_DEBUG("  >Staff");
			break;
		case kBolt:
			LOG_DEBUG("  >Bolt");
			break;
		case kArrow:
			LOG_DEBUG("  >Arrow");
			break;
		default:
			LOG_DEBUG("Shouldn't be seeing this");
			break;
		}
		LOG_DEBUG("  Form keywords:");
		for (const auto* keyword : this->formKeywords) {
			LOG_DEBUG("    >{}", keyword->GetFormEditorID());
		}
		LOG_DEBUG("  Effect keywords");
		for (const auto* keyword : this->formEffectKeywords) {
			LOG_DEBUG("    >{}", keyword->GetFormEditorID());
		}
		LOG_DEBUG("---------------------------------------------------------");
	}
}