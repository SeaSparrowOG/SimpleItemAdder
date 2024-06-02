#include "papyrusFunctions.h"

namespace Functions {

	void ToggleSetting(STATIC_ARGS, std::string a_setting) {
		Container::Manager::GetSingleton()->ToggleSetting(a_setting);
	}

	bool SearchItem(STATIC_ARGS, std::string a_name, std::string a_type) {
		if (a_type.empty()) return false;

		a_name = clib_util::string::tolower(a_name);
		a_type = clib_util::string::tolower(a_type);

		Container::QueryType query = Container::QueryType::kAll;
		if (a_type == "armo") {
			query = Container::QueryType::kArmor;
		}
		else if (a_type == "weap") {
			query = Container::QueryType::kWeapon;
		}
		else if (a_type == "book") {
			query = Container::QueryType::kBook;
		}
		else if (a_type == "ingr") {
			query = Container::QueryType::kIngredient;
		}
		else if (a_type == "misc") {
			query = Container::QueryType::kMisc;
		}
		else if (a_type == "alci") {
			query = Container::QueryType::kConsumable;
		}

		if (query == Container::QueryType::kAll) return false;

		return Container::Manager::GetSingleton()->SearchItem(a_name, query);
	}

	void DisplayPage(STATIC_ARGS, std::string a_page) {
		if (!clib_util::string::is_only_digit(a_page)) return;

		size_t pageNum = 0;
		std::stringstream stream(a_page);
		stream >> pageNum;
		if (pageNum < 1) pageNum = 1;
		Container::Manager::GetSingleton()->DisplayPage(pageNum);
	}

	void UpdateCount(STATIC_ARGS, std::string a_count) {
		if (!clib_util::string::is_only_digit(a_count)) return;

		int32_t count = 0;
		std::stringstream stream(a_count);
		stream >> count;
		if (count < 1) count = 1;
		if (count > 100) count = 100;
		Container::Manager::GetSingleton()->SetCount(count);
	}

	std::vector<int> GetSimpleItemAdderVersion(STATIC_ARGS) {
		return { Version::MAJOR, Version::MINOR, Version::PATCH };
	}

	void Bind(VM& a_vm) {
		BIND(SearchItem);
		BIND(ToggleSetting);
		BIND(GetSimpleItemAdderVersion);
		BIND(DisplayPage);
		BIND(UpdateCount);
		_loggerInfo("Bound papyrus functions.");
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}