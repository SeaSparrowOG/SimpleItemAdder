#include "Papyrus/Papyrus.h"

#include "ContainerManager/ContainerManager.h"
#include "ContainerManager/Filter.h"

namespace
{
	struct SanitizedPair
	{
		std::string key{ "UNDEFINED" };
		std::string value{ "UNDEFINED" };

		SanitizedPair(const std::string& a_key, const std::string& a_value)
		{
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

	private:
		static constexpr std::array valid_keys
		{
			"name",
			"goldvalue",
			"warmth",
			"type",
			"filter"
		};

		static constexpr bool IsKeyValid(const std::string& a_key) {
			if (a_key.empty()) {
				return false;
			}
			return std::find(valid_keys.begin(), valid_keys.end(), a_key) != valid_keys.end();
		}
	};

	struct ParsedToken
	{
	private:
		int32_t parsedMinValue{ -1 };
		int32_t parsedMaxValue{ -1 };
		int32_t parsedMinWarmthValue{ -1 };
		int32_t parsedMaxWarmthValue{ -1 };

		std::string                    parsedName{ "UNDEFINED" };

		std::vector<RE::BGSKeyword*>   parsedFormKeywords{};
		std::vector<RE::BGSKeyword*>   parsedEffectKeywords{};

		ContainerManager::FormType     acceptedFormType{ ContainerManager::FormType::kAll };
		ContainerManager::SimpleFilter acceptedFilters{ ContainerManager::SimpleFilter::kNoFilters };

		ContainerManager::RuleBuilder  builder{};

