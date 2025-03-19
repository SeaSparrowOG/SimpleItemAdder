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

	RuleBuilder& RuleBuilder::WithOriginalFilterString(const std::string& a_string)
	{
		this->m_rule.originalFilterString = a_string;
		return *this;
	}

	RuleBuilder& RuleBuilder::WithFormNameFilter(const std::string& a_string)
	{
		if (a_string.empty()) {
			throw std::invalid_argument("RuleBuilder: Empty string passed to WithFormNameFilter.");
		}

		this->m_rule.formName = a_string;
		return *this;
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

namespace ContainerManager::Helpers
{
	SanitizedPair::SanitizedPair(const std::string& a_key, const std::string& a_value) {
		if (a_value.empty()) {
			throw std::invalid_argument("Sanitized Pair: Empty value for key: " + a_key);
		}

		const auto sanitizedKey = Utilities::String::tolower(a_key);
		const auto sanitizedValue = Utilities::String::tolower(a_value);

		if (!IsKeyValid(sanitizedKey)) {
			throw std::invalid_argument("Sanitized Pair: Invalid key: " + a_key);
		}

		key = sanitizedKey;
		value = sanitizedValue;
	}

	ParsedToken::ParsedToken(const std::vector<SanitizedPair>& a_pairs,
		std::vector<RE::BGSKeyword*> a_formKeywords,
		std::vector<RE::BGSKeyword*> a_effectKeywords) 
	{
		if (!a_formKeywords.empty()) {
			if (std::find(a_formKeywords.begin(), a_formKeywords.end(), nullptr) != a_formKeywords.end()) {
				throw std::invalid_argument("Parsed Token: Nullptr found in form keywords.");
			}
			parsedFormKeywords = a_formKeywords;
		}
		if (!a_effectKeywords.empty()) {
			if (std::find(a_effectKeywords.begin(), a_effectKeywords.end(), nullptr) != a_effectKeywords.end()) {
				throw std::invalid_argument("Parsed Token: Nullptr found in effect keywords.");
			}
			parsedEffectKeywords = a_effectKeywords;
		}

		for (const auto& pair : a_pairs) {
			if (pair.key == "name") {
				parsedName = pair.value;
			}
			else if (pair.key == "itemname") {
				if (!parsedName.empty()) {
					throw std::invalid_argument("Parsed Token: 'name' already set.");
				}

				parsedSubstring = pair.value;
			}
			else if (pair.key == "goldvalue") {
				if (parsedMinValue != -1 || parsedMaxValue != -1) {
					throw std::invalid_argument("Parsed Token: 'goldvalue' already set.");
				}

				const auto [min, max] = Utilities::String::SplitIntegers<int>(pair.value);
				parsedMaxValue = max ? max.value() : min;
				parsedMinValue = !max ? std::numeric_limits<int>::min() : min;
			}
			else if (pair.key == "warmth") {
				if (parsedMinWarmthValue != -1 || parsedMaxWarmthValue != -1) {
					throw std::invalid_argument("Parsed Token: 'goldvalue' already set.");
				}

				const auto [min, max] = Utilities::String::SplitIntegers<int>(pair.value);
				parsedMaxValue = max ? max.value() : min;
				parsedMinValue = !max ? std::numeric_limits<int>::min() : min;
			}
			else if (pair.key == "type") {
				if (acceptedFormType != FormType::kAll) {
					throw std::invalid_argument("Parsed Token: 'type' already set.");
				}

				const auto& value = pair.value;
				if (value == "kdagger") {
					acceptedFormType = FormType::kDagger;
				}
				else if (value == "ksword") {
					acceptedFormType = FormType::kSword;
				}
				else if (value == "kaxe") {
					acceptedFormType = FormType::kAxe;
				}
				else if (value == "kmace") {
					acceptedFormType = FormType::kMace;
				}
				else if (value == "kbow") {
					acceptedFormType = FormType::kBow;
				}
				else if (value == "kgreatsword") {
					acceptedFormType = FormType::kGreatsword;
				}
				else if (value == "kwaraxe") {
					acceptedFormType = FormType::kWarAxe;
				}
				else if (value == "kwarhammer") {
					acceptedFormType = FormType::kWarhammer;
				}
				else if (value == "kstaff") {
					acceptedFormType = FormType::kStaff;
				}
				else if (value == "kbolt") {
					acceptedFormType = FormType::kBolt;
				}
				else if (value == "karrow") {
					acceptedFormType = FormType::kArrow;
				}
				else if (value == "kheavy") {
					acceptedFormType = FormType::kHeavy;
				}
				else if (value == "klight") {
					acceptedFormType = FormType::kLight;
				}
				else if (value == "kshield") {
					acceptedFormType = FormType::kShield;
				}
				else if (value == "kclothing") {
					acceptedFormType = FormType::kClothing;
				}
				else if (value == "kspelltome") {
					acceptedFormType = FormType::kSpellTome;
				}
				else if (value == "knovice") {
					acceptedFormType = FormType::kNovice;
				}
				else if (value == "kapprentice") {
					acceptedFormType = FormType::kApprentice;
				}
				else if (value == "kadept") {
					acceptedFormType = FormType::kAdept;
				}
				else if (value == "kexpert") {
					acceptedFormType = FormType::kExpert;
				}
				else if (value == "kmaster") {
					acceptedFormType = FormType::kMaster;
				}
				else if (value == "kbook") {
					acceptedFormType = FormType::kBook;
				}
				else if (value == "knote") {
					acceptedFormType = FormType::kNote;
				}
				else if (value == "kskillbook") {
					acceptedFormType = FormType::kSkillBook;
				}
				else if (value == "kscroll") {
					acceptedFormType = FormType::kScroll;
				}
				else if (value == "kpotion") {
					acceptedFormType = FormType::kPotion;
				}
				else if (value == "kpoison") {
					acceptedFormType = FormType::kPoison;
				}
				else if (value == "kfood") {
					acceptedFormType = FormType::kFood;
				}
				else if (value == "kingredient") {
					acceptedFormType = FormType::kIngredient;
				}
				else if (value == "ksoulgem") {
					acceptedFormType = FormType::kSoulGem;
				}
				else if (value == "kmisc") {
					acceptedFormType = FormType::kMisc;
				}
				else if (value == "kmeleeweapon") {
					acceptedFormType = FormType::kMeleeWeapon;
				}
				else if (value == "krangedweapon") {
					acceptedFormType = FormType::kRangedWeapon;
				}
				else if (value == "kammo") {
					acceptedFormType = FormType::kAmmo;
				}
				else if (value == "kweapon") {
					acceptedFormType = FormType::kWeapon;
				}
				else if (value == "karmor") {
					acceptedFormType = FormType::kArmor;
				}
				else if (value == "kdefensive") {
					acceptedFormType = FormType::kDefensive;
				}
				else if (value == "kbody") {
					acceptedFormType = FormType::kBody;
				}
				else if (value == "kapparel") {
					acceptedFormType = FormType::kApparel;
				}
				else if (value == "kspelltome") {
					acceptedFormType = FormType::kSpellTome;
				}
				else if (value == "kmagic") {
					acceptedFormType = FormType::kMagic;
				}
				else if (value == "knonmagic") {
					acceptedFormType = FormType::kNonMagic;
				}
				else if (value == "kwritten") {
					acceptedFormType = FormType::kWritten;
				}
				else if (value == "kalchemy") {
					acceptedFormType = FormType::kAlchemy;
				}
				else if (value == "kingestible") {
					acceptedFormType = FormType::kIngestible;
				}
				else {
					throw std::invalid_argument("Parsed Token: Invalid value for 'type': " + pair.value);
				}
			}
			else if (pair.key == "filter") {
				if (acceptedFilters != SimpleFilter::kNoFilters) {
					throw std::invalid_argument("Parsed Token: 'filter' already set.");
				}

				const auto& [value, setting] = Utilities::String::SplitStrings(pair.value);
				bool toggleOn = setting.has_value() ? setting.value() == "true" : true;

				if (value == "enchanted") {
					if (toggleOn) {
						acceptedFilters |= SimpleFilter::kEnchanted;
					}
					else {
						acceptedFilters |= SimpleFilter::kUnenchanted;
					}
				}
				else if (value == "filled") {
					if (toggleOn) {
						acceptedFilters |= SimpleFilter::kFilled;
					}
					else {
						acceptedFilters |= SimpleFilter::kEmpty;
					}
				}
				else if (value == "refillable") {
					if (toggleOn) {
						acceptedFilters |= SimpleFilter::kRefillable;
					}
					else {
						acceptedFilters |= SimpleFilter::kSingleUse;
					}
				}
				else {
					throw std::invalid_argument("Parsed Token: Invalid value for 'type': " + pair.value);
				}
			}
			else {
				throw std::invalid_argument("Parsed Token: Invalid key: " + pair.key);
			}
		}
	}

	int ParsedToken::ParseToken(const std::string& originalFilter) {
		builder.WithName(parsedName);
		builder.WithOriginalFilterString(originalFilter);

		if (parsedMinValue > -1 || parsedMaxValue > -1) {
			const auto requiredMin = parsedMinValue > -1 ? parsedMinValue : 0;
			const auto requiredMax = parsedMaxValue > -1 ? parsedMaxValue : std::numeric_limits<int32_t>::max();
			builder.WithMinMaxValue(requiredMin, requiredMax);
		}

		if (parsedMinWarmthValue > -1 || parsedMaxWarmthValue > -1) {
			const auto requiredMinWarmth = parsedMinWarmthValue > -1 ? parsedMinWarmthValue : 0;
			const auto requiredMaxWarmth = parsedMaxWarmthValue > -1 ? parsedMaxWarmthValue : std::numeric_limits<int32_t>::max();
			builder.WithMinMaxWarmthValue(requiredMinWarmth, requiredMaxWarmth);
		}

		if (acceptedFormType != FormType::kAll) {
			builder.WithFormType(acceptedFormType);
		}

		if (!parsedFormKeywords.empty()) {
			builder.WithFormKeywords(parsedFormKeywords);
		}

		if (!parsedEffectKeywords.empty()) {
			builder.WithEffectKeywords(parsedEffectKeywords);
		}

		if (acceptedFilters != SimpleFilter::kNoFilters) {
			builder.WithFilters(acceptedFilters);
		}

		if (!parsedSubstring.empty()) {
			builder.WithFormNameFilter(parsedSubstring);
		}

		return builder.Build();
	}

	ParsedToken TokenizeFilter(const std::string& input,
		const std::vector<RE::BGSKeyword*>& a_formKeywords,
		const std::vector<RE::BGSKeyword*>& a_effectKeywords,
		char mainDelimiter,
		char pairDelimiter)
	{
		std::vector<SanitizedPair> sanitizedSplitInput{};
		std::stringstream ss(input);
		std::string token;

		while (std::getline(ss, token, mainDelimiter)) {
			token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
			token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);

			size_t pos = token.find(pairDelimiter);
			if (pos == std::string::npos) {
				throw std::invalid_argument("Malformed token (missing ':'): " + token);
			}

			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);

			key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
			key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
			value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
			value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

			if (key.empty() || value.empty()) {
				throw std::invalid_argument("Malformed token (empty key or value): " + token);
			}

			auto sanitizedPair = SanitizedPair(key, value);
			sanitizedSplitInput.push_back(sanitizedPair);
		}
		ParsedToken result = ParsedToken(sanitizedSplitInput, a_formKeywords, a_effectKeywords);
		return result;
	}
}