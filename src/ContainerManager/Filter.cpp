#include "Filter.h"

#include "ContainerManager.h"

namespace ContainerManager
{
	StoredForm::StoredForm(RE::TESBoundObject* a_form) {
		if (!a_form) {
			throw std::invalid_argument("Nullptr passed in StoredForm constructor: " + Utilities::EDID::GetEditorID(a_form));
		}

		if (!a_form->GetPlayable()) {
			throw std::invalid_argument("Unplayable form passed in StoredForm constructor: " + Utilities::EDID::GetEditorID(a_form));
		}

		this->formFilter = SimpleFilter::kNoFilters;

		std::vector<const RE::BGSKeyword*> foundFormKeywords = std::vector<const RE::BGSKeyword*>();
		std::vector<const RE::BGSKeyword*> foundFormEffectKeywords = std::vector<const RE::BGSKeyword*>();
		RE::BSTArray<RE::Effect*> effects{};
		
		// There is some casting that should never fail. Sometimes, it fails.
		if (a_form->IsWeapon()) {
			const auto* weapon = a_form->As<RE::TESObjectWEAP>();
			if (!weapon) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Weapon, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}
			this->formValue = weapon->value;

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
				formFilter |= SimpleFilter::kEnchanted;
				effects = enchantment->effects;
			}
			else {
				formFilter |= SimpleFilter::kUnenchanted;
			}
		}
		else if (a_form->IsAmmo()) {
			auto* ammo = a_form->As<RE::TESAmmo>();
			if (!ammo) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Ammo, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}
			this->formValue = ammo->value;

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
				formFilter |= SimpleFilter::kEnchanted;
				effects = enchantment->effects;
			}
			else {
				formFilter |= SimpleFilter::kUnenchanted;
			}
		}
		else if (a_form->IsArmor()) {
			const auto* armor = a_form->As<RE::TESObjectARMO>();
			if (!armor) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Armor, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}
			this->formValue = armor->value;

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
				this->formFilter |= SimpleFilter::kEnchanted;
				effects = enchantment->effects;
			}
			else {
				this->formFilter |= SimpleFilter::kUnenchanted;
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
			this->formValue = bookForm->value;

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
				this->formType = FormType::kBook;
			}
		}
		else if (a_form->IsSoulGem()) {
			const auto* soulGemForm = a_form->As<RE::TESSoulGem>();
			if (!soulGemForm) {
				throw new std::invalid_argument("Object passed in StoredForm evaluated as Soul Gem, but could not be cast as one: " + Utilities::EDID::GetEditorID(a_form));
			}
			this->formType = FormType::kSoulGem;
			this->formValue = soulGemForm->value;

			const auto formKeywordCount = soulGemForm->numKeywords;
			if (formKeywordCount > 0) {
				auto** keywords = soulGemForm->keywords;
				for (const auto* keyword : std::span(keywords, formKeywordCount)) {
					if (std::find(foundFormKeywords.begin(), foundFormKeywords.end(), keyword) != foundFormKeywords.end()) {
						foundFormKeywords.push_back(keyword);
					}
				}
			}

			const auto* dobj = RE::BGSDefaultObjectManager::GetSingleton();
			if (!dobj) {
				logger::error("Failed to get Default Object Manager in StoredForm constructor. You will crash later and it won't be my fault.");
				throw new std::exception("Failed to get Default Object Manager in StoredForm constructor.");
			}

			const auto* reusableSoulGemKeyword = dobj->GetObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kKeywordReusableSoulGem);
			if (!reusableSoulGemKeyword) {
				logger::error("Failed to get the default Reusable Soul Gem keyword. You will (probably) crash later.");
				throw new std::exception("Failed to get the default Reusable Soul Gem keyword. You will (probably) crash later.");
			}

			if (soulGemForm->GetContainedSoul() != RE::SOUL_LEVEL::kNone) {
				this->formFilter |= SimpleFilter::kFilled;
			}
			if (soulGemForm->HasKeyword(reusableSoulGemKeyword->formID)) {
				this->formFilter |= SimpleFilter::kRefillable;
			}
		}
		// I hate the 4 letter abbreviation more than you, it's the game record type.
		else if (const auto* misc = a_form->As<RE::TESObjectMISC>(); misc) {
			this->formType = FormType::kMisc;
			this->formValue = misc->value;

			const auto formKeywordCount = misc->numKeywords;
			if (formKeywordCount > 0) {
				auto** keywords = misc->keywords;
				for (const auto* keyword : std::span(keywords, formKeywordCount)) {
					if (std::find(foundFormKeywords.begin(), foundFormKeywords.end(), keyword) != foundFormKeywords.end()) {
						foundFormKeywords.push_back(keyword);
					}
				}
			}
		}
		else if (const auto* scrl = a_form->As<RE::ScrollItem>(); scrl) {
			this->formType = FormType::kScroll;
			this->formValue = scrl->value;
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

			try {
				this->formValue = static_cast<int32_t>(alci->CalculateTotalGoldValue());
			}
			catch (std::exception& e) {
				logger::warn("Failed to calculate gold value for AlchemyItem: {}. Likely an overflow, defaulting to {}. Error: {}", e.what(),
					Utilities::EDID::GetEditorID(a_form),
					std::numeric_limits<int32_t>::max());
				this->formValue = std::numeric_limits<int32_t>::max();
			}

			effects = alci->effects;
		}
		else if (const auto* ingr = a_form->As<RE::IngredientItem>(); ingr) {
			this->formType = FormType::kIngredient;
			this->formValue = ingr->value;
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
		logger::info("{}", Utilities::EDID::GetEditorID(this->form));
		if (this->formValue > 0) {
			logger::info("  Value: {}", this->formValue);
		}

		if (!this->formKeywords.empty()) {
			logger::info("  Form keywords:");
			for (const auto* keyword : this->formKeywords) {
				logger::info("    >{}", keyword->GetFormEditorID());
			}
		}

		if (!this->formEffectKeywords.empty()) {
			logger::info("  Effect keywords");
			for (const auto* keyword : this->formEffectKeywords) {
				logger::info("    >{}", keyword->GetFormEditorID());
			}
		}
		logger::info("---------------------------------------------------------");
	}

	RuleBuilder::RuleBuilder() {
		this->m_rule = Rule();
	}

	int RuleBuilder::Build() {
		auto result = std::make_unique<Rule>(m_rule);
		return ContainerManager::GetSingleton()->AddRule(std::move(result));
	}

	RuleBuilder& RuleBuilder::WithMinMaxValue(int a_min, int a_max)
	{
		this->m_rule.minGoldValue = a_min;
		this->m_rule.maxGoldValue = a_max;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithMinMaxWarmthValue(int a_min, int a_max)
	{
		this->m_rule.minWarmthValue = a_min;
		this->m_rule.maxWarmthValue = a_max;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithName(const std::string& a_name)
	{
		this->m_rule.ruleName = a_name;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithFormKeywords(std::vector<RE::BGSKeyword*> a_keywords)
	{
		this->m_rule.formKeywords = a_keywords;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithEffectKeywords(std::vector<RE::BGSKeyword*> a_keywords)
	{
		this->m_rule.effectKeywords = a_keywords;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithFormType(FormType a_type)
	{
		this->m_rule.acceptedFormTypes = a_type;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithFilters(SimpleFilter a_filters)
	{
		this->m_rule.acceptedFilters = a_filters;
		return *this;
	}
}