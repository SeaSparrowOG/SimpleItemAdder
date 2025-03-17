#include "Papyrus/Papyrus.h"

#include "ContainerManager/ContainerManager.h"
#include "ContainerManager/Filter.h"

namespace Papyrus
{
	std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	enum ToggleSettingResponses : int
	{
		kInvalidChange = -1,
		kSuccessfulChange,
		kNonExistentChange
	};

	int ToggleSetting(STATIC_ARGS, std::string a_setting) {
		LOG_DEBUG("[Papyrus Function Call]: ToggleSetting");
		LOG_DEBUG("  >Argument: {}", a_setting);
		if (a_setting.empty()) {
			LOG_DEBUG("  >Argument is empty, aborting.");
			return kNonExistentChange;
		}

		return kSuccessfulChange;
	}

	enum UpdateCountResponse : int
	{
		kInvalidCount = -1,
		kSuccessfulCount,
		kInputCountTooLarge,
		kInputCountNotANumber
	};

	int UpdateCount(STATIC_ARGS, std::string a_count) {
		LOG_DEBUG("[Papyrus Function Call]: UpdateCount");
		LOG_DEBUG("  >Argument: {}", a_count);
		if (a_count.empty()) {
			LOG_DEBUG("  >Count number empty, aborting.");
			return kInvalidCount;
		}

		int newCount = 0;
		try {
			newCount = std::stoi(a_count);
		}
		catch (const std::invalid_argument& e) {
			LOG_DEBUG("  >Count number provided is not a number (could not be converted), aborting: {}", e.what());
			return kInputCountNotANumber;
		}
		catch (const std::out_of_range& e) {
			LOG_DEBUG("  >Count number too large to fit in an int value, aborting: {}", e.what());
			return kInputCountTooLarge;
		}
		catch (const std::exception& e) { // okay but like what
			logger::warn("Caught exception {} in UpdateCount call. If you are seeing this, something went catastrophically wrong.", e.what());
			return kInvalidCount;
		}

		// TODO: Ping manager singleton to request Count.
		return kSuccessfulCount;
	}

	enum DisplayPageResponse : int
	{
		kInvalidPage = -1,
		kSuccessfulPage,
		kFinalPage,
		kInputPageTooLarge,
		kInputPageNotANumber
	};

	int DisplayPage(STATIC_ARGS, std::string a_page) {
		LOG_DEBUG("[Papyrus Function Call]: DisplayPage");
		LOG_DEBUG("  >Argument: {}", a_page);
		if (a_page.empty()) {
			LOG_DEBUG("  >Page number empty, aborting.");
			return kInvalidPage;
		}

		int pageToShow = 0;
		try {
			pageToShow = std::stoi(a_page);
		}
		catch (const std::invalid_argument& e) {
			LOG_DEBUG("  >Page number provided is not a number (could not be converted), aborting: {}", e.what());
			return kInputPageNotANumber;
		}
		catch (const std::out_of_range& e) {
			LOG_DEBUG("  >Page number too large to fit in an int value, aborting: {}", e.what());
			return kInputPageTooLarge;
		}
		catch (const std::exception& e) { // okay but like what
			logger::warn("Caught exception {} in DisplayPage call. If you are seeing this, something went catastrophically wrong.", e.what());
			return kInvalidPage;
		}

		// TODO: Ping manager singleton to request page.
		return kSuccessfulPage;
	}

	enum SearchItemResponses : int
	{
		kInvalidQuery = -1,
		kSuccessfulQuery,
		kTruncatedQuery
	};

	int SearchItem(STATIC_ARGS, std::string a_name, std::string a_type) {
		LOG_DEBUG("[Papyrus Function Call]: SearchItem");
		if (a_type.empty()) {
			LOG_DEBUG("  >Type empty, aborting.");
			return kInvalidQuery;
		}

		return kSuccessfulQuery;
	}

	std::optional<RE::FormType> ToFormType(int value) {
		if (value >= static_cast<int>(RE::FormType::None) && value < static_cast<int>(RE::FormType::Max)) {
			return static_cast<RE::FormType>(value);
		}
		return std::nullopt;
	}

	std::unique_ptr<ContainerManager::Rule> TokenizeFilter(const RE::FormType a_formType, const std::string& input, char mainDelimiter = '|', char pairDelimiter = ':') {
		std::unique_ptr<ContainerManager::Rule> result = std::make_unique<ContainerManager::Rule>();
		std::stringstream ss(input);
		std::string token;

		switch (a_formType) {
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
			break;
		case RE::FormType::Armor:
			break;
		case RE::FormType::Book:
			break;
		case RE::FormType::Ingredient:
			break;
		case RE::FormType::AlchemyItem:
			break;
		default:
			throw std::invalid_argument("Bad FormType given: " + std::to_string(static_cast<int>(a_formType)));
		}

		while (std::getline(ss, token, mainDelimiter)) {
			// Some people will inevitably try to add whitespaces between the filter and the pipe
			// (or do it by accident)
			// so we handle that gracefully.
			token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
			token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);

			// But if they forget ":" slap them.
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

			std::string newFilter = key + value;
		}
		return result;
	}

	/// <summary>
	/// Creates a unique filter based on a given string. The filter "lives" in the ContainerManager and cannot be manipulated.
	/// </summary>
	/// <param name="a_filters">The string representing the desired filter. If any part is invalid, the entire filter is discarded.</param>
	/// <returns>An int corresponding to the filter's ID. Returns -1 if the filter is not created.</returns>
	int CreateContainerStringFilter(STATIC_ARGS, std::string a_filters) {
		LOG_DEBUG("[Papyrus Function Call]: CreateContainerFilter");
		
		if (a_filters.empty()) {
			LOG_DEBUG("  >All arguments empty, aborting.");
			return -1;
		}

		try {
			
		}
		catch (std::invalid_argument& e) {
			logger::warn("Caught exception {} in CreateContainerFilter call.", e.what());
		}
		return -1;
	}

	/// <summary>
	/// Fills the container with the matching forms, and opens it when all menus close. Optionally may be provided with an array of filters.
	/// </summary>
	/// <param name="a_filters">An array of integers corresponding to stored filter handles. If empty, no filters will be applied. If an invalid filter is provided, or the filters conflict, will not work.</param>
	/// <returns>True if the container opens, false otherwise.</returns>
	bool OpenContainer(STATIC_ARGS, std::vector<int> a_filters = {}) {
		LOG_DEBUG("[Papyrus Function Call]: OpenContainer");
		return false;
	}

	void Bind(VM& a_vm) {
		BIND(GetVersion);
		BIND(ToggleSetting);
		BIND(UpdateCount);
		BIND(DisplayPage);
		BIND(SearchItem);
		BIND(CreateContainerStringFilter);
		BIND(OpenContainer);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}