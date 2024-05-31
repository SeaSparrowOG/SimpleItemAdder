#include "papyrusFunctions.h"

namespace Functions {

	void ToggleSetting(STATIC_ARGS, std::string a_setting) {
		Container::Manager::GetSingleton()->ToggleSetting(a_setting);
	}

	bool SearchItem(STATIC_ARGS, std::string a_name, std::string a_type) {
		if (a_type.empty() || a_name.empty()) return false;

		a_name = clib_util::string::tolower(a_name);
		a_type = clib_util::string::tolower(a_type);

		Container::QueryType query = Container::QueryType::kAll;
		if (a_type == "armo") {
			query = Container::QueryType::kArmor;
		}
		else if (a_type == "weap") {
			query = Container::QueryType::kWeapon;
		}
		if (query == Container::QueryType::kAll) return false;

		return Container::Manager::GetSingleton()->SearchItem(a_name, query);
	}

	void Bind(VM& a_vm) {
		BIND(SearchItem);
		BIND(ToggleSetting);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}