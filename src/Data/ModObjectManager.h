#pragma once

namespace
{
	static constexpr auto MakeQuestName() {
		constexpr std::string_view name = Plugin::NAME;
		constexpr std::string_view suffix = "_ModObjectsQuest";

		std::array<char, name.size() + suffix.size() + 1> buffer{};
		std::copy(name.begin(), name.end(), buffer.begin());
		std::copy(suffix.begin(), suffix.end(), buffer.begin() + name.size());

		return buffer;
	}

	static constexpr auto MakeScriptName() {
		constexpr std::string_view name = Plugin::NAME;
		constexpr std::string_view suffix = "_ModObjectsScript";

		std::array<char, name.size() + suffix.size() + 1> buffer{};
		std::copy(name.begin(), name.end(), buffer.begin());
		std::copy(suffix.begin(), suffix.end(), buffer.begin() + name.size());

		return buffer;
	}
}

namespace Data
{
	class ModObjectManager : public Utilities::Singleton::EventClass<ModObjectManager, RE::TESQuestInitEvent>
	{
	public:
		static constexpr auto QuestNameArray = MakeQuestName();
		static constexpr std::string_view QuestName{ QuestNameArray.data(), QuestNameArray.size() };
		static constexpr auto ScriptNameArray = MakeScriptName();
		static constexpr std::string_view ScriptName{ ScriptNameArray.data(), ScriptNameArray.size() };
		
		RE::BSEventNotifyControl ProcessEvent(
			const RE::TESQuestInitEvent* a_event,
			RE::BSTEventSource<RE::TESQuestInitEvent>* a_eventSource) override;

		void Reload();

		[[nodiscard]] RE::TESForm* Get(std::string_view a_key) const;

	private:
		void Initialize(RE::TESQuest* a_quest);

		util::istring_map<RE::TESForm*> objects;
	};

	template <typename T>
	[[nodiscard]] inline T* ModObject(std::string_view a_key)
	{
		if (const auto object = ModObjectManager::Instance().Get(a_key))
			return object->As<T>();
		return nullptr;
	}
}