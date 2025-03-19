#include "Papyrus/Papyrus.h"

#include "ContainerManager/ContainerManager.h"
#include "ContainerManager/Filter.h"

namespace Papyrus
{
	/// <summary>
	/// Simple GetVersion function to call from Papyrus. The version is defined in CMake. Good way to tell if the plugin is loaded.
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	/// <summary>
	/// Papyrus version of the string tokenizer, to be called from Papyrus scripts. Also allows you to specify form and effect keywords.
	/// </summary>
	/// <param name="a_raw">The filter string. Must follow the structure outlined in the mod page.</param>
	/// <param name="a_formKeywords">Array of keywords that the allowed items must have.</param>
	/// <param name="a_effectKeywords">Array of keywords that the allowed items' effects must have. Some forms (misc, slgm, etc) may not have effects.</param>
	/// <returns>An int corresponding to the handle of the created function. Papyrus conditions PERSIST across saves.</returns>
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
			auto token = ContainerManager::Helpers::TokenizeFilter(a_raw, a_formKeywords, a_effectKeywords);
			return token.ParseToken(a_raw);
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

	/// <summary>
	/// Console version of the string tokenizer, to be called from the console. Does not allow you to specify form and effect keywords.
	/// </summary>
	/// <param name="a_raw">The filter string. Must follow the structure outlined in the mod page.</param>
	/// <returns>An int corresponding to the handle of the created function. Console conditions do NOT persist across saves.</returns>
	int CreateFilterConsole(STATIC_ARGS, std::string a_raw) {
		LOG_DEBUG("[Papyrus Function Call]: CreateFilterConsole");

		if (a_raw.empty()) {
			LOG_DEBUG("  >All arguments empty, aborting.");
			return -1;
		}

		try {
			std::vector<RE::BGSKeyword*> emptyKeywords{};
			auto token = ContainerManager::Helpers::TokenizeFilter(a_raw, emptyKeywords, emptyKeywords);
			return token.ParseToken(a_raw);
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