	public:
		ParsedToken(const std::vector<SanitizedPair>& a_pairs,
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
				else if (pair.key == "goldvalue") {
					const auto [min, max] = Utilities::String::SplitIntegers<int>(pair.value);
					parsedMaxValue = max ? max.value() : min;
					parsedMinValue = !max ? std::numeric_limits<int>::min() : min;
				}
				else if (pair.key == "warmth") {
					const auto [min, max] = Utilities::String::SplitIntegers<int>(pair.value);
					parsedMaxValue = max ? max.value() : min;
					parsedMinValue = !max ? std::numeric_limits<int>::min() : min;
				}
				else if (pair.key == "type") {
					if (acceptedFormType != ContainerManager::FormType::kAll) {
						throw std::invalid_argument("Parsed Token: 'type' already set.");
					}

					const auto& value = pair.value;
					if (value == "kdagger") {
						acceptedFormType = ContainerManager::FormType::kDagger;
					}
					else if (value == "ksword") {
						acceptedFormType = ContainerManager::FormType::kSword;
					}
					else if (value == "kaxe") {
						acceptedFormType = ContainerManager::FormType::kAxe;
					}
					else if (value == "kmace") {
						acceptedFormType = ContainerManager::FormType::kMace;
					}
					else if (value == "kbow") {
						acceptedFormType = ContainerManager::FormType::kBow;
					}
					else if (value == "kgreatsword") {
						acceptedFormType = ContainerManager::FormType::kGreatsword;
					}
					else if (value == "kwaraxe") {
						acceptedFormType = ContainerManager::FormType::kWarAxe;
					}
					else if (value == "kwarhammer") {
						acceptedFormType = ContainerManager::FormType::kWarhammer;
					}
					else if (value == "kstaff") {
						acceptedFormType = ContainerManager::FormType::kStaff;
					}
					else if (value == "kbolt") {
						acceptedFormType = ContainerManager::FormType::kBolt;
					}
					else if (value == "karrow") {
						acceptedFormType = ContainerManager::FormType::kArrow;
					}
					else if (value == "kheavy") {
						acceptedFormType = ContainerManager::FormType::kHeavy;
					}
					else if (value == "klight") {
						acceptedFormType = ContainerManager::FormType::kLight;
					}
					else if (value == "kshield") {
						acceptedFormType = ContainerManager::FormType::kShield;
					}
					else if (value == "kclothing") {
						acceptedFormType = ContainerManager::FormType::kClothing;
					}
					else if (value == "kspelltome") {
						acceptedFormType = ContainerManager::FormType::kSpellTome;
					}
					else if (value == "knovice") {
						acceptedFormType = ContainerManager::FormType::kNovice;
					}
					else if (value == "kapprentice") {
						acceptedFormType = ContainerManager::FormType::kApprentice;
					}
					else if (value == "kadept") {
						acceptedFormType = ContainerManager::FormType::kAdept;
					}
					else if (value == "kexpert") {
						acceptedFormType = ContainerManager::FormType::kExpert;
					}
					else if (value == "kmaster") {
						acceptedFormType = ContainerManager::FormType::kMaster;
					}
					else if (value == "kbook") {
						acceptedFormType = ContainerManager::FormType::kBook;
					}
					else if (value == "knote") {
						acceptedFormType = ContainerManager::FormType::kNote;
					}
					else if (value == "kskillbook") {
						acceptedFormType = ContainerManager::FormType::kSkillBook;
					}
					else if (value == "kscroll") {
						acceptedFormType = ContainerManager::FormType::kScroll;
					}
					else if (value == "kpotion") {
						acceptedFormType = ContainerManager::FormType::kPotion;
					}
					else if (value == "kpoison") {
						acceptedFormType = ContainerManager::FormType::kPoison;
					}
					else if (value == "kfood") {
						acceptedFormType = ContainerManager::FormType::kFood;
					}
					else if (value == "kingredient") {
						acceptedFormType = ContainerManager::FormType::kIngredient;
					}
					else if (value == "ksoulgem") {
						acceptedFormType = ContainerManager::FormType::kSoulGem;
					}
					else if (value == "kmisc") {
						acceptedFormType = ContainerManager::FormType::kMisc;
					}
					else if (value == "kmeleeweapon") {
						acceptedFormType = ContainerManager::FormType::kMeleeWeapon;
					}
					else if (value == "krangedweapon") {
						acceptedFormType = ContainerManager::FormType::kRangedWeapon;
					}
					else if (value == "kammo") {
						acceptedFormType = ContainerManager::FormType::kAmmo;
					}
					else if (value == "kweapon") {
						acceptedFormType = ContainerManager::FormType::kWeapon;
					}
					else if (value == "karmor") {
						acceptedFormType = ContainerManager::FormType::kArmor;
					}
					else if (value == "kdefensive") {
						acceptedFormType = ContainerManager::FormType::kDefensive;
					}
					else if (value == "kbody") {
						acceptedFormType = ContainerManager::FormType::kBody;
					}
					else if (value == "kapparel") {
						acceptedFormType = ContainerManager::FormType::kApparel;
					}
					else if (value == "kspelltome") {
						acceptedFormType = ContainerManager::FormType::kSpellTome;
					}
					else if (value == "kmagic") {
						acceptedFormType = ContainerManager::FormType::kMagic;
					}
					else if (value == "knonmagic") {
						acceptedFormType = ContainerManager::FormType::kNonMagic;
					}
					else if (value == "kwritten") {
						acceptedFormType = ContainerManager::FormType::kWritten;
					}
					else if (value == "kalchemy") {
						acceptedFormType = ContainerManager::FormType::kAlchemy;
					}
					else if (value == "kingestible") {
						acceptedFormType = ContainerManager::FormType::kIngestible;
					}
					else {
						throw std::invalid_argument("Parsed Token: Invalid value for 'type': " + pair.value);
					}
				}
				else if (pair.key == "filter") {
					const auto& [value, setting] = Utilities::String::SplitStrings(pair.value);
					bool toggleOn = setting.has_value() ? setting.value() == "true" : true;

					if (value == "enchanted") {
						if (toggleOn) {
							acceptedFilters |= ContainerManager::SimpleFilter::kEnchanted;
						}
						else {
							acceptedFilters |= ContainerManager::SimpleFilter::kUnenchanted;
						}
					}
					else if (value == "filled") {
						if (toggleOn) {
							acceptedFilters |= ContainerManager::SimpleFilter::kFilled;
						}
						else {
							acceptedFilters |= ContainerManager::SimpleFilter::kEmpty;
						}
					}
					else if (value == "refillable") {
						if (toggleOn) {
							acceptedFilters |= ContainerManager::SimpleFilter::kRefillable;
						}
						else {
							acceptedFilters |= ContainerManager::SimpleFilter::kSingleUse;
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

		void RegisterKeywords(const std::vector<RE::BGSKeyword*>& a_formKeywords,
			const std::vector<RE::BGSKeyword*>& a_effectKeywords)
		{
			this->parsedFormKeywords = a_formKeywords;
			this->parsedEffectKeywords = a_effectKeywords;
		}

		int ParseToken() {
			builder.WithName(parsedName);

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

			if (acceptedFormType != ContainerManager::FormType::kAll) {
				builder.WithFormType(acceptedFormType);
			}

			if (!parsedFormKeywords.empty()) {
				builder.WithFormKeywords(parsedFormKeywords);
			}

			if (!parsedEffectKeywords.empty()) {
				builder.WithEffectKeywords(parsedEffectKeywords);
			}

			if (acceptedFilters != ContainerManager::SimpleFilter::kNoFilters) {
				builder.WithFilters(acceptedFilters);
			}

			return builder.Build();
		}
	};

	/// <summary>
	/// Transforms a given string from Papyrus into a usable token. Deletes whitespaces within reason.
	/// </summary>
	/// <param name="input">The given string.</param>
	/// <param name="mainDelimiter">The main character to use as a delimiter between different filters. Default(|)</param>
	/// <param name="pairDelimiter">The character to use as a delimiter for the filter's key and value. Default(:)</param>
	/// <returns>A proper token.</returns>
	/// <exception cref="std::invalid_argument If the given string contains errors. Different exceptions contain more info."></exception>
	ParsedToken TokenizeFilter(const std::string& input, char mainDelimiter = '|', char pairDelimiter = ':') {
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
		std::vector<RE::BGSKeyword*> emptyKeywords{};
		ParsedToken result = ParsedToken(sanitizedSplitInput, emptyKeywords, emptyKeywords);
		return result;
	}
}
namespace Papyrus
{
	std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	int CreateFilterPapyrus(STATIC_ARGS, 
		std::string a_raw,
		std::vector<RE::BGSKeyword*> a_formKeywords,
		std::vector<RE::BGSKeyword*> a_effectKeywords)
	{
		LOG_DEBUG("[Papyrus Function Call]: CreateFilterPapyrus");
		
		if (a_raw.empty() && a_formKeywords.empty() && a_effectKeywords.empty()) {
			LOG_DEBUG("  >All arguments empty, aborting.");
			return -1;
		}

		try {
			auto token = TokenizeFilter(a_raw);
			token.RegisterKeywords(a_formKeywords, a_effectKeywords);
			return token.ParseToken();
		}
		catch (std::invalid_argument& e) {
			logger::warn("Caught {}.", e.what());
			return -1;
		}
		catch (std::exception& e) {
			logger::error("Caught {}. While you didn't crash, this is unexpected. If you see this, please report it.", e.what());
			return -1;
		}
	}

	int CreateFilterConsole(STATIC_ARGS, std::string a_raw) {
		LOG_DEBUG("[Papyrus Function Call]: CreateFilterConsole");

		if (a_raw.empty()) {
			LOG_DEBUG("  >All arguments empty, aborting.");
			return -1;
		}

		try {
			auto token = TokenizeFilter(a_raw);
			return token.ParseToken();
		}
		catch (std::invalid_argument& e) {
			logger::warn("Caught {}.", e.what());
			return -1;
		}
		catch (std::exception& e) {
			logger::error("Caught {}. While you didn't crash, this is unexpected. If you see this, please report it.", e.what());
			return -1;
		}
	}

	void Bind(VM& a_vm) {
		BIND(GetVersion);
		BIND(CreateFilterPapyrus);
		BIND(CreateFilterConsole);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}